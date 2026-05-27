/*-----------------------------------------------------------------------*/
/* SPI Master Control for FatFs to Drive SD Card                         */
/*-----------------------------------------------------------------------*/

#include "fatfs_sd.h"
#include <stdio.h>
#include <string.h>

/* External UART for debug logging */
extern UART_HandleTypeDef huart1;

/* Debug logging function */
static void debug_log(const char* msg)
{
    HAL_UART_Transmit(&huart1, (uint8_t*)msg, strlen(msg), 1000);
    HAL_UART_Transmit(&huart1, (uint8_t*)"\r\n", 2, 1000);
}

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
    uint32_t timeout = HAL_GetTick() + 2000; 
    
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
    HAL_Delay(1);
    SD_CS_LOW();
    HAL_Delay(1);
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
    
    HAL_Delay(1);
    return res;
}

// =======================================================
// FatFs Required Functions (ધ બ્રિજ)
// =======================================================

DSTATUS SD_disk_initialize(BYTE pdrv)
{
    uint8_t n, cmd, ty, ocr[4];
    uint32_t timeout;
    char buf[64];
    
    if (pdrv != 0) return STA_NOINIT;
    
    debug_log("[SD] Initialize start");
    
    SD_CS_HIGH();
    HAL_Delay(10);
    
    for (n = 80; n; n--) SPI_TxRx(0xFF);
    debug_log("[SD] Sent 80 clock cycles");
    
    ty = 0;
    n = SD_SendCmd(CMD0, 0);
    sprintf(buf, "[SD] CMD0 response: 0x%02X", n);
    debug_log(buf);
    
    if (n == 1) {
        debug_log("[SD] CMD0 OK - Idle state");
        timeout = HAL_GetTick() + 1000;
        
        n = SD_SendCmd(CMD8, 0x1AA);
        sprintf(buf, "[SD] CMD8 response: 0x%02X", n);
        debug_log(buf);
        
        if (n == 1) {
            debug_log("[SD] CMD8 OK - SD v2 card detected");
            for (n = 0; n < 4; n++) ocr[n] = SPI_TxRx(0xFF);
            if (ocr[2] == 0x01 && ocr[3] == 0xAA) {
                debug_log("[SD] Voltage range OK, sending CMD41");
                while (HAL_GetTick() < timeout && SD_SendCmd(CMD41, 1UL << 30));
                if (HAL_GetTick() < timeout) {
                    n = SD_SendCmd(CMD58, 0);
                    sprintf(buf, "[SD] CMD58 response: 0x%02X", n);
                    debug_log(buf);
                    if (n == 0) {
                        for (n = 0; n < 4; n++) ocr[n] = SPI_TxRx(0xFF);
                        ty = (ocr[0] & 0x40) ? 3 : 2;
                        sprintf(buf, "[SD] Card type: %d", ty);
                        debug_log(buf);
                    }
                } else {
                    debug_log("[SD] CMD41 timeout");
                }
            } else {
                debug_log("[SD] Voltage range mismatch");
            }
        } else {
            debug_log("[SD] CMD8 failed - SD v1 card, trying CMD1");
            cmd = (SD_SendCmd(CMD55, 0) <= 1 && SD_SendCmd(CMD41, 0) <= 1) ? CMD41 : CMD1;
            while (HAL_GetTick() < timeout && SD_SendCmd(cmd, 0));
            if (HAL_GetTick() < timeout) {
                ty = 1;
                SD_SendCmd(CMD16, 512);
                debug_log("[SD] SD v1 card initialized");
            } else {
                debug_log("[SD] CMD1/CMD41 timeout");
            }
        }
    } else {
        debug_log("[SD] CMD0 failed!");
    }
    
    CardType = ty;
    SD_CS_HIGH();
    SPI_TxRx(0xFF);
    
    if (ty) {
        Stat &= ~STA_NOINIT;
        debug_log("[SD] Init SUCCESS");
    } else {
        Stat |= STA_NOINIT;
        debug_log("[SD] Init FAILED");
    }
    
    return Stat;
}

DSTATUS SD_disk_status(BYTE pdrv)
{
    if (pdrv != 0) return STA_NOINIT;
    return Stat;
}

DRESULT SD_disk_read(BYTE pdrv, BYTE* buff, DWORD sector, UINT count)
{
    char buf[64];
    if (pdrv != 0 || !count) return RES_PARERR;
    if (Stat & STA_NOINIT) {
        debug_log("[SD] Read: Disk not initialized");
        return RES_NOTRDY;
    }
    
    sprintf(buf, "[SD] Read sector %lu, count %d", sector, count);
    debug_log(buf);
    
    if (!(CardType & 2)) sector *= 512; 
    
    if (count == 1) {
        if (SD_SendCmd(CMD17, sector) == 0) {
            if (SD_ReadyWait()) {
                SD_CS_LOW();
                HAL_Delay(2);
                uint32_t timeout = HAL_GetTick() + 500;
                while (SPI_TxRx(0xFF) != 0xFE && HAL_GetTick() < timeout);
                if (HAL_GetTick() < timeout) {
                    HAL_SPI_Receive(HSPI_SDCARD, buff, 512, 500);
                    SPI_TxRx(0xFF); 
                    SPI_TxRx(0xFF);
                    count = 0;
                    debug_log("[SD] Read OK");
                } else {
                    debug_log("[SD] Read timeout - no data token");
                }
            } else {
                debug_log("[SD] Read: Card not ready");
            }
        } else {
            debug_log("[SD] Read: CMD17 failed");
        }
    }
    
    SD_CS_HIGH();
    SPI_TxRx(0xFF);
    return count ? RES_ERROR : RES_OK;
}

DRESULT SD_disk_write(BYTE pdrv, const BYTE* buff, DWORD sector, UINT count)
{
    char buf[64];
    if (pdrv != 0 || !count) return RES_PARERR;
    if (Stat & STA_NOINIT) {
        debug_log("[SD] Write: Disk not initialized");
        return RES_NOTRDY;
    }
    if (Stat & STA_PROTECT) {
        debug_log("[SD] Write: Disk is write protected");
        return RES_WRPRT;
    }
    
    sprintf(buf, "[SD] Write sector %lu, count %d", sector, count);
    debug_log(buf);
    
    if (!(CardType & 2)) sector *= 512;
    
    if (count == 1) {
        if (SD_SendCmd(CMD24, sector) == 0) {
            SD_CS_LOW();
            HAL_Delay(2);
            SPI_TxRx(0xFF);
            SPI_TxRx(0xFE); 
            HAL_SPI_Transmit(HSPI_SDCARD, (BYTE*)buff, 512, 500);
            SPI_TxRx(0xFF); 
            SPI_TxRx(0xFF);
            HAL_Delay(1);
            
            if ((SPI_TxRx(0xFF) & 0x1F) == 0x05) { 
                SD_ReadyWait(); 
                count = 0;
                debug_log("[SD] Write OK");
            } else {
                debug_log("[SD] Write: Bad response token");
            }
        } else {
            debug_log("[SD] Write: CMD24 failed");
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