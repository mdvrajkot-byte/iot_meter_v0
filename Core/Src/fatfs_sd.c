/*-----------------------------------------------------------------------*/
/* SPI Master Control for FatFs to Drive SD Card                         */
/*-----------------------------------------------------------------------*/

#include "fatfs_sd.h"

// =======================================================
// SD Card Commands (SPI Mode)
// =======================================================
#define CMD0     (0x40+0)     /* GO_IDLE_STATE */
#define CMD1     (0x40+1)     /* SEND_OP_COND */
#define CMD8     (0x40+8)     /* SEND_IF_COND */
#define CMD9     (0x40+9)     /* SEND_CSD */
#define CMD10    (0x40+10)    /* SEND_CID */
#define CMD12    (0x40+12)    /* STOP_TRANSMISSION */
#define CMD16    (0x40+16)    /* SET_BLOCKLEN */
#define CMD17    (0x40+17)    /* READ_SINGLE_BLOCK */
#define CMD18    (0x40+18)    /* READ_MULTIPLE_BLOCK */
#define CMD23    (0x40+23)    /* SET_BLOCK_COUNT */
#define CMD24    (0x40+24)    /* WRITE_BLOCK */
#define CMD25    (0x40+25)    /* WRITE_MULTIPLE_BLOCK */
#define CMD41    (0x40+41)    /* SEND_OP_COND (ACMD) */
#define CMD55    (0x40+55)    /* APP_CMD */
#define CMD58    (0x40+58)    /* READ_OCR */

/* CS Control Macros */
#define SD_CS_LOW()     HAL_GPIO_WritePin(SD_CS_PORT, SD_CS_PIN, GPIO_PIN_RESET)
#define SD_CS_HIGH()    HAL_GPIO_WritePin(SD_CS_PORT, SD_CS_PIN, GPIO_PIN_SET)

static volatile DSTATUS Stat = STA_NOINIT;    /* Disk status */
static uint8_t CardType;                      /* Card type flags */

// =======================================================
// Low Level SPI Functions
// =======================================================

static uint8_t SPI_TxRx(uint8_t data)
{
    uint8_t rx_data;
    HAL_SPI_TransmitReceive(HSPI_SDCARD, &data, &rx_data, 1, 100);
    return rx_data;
}

static int SD_ReadyWait(void)
{
    uint8_t res;
    uint32_t timeout = HAL_GetTick() + 500; 
    
    SPI_TxRx(0xFF);
    do {
        res = SPI_TxRx(0xFF);
    } while (res != 0xFF && HAL_GetTick() < timeout);
    
    return (res == 0xFF) ? 1 : 0;
}

static uint8_t SD_SendCmd(uint8_t cmd, uint32_t arg)
{
    uint8_t n, res;
    
    if (cmd & 0x80) {
        cmd &= 0x7F;
        res = SD_SendCmd(CMD55, 0);
        if (res > 1) return res;
    }
    
    SD_CS_HIGH();
    SPI_TxRx(0xFF);
    SD_CS_LOW();
    SPI_TxRx(0xFF);
    
    SPI_TxRx(cmd);                  
    SPI_TxRx((uint8_t)(arg >> 24)); 
    SPI_TxRx((uint8_t)(arg >> 16)); 
    SPI_TxRx((uint8_t)(arg >> 8));  
    SPI_TxRx((uint8_t)arg);         
    
    n = 0x01;                       
    if (cmd == CMD0) n = 0x95;      
    if (cmd == CMD8) n = 0x87;      
    SPI_TxRx(n);
    
    n = 255;
    do {
        res = SPI_TxRx(0xFF);
    } while ((res & 0x80) && --n);
    
    return res;
}

// =======================================================
// FatFs Required Functions (ધ બ્રિજ)
// =======================================================

DSTATUS SD_disk_initialize(BYTE pdrv)
{
ty = 0;
    uint8_t res_cmd0 = SD_SendCmd(CMD0, 0); // રિસ્પોન્સ પકડો
    
    // ----- નવો ડિબગ કોડ શરૂ (Mutex સાથે) -----
    extern osMutexId_t lcdMutexHandle; // main.c માંથી Mutex લાવો
    extern void lcd_clear(void);
    extern void lcd_put_cur(int row, int col);
    extern void lcd_send_string(char *str);
    char debugBuf[16];
    
    // Mutex મેળવો
    if (osMutexAcquire(lcdMutexHandle, osWaitForever) == osOK) {
        lcd_clear();
        lcd_put_cur(0, 0);
        lcd_send_string("CMD0 Response:");
        
        sprintf(debugBuf, "HEX: 0x%02X", res_cmd0);
        lcd_put_cur(1, 0);
        lcd_send_string(debugBuf);
        
        osMutexRelease(lcdMutexHandle); // Mutex પાછો આપો
    }
    
    HAL_Delay(4000); // 4 સેકન્ડ માટે સિસ્ટમ થોભાવો જેથી સ્ક્રીન વાંચી શકાય
    // ----- નવો ડિબગ કોડ પૂરો -----

    if (res_cmd0 == 1) {
        timeout = HAL_GetTick() + 1000;
}

DSTATUS SD_disk_status(BYTE pdrv)
{
    if (pdrv != 0) return STA_NOINIT;
    return Stat;
}

DRESULT SD_disk_read(BYTE pdrv, BYTE* buff, DWORD sector, UINT count)
{
    if (pdrv != 0 || !count) return RES_PARERR;
    if (Stat & STA_NOINIT) return RES_NOTRDY;
    
    if (!(CardType & 2)) sector *= 512; 
    
    if (count == 1) {
        if (SD_SendCmd(CMD17, sector) == 0) {
            if (SD_ReadyWait()) {
                uint32_t timeout = HAL_GetTick() + 100;
                while (SPI_TxRx(0xFF) != 0xFE && HAL_GetTick() < timeout);
                if (HAL_GetTick() < timeout) {
                    HAL_SPI_Receive(HSPI_SDCARD, buff, 512, 100);
                    SPI_TxRx(0xFF); 
                    SPI_TxRx(0xFF);
                    count = 0;
                }
            }
        }
    }
    
    SD_CS_HIGH();
    SPI_TxRx(0xFF);
    return count ? RES_ERROR : RES_OK;
}

DRESULT SD_disk_write(BYTE pdrv, const BYTE* buff, DWORD sector, UINT count)
{
    if (pdrv != 0 || !count) return RES_PARERR;
    if (Stat & STA_NOINIT) return RES_NOTRDY;
    if (Stat & STA_PROTECT) return RES_WRPRT;
    
    if (!(CardType & 2)) sector *= 512;
    
    if (count == 1) {
        if (SD_SendCmd(CMD24, sector) == 0) {
            SPI_TxRx(0xFF);
            SPI_TxRx(0xFE); 
            HAL_SPI_Transmit(HSPI_SDCARD, (BYTE*)buff, 512, 100);
            SPI_TxRx(0xFF); 
            SPI_TxRx(0xFF);
            
            if ((SPI_TxRx(0xFF) & 0x1F) == 0x05) { 
                SD_ReadyWait(); 
                count = 0;
            }
        }
    }
    
    SD_CS_HIGH();
    SPI_TxRx(0xFF);
    return count ? RES_ERROR : RES_OK;
}

DRESULT SD_disk_ioctl(BYTE pdrv, BYTE cmd, void* buff)
{
    DRESULT res = RES_ERROR;
    
    if (pdrv != 0) return RES_PARERR;
    if (Stat & STA_NOINIT) return RES_NOTRDY;
    
    switch (cmd) {
        case CTRL_SYNC:
            if (SD_ReadyWait()) res = RES_OK;
            break;
        case GET_SECTOR_COUNT:
            *(DWORD*)buff = 131072; 
            res = RES_OK;
            break;
        case GET_SECTOR_SIZE:
            *(WORD*)buff = 512;
            res = RES_OK;
            break;
        case GET_BLOCK_SIZE:
            *(DWORD*)buff = 1;
            res = RES_OK;
            break;
        default:
            res = RES_PARERR;
    }
    
    SD_CS_HIGH();
    SPI_TxRx(0xFF);
    return res;
}