#ifndef QUECTEL_EC200_H
#define QUECTEL_EC200_H

#include "stm32g0xx_hal.h"
#include "cmsis_os.h"
#include "lwrb.h"
#include <stdint.h>
#include <stdbool.h>

/* 🔴 કમાન્ડના સ્ટેટસ માટે */
typedef enum {
    GSM_OK = 0,
    GSM_ERROR,
    GSM_TIMEOUT,
    GSM_BUSY
} GSM_Status_t;

/* 🔴 આ આપણું "Dynamic" ઓબ્જેક્ટ છે (આ સૌથી અગત્યનું છે) */
typedef struct {
    // 1. હાર્ડવેર લિંક
    UART_HandleTypeDef *huart;       // કયા UART પર મોડેમ છે?
    
    // 2. મેમરી અને બફર (lwrb)
    lwrb_t rx_ring_buffer;           // રીંગ બફરનો કંટ્રોલ
    uint8_t rx_buffer_data[1024];    // 1 KB મેઈન મેમરી
    uint8_t dma_rx_buffer[256];      // DMA માટે ટેમ્પરરી મેમરી
    
    // 3. FreeRTOS કંટ્રોલ (ટાસ્ક અને સેમાફોર)
    osThreadId_t parser_task_handle; // બેકગ્રાઉન્ડ ટાસ્કનું હેન્ડલ
    osSemaphoreId_t parser_sem;      // ટાસ્કને જગાડવા માટે
    osSemaphoreId_t cmd_sem;         // કમાન્ડની રાહ જોવા માટે
    
    // 4. AT કમાન્ડ ટ્રેકિંગ
    char expected_response[64];      // આપણે કયા જવાબની રાહ જોઈએ છીએ (દા.ત. "OK")
    volatile GSM_Status_t cmd_status;// કમાન્ડનું રિઝલ્ટ શું આવ્યું?
    
    // 5. મોડેમનું પોતાનું સ્ટેટસ
    bool is_ready;
    bool is_network_registered;
    uint8_t signal_strength;
    
} Quectel_Handle_t;


/* ------------------------------------------------------------- */
/* પબ્લિક ફંક્શન્સ (Public APIs)                                 */
/* ------------------------------------------------------------- */

// લાઇબ્રેરી ચાલુ કરવા માટે
void Quectel_Init(Quectel_Handle_t *hgsm, UART_HandleTypeDef *huart);

// ઇન્ટરપ્ટ (ISR) માંથી ડેટા આપવા માટે
void Quectel_UART_RxCpltCallback(Quectel_Handle_t *hgsm, uint16_t Size);
GSM_Status_t Quectel_Send_AT_Command(Quectel_Handle_t *hgsm, const char* cmd, const char* expected_response, uint32_t timeout_ms);
void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart);

#endif /* QUECTEL_EC200_H */