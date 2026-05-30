/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32g0xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define mcv_vlt_control_Pin GPIO_PIN_13
#define mcv_vlt_control_GPIO_Port GPIOC
#define mcu_vlt_measure_Pin GPIO_PIN_0
#define mcu_vlt_measure_GPIO_Port GPIOA
#define mcu_gsm_tx_Pin GPIO_PIN_2
#define mcu_gsm_tx_GPIO_Port GPIOA
#define mcu_gsm_rx_Pin GPIO_PIN_3
#define mcu_gsm_rx_GPIO_Port GPIOA
#define mcu_gsm_ri_Pin GPIO_PIN_5
#define mcu_gsm_ri_GPIO_Port GPIOA
#define mcu_uart_gpio_Pin GPIO_PIN_7
#define mcu_uart_gpio_GPIO_Port GPIOA
#define mcu_gsm_wakeup_Pin GPIO_PIN_0
#define mcu_gsm_wakeup_GPIO_Port GPIOB
#define mcu_gsm_pwr_Pin GPIO_PIN_1
#define mcu_gsm_pwr_GPIO_Port GPIOB
#define mcu_gsm_status_Pin GPIO_PIN_2
#define mcu_gsm_status_GPIO_Port GPIOB
#define mcu_led_wifi_Pin GPIO_PIN_10
#define mcu_led_wifi_GPIO_Port GPIOB
#define mcu_sd_gpio_Pin GPIO_PIN_11
#define mcu_sd_gpio_GPIO_Port GPIOB
#define mcu_sd_cs_Pin GPIO_PIN_12
#define mcu_sd_cs_GPIO_Port GPIOB
#define mcu_sd_sck_Pin GPIO_PIN_13
#define mcu_sd_sck_GPIO_Port GPIOB
#define mcu_sd_miso_Pin GPIO_PIN_14
#define mcu_sd_miso_GPIO_Port GPIOB
#define mcu_sd_mosi_Pin GPIO_PIN_15
#define mcu_sd_mosi_GPIO_Port GPIOB
#define mcu_led_modbus_Pin GPIO_PIN_8
#define mcu_led_modbus_GPIO_Port GPIOA
#define mcu_uart_tx_Pin GPIO_PIN_9
#define mcu_uart_tx_GPIO_Port GPIOA
#define mcu_esp_wakeup_Pin GPIO_PIN_6
#define mcu_esp_wakeup_GPIO_Port GPIOC
#define mcu_display_on_off_sw_Pin GPIO_PIN_7
#define mcu_display_on_off_sw_GPIO_Port GPIOC
#define mcu_uart_rx_Pin GPIO_PIN_10
#define mcu_uart_rx_GPIO_Port GPIOA
#define esp_mcu_wakeup_Pin GPIO_PIN_11
#define esp_mcu_wakeup_GPIO_Port GPIOA
#define mcu_stdby_Pin GPIO_PIN_12
#define mcu_stdby_GPIO_Port GPIOA
#define mcu_charge_Pin GPIO_PIN_15
#define mcu_charge_GPIO_Port GPIOA
#define mcu_led_gsm_Pin GPIO_PIN_0
#define mcu_led_gsm_GPIO_Port GPIOD
#define mcu_lcd_rs_Pin GPIO_PIN_1
#define mcu_lcd_rs_GPIO_Port GPIOD
#define mcu_lcd_en_Pin GPIO_PIN_2
#define mcu_lcd_en_GPIO_Port GPIOD
#define mcu_lcd_d4_Pin GPIO_PIN_3
#define mcu_lcd_d4_GPIO_Port GPIOD
#define mcu_lcd_d5_Pin GPIO_PIN_3
#define mcu_lcd_d5_GPIO_Port GPIOB
#define mcu_lcd_d6_Pin GPIO_PIN_4
#define mcu_lcd_d6_GPIO_Port GPIOB
#define mcu_lcd_d7_Pin GPIO_PIN_6
#define mcu_lcd_d7_GPIO_Port GPIOB
#define mcu_lcd_led_Pin GPIO_PIN_7
#define mcu_lcd_led_GPIO_Port GPIOB
#define mcu_esp_tx_Pin GPIO_PIN_8
#define mcu_esp_tx_GPIO_Port GPIOB
#define mcu_esp_rx_Pin GPIO_PIN_9
#define mcu_esp_rx_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
