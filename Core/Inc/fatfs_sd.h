#ifndef __FATFS_SD_H
#define __FATFS_SD_H

#include "main.h"           // <--- આ લાઈન ઉમેરો! (સૌથી અગત્યનું)
#include "stm32g0xx_hal.h"
#include "diskio.h"

// ==========================================
// હાર્ડવેર સેટિંગ્સ (તમારા બોર્ડ અને CubeMX મુજબ)
// ==========================================
// તમારું SPI2 છે, એટલે આપણે hspi2 વાપરીશું
#define HSPI_SDCARD &hspi2  

// તમારી CS પિનનું નામ mcu_sd_cs_Pin છે, જે GPIOB પર છે
#define SD_CS_PORT mcu_sd_cs_GPIO_Port
#define SD_CS_PIN  mcu_sd_cs_Pin
// ==========================================

extern SPI_HandleTypeDef hspi2;

/* ડ્રાઈવર ફંક્શન્સ (Engine Functions) */
DSTATUS SD_disk_initialize (BYTE pdrv);
DSTATUS SD_disk_status (BYTE pdrv);
DRESULT SD_disk_read (BYTE pdrv, BYTE* buff, DWORD sector, UINT count);
DRESULT SD_disk_write (BYTE pdrv, const BYTE* buff, DWORD sector, UINT count);
DRESULT SD_disk_ioctl (BYTE pdrv, BYTE cmd, void* buff);

#endif