#include "quectel_ec200.h"
#include <string.h>

/* ઇન્ટરનલ ટાસ્કનું ડિક્લેરેશન */
static void Quectel_Parser_Task(void *argument);

/* ==================================================================== */
/* 1. Initialization Function (લાઇબ્રેરી અને DMA શરૂ કરવા માટે)        */
/* ==================================================================== */
void Quectel_Init(Quectel_Handle_t *hgsm, UART_HandleTypeDef *huart)
{
    // હાર્ડવેર અને વેરીએબલ્સ લિંક કરો
    hgsm->huart = huart;
    hgsm->is_ready = false;
    hgsm->is_network_registered = false;
    hgsm->cmd_status = GSM_TIMEOUT;
    memset(hgsm->expected_response, 0, sizeof(hgsm->expected_response));

    // 🔴 1. રીંગ બફર શરૂ કરો
    lwrb_init(&hgsm->rx_ring_buffer, hgsm->rx_buffer_data, sizeof(hgsm->rx_buffer_data));

    // 🔴 2. RTOS સેમાફોર બનાવો
    hgsm->parser_sem = osSemaphoreNew(1, 0, NULL);
    hgsm->cmd_sem = osSemaphoreNew(1, 0, NULL);

    // 🔴 3. બેકગ્રાઉન્ડ પાર્સર ટાસ્ક બનાવો
    const osThreadAttr_t parserTask_attributes = {
        .name = "GsmParser",
        .stack_size = 512 * 4,
        .priority = (osPriority_t) osPriorityHigh, // હાઈ પ્રાયોરિટી રાખવી
    };
    hgsm->parser_task_handle = osThreadNew(Quectel_Parser_Task, hgsm, &parserTask_attributes);

    // 🔴 4. મોડેમમાંથી ડેટા પકડવા માટે DMA (Idle line) ચાલુ કરો
    HAL_UARTEx_ReceiveToIdle_DMA(hgsm->huart, hgsm->dma_rx_buffer, sizeof(hgsm->dma_rx_buffer));
    __HAL_DMA_DISABLE_IT(hgsm->huart->hdmarx, DMA_IT_HT); // નકામા હાફ-ઇન્ટરપ્ટ બંધ કરો
}

/* ==================================================================== */
/* 2. UART DMA Callback (ISR માંથી અહી ડેટા આવશે)                       */
/* ==================================================================== */
void Quectel_UART_RxCpltCallback(Quectel_Handle_t *hgsm, uint16_t Size)
{
    // DMA માં જે ડેટા આવ્યો તે તરત જ Ring Buffer માં નાખી દો
    lwrb_write(&hgsm->rx_ring_buffer, hgsm->dma_rx_buffer, Size);

    // પાર્સર ટાસ્કને જગાડો (કે ભાઈ નવો ડેટા આવ્યો છે, વાંચી લે!)
    osSemaphoreRelease(hgsm->parser_sem);

    // બીજી વાર ડેટા લેવા માટે DMA ફરીથી ચાલુ કરો
    HAL_UARTEx_ReceiveToIdle_DMA(hgsm->huart, hgsm->dma_rx_buffer, sizeof(hgsm->dma_rx_buffer));
    __HAL_DMA_DISABLE_IT(hgsm->huart->hdmarx, DMA_IT_HT);
}

/* ==================================================================== */
/* 3. The Parser Task (આ ટાસ્ક હંમેશા બેકગ્રાઉન્ડમાં ચાલતો રહેશે)       */
/* ==================================================================== */
static void Quectel_Parser_Task(void *argument)
{
    Quectel_Handle_t *hgsm = (Quectel_Handle_t *)argument;
    uint8_t ch;
    char current_line[256];
    uint16_t line_idx = 0;

    for(;;)
    {
        // નવો ડેટા આવે ત્યાં સુધી સુઈ જાવ
        osSemaphoreAcquire(hgsm->parser_sem, osWaitForever);

        // બફરમાંથી એક-એક અક્ષર વાંચો
        while (lwrb_read(&hgsm->rx_ring_buffer, &ch, 1) == 1)
        {
            if (ch == '\n' || ch == '\r') // લાઈન પૂરી થાય ત્યારે
            {
                if (line_idx > 0)
                {
                    current_line[line_idx] = '\0'; // સ્ટ્રિંગને ટર્મિનેટ કરો

                    // 🔴 ચેક કરો કે આપણે જે જવાબની રાહ જોતા હતા (દા.ત. "OK") તે આ લાઈનમાં છે?
                    if (strlen(hgsm->expected_response) > 0 && strstr(current_line, hgsm->expected_response) != NULL)
                    {
                        hgsm->cmd_status = GSM_OK;
                        osSemaphoreRelease(hgsm->cmd_sem); // કમાન્ડ મોકલનારને જગાડો
                        hgsm->expected_response[0] = '\0'; // ક્લીયર કરો
                    }
                    else if (strlen(hgsm->expected_response) > 0 && strstr(current_line, "ERROR") != NULL)
                    {
                        hgsm->cmd_status = GSM_ERROR;
                        osSemaphoreRelease(hgsm->cmd_sem);
                        hgsm->expected_response[0] = '\0';
                    }

                    /* 💡 (ભવિષ્યમાં અહી MQTT અને HTTP ના આવતા મેસેજ પકડવાનું લોજિક આવશે) */

                    line_idx = 0; // નવી લાઈન માટે 0 કરો
                }
            }
            else // લાઈન ચાલુ હોય તો અક્ષર ભેગા કરો
            {
                if (line_idx < sizeof(current_line) - 1)
                {
                    current_line[line_idx++] = (char)ch;
                }
            }
        }
    }
}

/* ==================================================================== */
/* 4. AT Command Engine (કમાન્ડ મોકલવા અને જવાબની રાહ જોવી)              */
/* ==================================================================== */
GSM_Status_t Quectel_Send_AT_Command(Quectel_Handle_t *hgsm, const char* cmd, const char* expected_response, uint32_t timeout_ms)
{
    // 1. જૂના કોઈ સેમાફોર પડ્યા હોય તો તેને ક્લીયર કરો
    while (osSemaphoreAcquire(hgsm->cmd_sem, 0) == osOK) { /* Do nothing */ }
    
    // 2. આપણે કયા જવાબની રાહ જોઈએ છીએ તે સેટ કરો (દા.ત. "OK")
    if (expected_response != NULL) {
        strncpy(hgsm->expected_response, expected_response, sizeof(hgsm->expected_response) - 1);
    } else {
        hgsm->expected_response[0] = '\0'; // કોઈ રાહ જોવાની નથી
    }
    
    hgsm->cmd_status = GSM_TIMEOUT; // ડિફોલ્ટ સ્ટેટસ
    
    // 3. મોડેમને કમાન્ડ મોકલો
    HAL_UART_Transmit(hgsm->huart, (uint8_t*)cmd, strlen(cmd), 100);
    
    // 4. જો આપણે જવાબ જોઈતો હોય, તો ટાસ્કને સુવડાવી દો (Timeout સુધી)
    if (expected_response != NULL) 
    {
        // 🔴 અહી તમારો ટાસ્ક સુઈ જશે. પાર્સર ટાસ્ક તેને જગાડશે.
        if (osSemaphoreAcquire(hgsm->cmd_sem, timeout_ms) == osOK) 
        {
            return hgsm->cmd_status; // (GSM_OK અથવા GSM_ERROR)
        } 
        else 
        {
            // જો ટાઈમઆઉટ થઈ જાય તો
            hgsm->expected_response[0] = '\0';
            return GSM_TIMEOUT;
        }
    }
    
    return GSM_OK; // જો કોઈ જવાબ ના જોતો હોય તો સીધું OK આપી દો
}

// =====================================================================
// 🔴 AUTO-RECOVERY FROM UART ERRORS (DMA MODE)
// =====================================================================

extern uint32_t DBG_RxCount;
extern uint32_t DBG_ErrCount;
extern uint32_t DBG_LastErrorCode;
extern char DBG_LastData[50];

extern Quectel_Handle_t MyModem;

void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART2) 
    {
        DBG_ErrCount++; // એરર કાઉન્ટર વધારો
        DBG_LastErrorCode = huart->ErrorCode; // કઈ એરર આવી તે સેવ કરો (1 = PE, 2 = FE, 4 = NE, 8 = ORE)

        // તમારો જૂનો કોડ...
        __HAL_UART_CLEAR_FLAG(huart, UART_CLEAR_OREF | UART_CLEAR_NEF | UART_CLEAR_PEF | UART_CLEAR_FEF);
        huart->ErrorCode = HAL_UART_ERROR_NONE;
        HAL_UARTEx_ReceiveToIdle_DMA(huart, MyModem.dma_rx_buffer, sizeof(MyModem.dma_rx_buffer));
        __HAL_DMA_DISABLE_IT(huart->hdmarx, DMA_IT_HT); 
    }
}