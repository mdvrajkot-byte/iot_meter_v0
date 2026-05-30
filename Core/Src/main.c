/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "cmsis_os.h"
#include "app_fatfs.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "lcd.h"     // હવે આ લાઈન ક્યારેય ગાયબ નહિ થાય!
#include <string.h>
#include <stdio.h>  
#include "quectel_ec200.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
typedef struct {   
    int wifi_communication_on;      // 1 = ચાલુ, 0 = બંધ
    int modbus_communication_on;    // 1 = ચાલુ, 0 = બંધ
    int gsm_communication_on;       // 1 = ચાલુ, 0 = બંધ
    int battery_healthy;
} MeterData_t;
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc1;

I2S_HandleTypeDef hi2s1;

RTC_HandleTypeDef hrtc;

SPI_HandleTypeDef hspi2;

UART_HandleTypeDef huart1;
UART_HandleTypeDef huart2;
UART_HandleTypeDef huart3;
DMA_HandleTypeDef hdma_usart2_rx;

/* Definitions for defaultTask */
osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = {
  .name = "defaultTask",
  .priority = (osPriority_t) osPriorityNormal,
  .stack_size = 128 * 4
};
/* Definitions for GSM_Task */
osThreadId_t GSM_TaskHandle;
const osThreadAttr_t GSM_Task_attributes = {
  .name = "GSM_Task",
  .priority = (osPriority_t) osPriorityNormal,
  .stack_size = 256 * 4
};
/* Definitions for LCD_Task */
osThreadId_t LCD_TaskHandle;
const osThreadAttr_t LCD_Task_attributes = {
  .name = "LCD_Task",
  .priority = (osPriority_t) osPriorityBelowNormal,
  .stack_size = 256 * 4
};
/* Definitions for LED_Task */
osThreadId_t LED_TaskHandle;
const osThreadAttr_t LED_Task_attributes = {
  .name = "LED_Task",
  .priority = (osPriority_t) osPriorityLow,
  .stack_size = 128 * 4
};
/* Definitions for Modbus_Task */
osThreadId_t Modbus_TaskHandle;
const osThreadAttr_t Modbus_Task_attributes = {
  .name = "Modbus_Task",
  .priority = (osPriority_t) osPriorityAboveNormal,
  .stack_size = 512 * 4
};
/* Definitions for SD_Task */
osThreadId_t SD_TaskHandle;
const osThreadAttr_t SD_Task_attributes = {
  .name = "SD_Task",
  .priority = (osPriority_t) osPriorityNormal,
  .stack_size = 1024 * 4
};
/* Definitions for Battery_Task */
osThreadId_t Battery_TaskHandle;
const osThreadAttr_t Battery_Task_attributes = {
  .name = "Battery_Task",
  .priority = (osPriority_t) osPriorityLow,
  .stack_size = 128 * 4
};
/* Definitions for ESP_Task */
osThreadId_t ESP_TaskHandle;
const osThreadAttr_t ESP_Task_attributes = {
  .name = "ESP_Task",
  .priority = (osPriority_t) osPriorityLow,
  .stack_size = 512 * 4
};
/* Definitions for lcdMutex */
osMutexId_t lcdMutexHandle;
const osMutexAttr_t lcdMutex_attributes = {
  .name = "lcdMutex"
};
/* USER CODE BEGIN PV */
MeterData_t myMeter = {0};
extern osMutexId_t lcdMutexHandle;
Quectel_Handle_t MyModem; // 🔴 આ આપણું માસ્ટર મોડેમ હેન્ડલ છે
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_ADC1_Init(void);
static void MX_RTC_Init(void);
static void MX_SPI2_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_USART3_UART_Init(void);
static void MX_I2S1_Init(void);
void StartDefaultTask(void *argument);
void StartGsmTask(void *argument);
void StartLcdTask(void *argument);
void StartLedTask(void *argument);
void StartModbusTask(void *argument);
void StartSdTask(void *argument);
void StartBatteryTask(void *argument);
void StartEspTask(void *argument);

/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */
  HAL_Delay(2000);
  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_ADC1_Init();
  MX_RTC_Init();
  MX_SPI2_Init();
  MX_USART1_UART_Init();
  MX_USART2_UART_Init();
  MX_USART3_UART_Init();
  if (MX_FATFS_Init() != APP_OK) {
    Error_Handler();
  }
  MX_I2S1_Init();
  /* USER CODE BEGIN 2 */
  
  HAL_GPIO_WritePin(mcu_charge_GPIO_Port, mcu_charge_Pin, GPIO_PIN_SET);
  
/* USER CODE BEGIN 2 */
  
  HAL_GPIO_WritePin(mcu_charge_GPIO_Port, mcu_charge_Pin, GPIO_PIN_SET);
  HAL_GPIO_WritePin(mcu_lcd_led_GPIO_Port, mcu_lcd_led_Pin, GPIO_PIN_SET);

  Quectel_Init(&MyModem, &huart2);
  /* USER CODE END 2 */

  /* Init scheduler */
  osKernelInitialize();
  /* Create the mutex(es) */
  /* creation of lcdMutex */
  lcdMutexHandle = osMutexNew(&lcdMutex_attributes);

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of defaultTask */
  defaultTaskHandle = osThreadNew(StartDefaultTask, NULL, &defaultTask_attributes);

  /* creation of GSM_Task */
  GSM_TaskHandle = osThreadNew(StartGsmTask, NULL, &GSM_Task_attributes);

  /* creation of LCD_Task */
  LCD_TaskHandle = osThreadNew(StartLcdTask, NULL, &LCD_Task_attributes);

  /* creation of LED_Task */
  LED_TaskHandle = osThreadNew(StartLedTask, NULL, &LED_Task_attributes);

  /* creation of Modbus_Task */
  Modbus_TaskHandle = osThreadNew(StartModbusTask, NULL, &Modbus_Task_attributes);

  /* creation of SD_Task */
  SD_TaskHandle = osThreadNew(StartSdTask, NULL, &SD_Task_attributes);

  /* creation of Battery_Task */
  Battery_TaskHandle = osThreadNew(StartBatteryTask, NULL, &Battery_Task_attributes);

  /* creation of ESP_Task */
  ESP_TaskHandle = osThreadNew(StartEspTask, NULL, &ESP_Task_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

  /* Start scheduler */
  osKernelStart();

  /* We should never get here as control is now taken by the scheduler */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {

    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI|RCC_OSCILLATORTYPE_LSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSIDiv = RCC_HSI_DIV1;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief ADC1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC1_Init(void)
{

  /* USER CODE BEGIN ADC1_Init 0 */

  /* USER CODE END ADC1_Init 0 */

  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC1_Init 1 */

  /* USER CODE END ADC1_Init 1 */

  /** Configure the global features of the ADC (Clock, Resolution, Data Alignment and number of conversion)
  */
  hadc1.Instance = ADC1;
  hadc1.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV2;
  hadc1.Init.Resolution = ADC_RESOLUTION_12B;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.ScanConvMode = ADC_SCAN_DISABLE;
  hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  hadc1.Init.LowPowerAutoWait = DISABLE;
  hadc1.Init.LowPowerAutoPowerOff = DISABLE;
  hadc1.Init.ContinuousConvMode = DISABLE;
  hadc1.Init.NbrOfConversion = 1;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc1.Init.DMAContinuousRequests = DISABLE;
  hadc1.Init.Overrun = ADC_OVR_DATA_PRESERVED;
  hadc1.Init.SamplingTimeCommon1 = ADC_SAMPLETIME_1CYCLE_5;
  hadc1.Init.SamplingTimeCommon2 = ADC_SAMPLETIME_1CYCLE_5;
  hadc1.Init.OversamplingMode = DISABLE;
  hadc1.Init.TriggerFrequencyMode = ADC_TRIGGER_FREQ_HIGH;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_0;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLINGTIME_COMMON_1;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC1_Init 2 */

  /* USER CODE END ADC1_Init 2 */

}

/**
  * @brief I2S1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2S1_Init(void)
{

  /* USER CODE BEGIN I2S1_Init 0 */

  /* USER CODE END I2S1_Init 0 */

  /* USER CODE BEGIN I2S1_Init 1 */

  /* USER CODE END I2S1_Init 1 */
  hi2s1.Instance = SPI1;
  hi2s1.Init.Mode = I2S_MODE_SLAVE_TX;
  hi2s1.Init.Standard = I2S_STANDARD_PHILIPS;
  hi2s1.Init.DataFormat = I2S_DATAFORMAT_16B;
  hi2s1.Init.MCLKOutput = I2S_MCLKOUTPUT_DISABLE;
  hi2s1.Init.AudioFreq = I2S_AUDIOFREQ_8K;
  hi2s1.Init.CPOL = I2S_CPOL_LOW;
  if (HAL_I2S_Init(&hi2s1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2S1_Init 2 */

  /* USER CODE END I2S1_Init 2 */

}

/**
  * @brief RTC Initialization Function
  * @param None
  * @retval None
  */
static void MX_RTC_Init(void)
{

  /* USER CODE BEGIN RTC_Init 0 */

  /* USER CODE END RTC_Init 0 */

  /* USER CODE BEGIN RTC_Init 1 */

  /* USER CODE END RTC_Init 1 */

  /** Initialize RTC Only
  */
  hrtc.Instance = RTC;
  hrtc.Init.HourFormat = RTC_HOURFORMAT_24;
  hrtc.Init.AsynchPrediv = 127;
  hrtc.Init.SynchPrediv = 255;
  hrtc.Init.OutPut = RTC_OUTPUT_DISABLE;
  hrtc.Init.OutPutRemap = RTC_OUTPUT_REMAP_NONE;
  hrtc.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
  hrtc.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;
  hrtc.Init.OutPutPullUp = RTC_OUTPUT_PULLUP_NONE;
  if (HAL_RTC_Init(&hrtc) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN RTC_Init 2 */

  /* USER CODE END RTC_Init 2 */

}

/**
  * @brief SPI2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI2_Init(void)
{

  /* USER CODE BEGIN SPI2_Init 0 */

  /* USER CODE END SPI2_Init 0 */

  /* USER CODE BEGIN SPI2_Init 1 */

  /* USER CODE END SPI2_Init 1 */
  /* SPI2 parameter configuration*/
  hspi2.Instance = SPI2;
  hspi2.Init.Mode = SPI_MODE_MASTER;
  hspi2.Init.Direction = SPI_DIRECTION_2LINES;
  hspi2.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi2.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi2.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi2.Init.NSS = SPI_NSS_SOFT;
  hspi2.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_128;
  hspi2.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi2.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi2.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi2.Init.CRCPolynomial = 7;
  hspi2.Init.CRCLength = SPI_CRC_LENGTH_DATASIZE;
  hspi2.Init.NSSPMode = SPI_NSS_PULSE_DISABLE;
  if (HAL_SPI_Init(&hspi2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI2_Init 2 */

  /* USER CODE END SPI2_Init 2 */

}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  huart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart1.Init.ClockPrescaler = UART_PRESCALER_DIV1;
  huart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetTxFifoThreshold(&huart1, UART_TXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetRxFifoThreshold(&huart1, UART_RXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_DisableFifoMode(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  huart2.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart2.Init.ClockPrescaler = UART_PRESCALER_DIV1;
  huart2.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetTxFifoThreshold(&huart2, UART_TXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetRxFifoThreshold(&huart2, UART_RXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_DisableFifoMode(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * @brief USART3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART3_UART_Init(void)
{

  /* USER CODE BEGIN USART3_Init 0 */

  /* USER CODE END USART3_Init 0 */

  /* USER CODE BEGIN USART3_Init 1 */

  /* USER CODE END USART3_Init 1 */
  huart3.Instance = USART3;
  huart3.Init.BaudRate = 115200;
  huart3.Init.WordLength = UART_WORDLENGTH_8B;
  huart3.Init.StopBits = UART_STOPBITS_1;
  huart3.Init.Parity = UART_PARITY_NONE;
  huart3.Init.Mode = UART_MODE_TX_RX;
  huart3.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart3.Init.OverSampling = UART_OVERSAMPLING_16;
  huart3.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart3.Init.ClockPrescaler = UART_PRESCALER_DIV1;
  huart3.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart3) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART3_Init 2 */

  /* USER CODE END USART3_Init 2 */

}

/**
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void)
{

  /* DMA controller clock enable */
  __HAL_RCC_DMA1_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA1_Channel1_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel1_IRQn, 3, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel1_IRQn);

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, mcv_vlt_control_Pin|mcu_esp_wakeup_Pin|mcu_display_on_off_sw_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, mcu_uart_gpio_Pin|mcu_led_modbus_Pin|mcu_charge_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, mcu_gsm_pwr_Pin|mcu_gsm_status_Pin|mcu_led_wifi_Pin|mcu_sd_cs_Pin
                          |mcu_lcd_d5_Pin|mcu_lcd_d6_Pin|mcu_lcd_d7_Pin|mcu_lcd_led_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOD, mcu_led_gsm_Pin|mcu_lcd_rs_Pin|mcu_lcd_en_Pin|mcu_lcd_d4_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pins : mcv_vlt_control_Pin mcu_esp_wakeup_Pin mcu_display_on_off_sw_Pin */
  GPIO_InitStruct.Pin = mcv_vlt_control_Pin|mcu_esp_wakeup_Pin|mcu_display_on_off_sw_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pin : mcu_gsm_ri_Pin */
  GPIO_InitStruct.Pin = mcu_gsm_ri_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(mcu_gsm_ri_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : mcu_uart_gpio_Pin mcu_led_modbus_Pin mcu_charge_Pin */
  GPIO_InitStruct.Pin = mcu_uart_gpio_Pin|mcu_led_modbus_Pin|mcu_charge_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : mcu_gsm_wakeup_Pin */
  GPIO_InitStruct.Pin = mcu_gsm_wakeup_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(mcu_gsm_wakeup_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : mcu_gsm_pwr_Pin mcu_gsm_status_Pin mcu_led_wifi_Pin mcu_sd_cs_Pin
                           mcu_lcd_d5_Pin mcu_lcd_d6_Pin mcu_lcd_d7_Pin mcu_lcd_led_Pin */
  GPIO_InitStruct.Pin = mcu_gsm_pwr_Pin|mcu_gsm_status_Pin|mcu_led_wifi_Pin|mcu_sd_cs_Pin
                          |mcu_lcd_d5_Pin|mcu_lcd_d6_Pin|mcu_lcd_d7_Pin|mcu_lcd_led_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : mcu_sd_gpio_Pin */
  GPIO_InitStruct.Pin = mcu_sd_gpio_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(mcu_sd_gpio_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : esp_mcu_wakeup_Pin mcu_stdby_Pin */
  GPIO_InitStruct.Pin = esp_mcu_wakeup_Pin|mcu_stdby_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : mcu_led_gsm_Pin mcu_lcd_rs_Pin mcu_lcd_en_Pin mcu_lcd_d4_Pin */
  GPIO_InitStruct.Pin = mcu_led_gsm_Pin|mcu_lcd_rs_Pin|mcu_lcd_en_Pin|mcu_lcd_d4_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */
void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
{
    // જો ડેટા huart2 (મોડેમ) માંથી આવ્યો હોય, તો આપણી લાઇબ્રેરીને આપો
    if (huart->Instance == USART2) 
    {
        Quectel_UART_RxCpltCallback(&MyModem, Size);
    }
}
/* USER CODE END 4 */

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void *argument)
{
  /* USER CODE BEGIN 5 */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END 5 */
}

/* USER CODE BEGIN Header_StartGsmTask */
/**
* @brief Function implementing the GSM_Task thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartGsmTask */

void StartGsmTask(void *argument)
{
  /* USER CODE BEGIN StartGsmTask */
  osDelay(2000); 
  uint8_t fail_count = 0; // ફેલ થવાનું કાઉન્ટર
  
  /* Infinite loop */
  for(;;)
  {

    // કમાન્ડ મોકલો
    if (Quectel_Send_AT_Command(&MyModem, "AT\r\n", "OK", 1000) == GSM_OK) 
    {
        fail_count = 0; // જો OK આવે તો કાઉન્ટર ઝીરો કરી દો
        
        if (osMutexAcquire(lcdMutexHandle, osWaitForever) == osOK) {
            lcd_clear();
            lcd_put_cur(0, 0);
            lcd_send_string("Modem OK");
            osMutexRelease(lcdMutexHandle);
        }
    } 
    else 
    {
        fail_count++; // જો ફેલ થાય તો કાઉન્ટર વધારો
        
        if (osMutexAcquire(lcdMutexHandle, osWaitForever) == osOK) {
            lcd_clear();
            lcd_put_cur(0, 0);
            lcd_send_string("Modem Fail");
            osMutexRelease(lcdMutexHandle);
        }

        // 🔴 AUTO-RECOVERY: જો સળંગ 3 વાર ફેલ જાય, તો મોડેમને રીબૂટ કરો!
        if (fail_count >= 3) 
        {
            if (osMutexAcquire(lcdMutexHandle, osWaitForever) == osOK) {
                lcd_put_cur(1, 0); lcd_send_string("Re-Booting...");
                osMutexRelease(lcdMutexHandle);
            }
            
            // 2.1 સેકન્ડનો બૂટ પલ્સ
            HAL_GPIO_WritePin(mcu_gsm_pwr_GPIO_Port, mcu_gsm_pwr_Pin, GPIO_PIN_SET); 
            osDelay(2100); 
            HAL_GPIO_WritePin(mcu_gsm_pwr_GPIO_Port, mcu_gsm_pwr_Pin, GPIO_PIN_RESET);

            osDelay(10000); // બુટ થવાનો સમય આપો
            fail_count = 0; // રીબૂટ કર્યા પછી કાઉન્ટર પાછું ઝીરો કરો
        }
    }
    
    osDelay(2000); // 2 સેકન્ડનો વિરામ
  }
  /* USER CODE END StartGsmTask */
}

/* USER CODE BEGIN Header_StartLcdTask */
/**
* @brief Function implementing the LCD_Task thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartLcdTask */
void StartLcdTask(void *argument)
{
  /* USER CODE BEGIN StartLcdTask */
  osDelay(100);
  lcd_init();
  lcd_clear();         

  osDelay(100);
  lcd_init();               
  lcd_clear();              

  HAL_GPIO_WritePin(mcu_lcd_led_GPIO_Port, mcu_lcd_led_Pin, GPIO_PIN_SET); 
  lcd_put_cur(0, 0);
  lcd_send_string("SMART METER V1");
  lcd_put_cur(1, 0);
  lcd_send_string("System Booting..");
  
  // આ મેસેજને ૨ સેકન્ડ (2000 ms) માટે સ્ક્રીન પર રહેવા દો
  osDelay(2000); 

    /* Infinite loop */
  for(;;)
  {
      osDelay(1000);
  }
  /* USER CODE END StartLcdTask */
}

/* USER CODE BEGIN Header_StartLedTask */
/**
* @brief Function implementing the LED_Task thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartLedTask */
void StartLedTask(void *argument)
{
  /* USER CODE BEGIN StartLedTask */

  uint32_t battery_timer = 0;  
  
  /* Infinite loop */
  for(;;)
  {
    if (myMeter.wifi_communication_on == 1)
    {
      HAL_GPIO_WritePin(mcu_led_wifi_GPIO_Port, mcu_led_wifi_Pin, GPIO_PIN_RESET); // ON
    }
    else
    {
      HAL_GPIO_WritePin(mcu_led_wifi_GPIO_Port, mcu_led_wifi_Pin, GPIO_PIN_SET);   // OFF
    }
    if (myMeter.modbus_communication_on  == 1)
    {
        HAL_GPIO_WritePin(mcu_led_modbus_GPIO_Port, mcu_led_modbus_Pin, GPIO_PIN_RESET); // ON
    }
    else
    {
        HAL_GPIO_WritePin(mcu_led_modbus_GPIO_Port, mcu_led_modbus_Pin, GPIO_PIN_SET);   // OFF
    }
    if (myMeter.gsm_communication_on  == 1)
    {
        HAL_GPIO_WritePin(mcu_led_gsm_GPIO_Port, mcu_led_gsm_Pin, GPIO_PIN_RESET); // ON
    }
    else
    {
        HAL_GPIO_WritePin(mcu_led_gsm_GPIO_Port, mcu_led_gsm_Pin, GPIO_PIN_SET);   // OFF
    }
    if (myMeter.battery_healthy == 1)
    {
        if (battery_timer >= 10000)
        {

            battery_timer = 0; // ટાઈમર રીસેટ
        }
    }
    else
    {
        battery_timer = 0;
    }    
    osDelay(1000);
    battery_timer += 100;
  }
  /* USER CODE END StartLedTask */
}

/* USER CODE BEGIN Header_StartModbusTask */
/**
* @brief Function implementing the Modbus_Task thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartModbusTask */
void StartModbusTask(void *argument)
{
  /* USER CODE BEGIN StartModbusTask */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END StartModbusTask */
}

/* USER CODE BEGIN Header_StartSdTask */
/**
* @brief Function implementing the SD_Task thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartSdTask */
void StartSdTask(void *argument)
{
  /* USER CODE BEGIN StartSdTask */
  FATFS fs;
  FIL fil;
  FRESULT fres;
  UINT bytesWrote;
  uint8_t is_mounted = 0; // કાર્ડ માઉન્ટ થયેલ છે કે નહીં તે જાણવા માટે
  /* Infinite loop */
  for(;;)
  {
      // ૧. SD કાર્ડ ફિઝિકલ ડિટેક્શન
      if (HAL_GPIO_ReadPin(mcu_sd_gpio_GPIO_Port, mcu_sd_gpio_Pin) != GPIO_PIN_RESET) 
      {
          if (is_mounted)
          {
              f_mount(NULL, "", 0); // કાર્ડ નીકળી ગયું તો અનમાઉન્ટ કરો
              is_mounted = 0;
          }
          osDelay(1000);
          continue; 
      }

      // ૨. કાર્ડ માઉન્ટિંગ (માત્ર એક જ વાર)
      if (!is_mounted) 
      {
          fres = f_mount(&fs, "", 1);
          if (fres == FR_OK)
          {
              is_mounted = 1;
          } 
          else
          {
              osDelay(1000);
              continue;
          }
      }
      // ૩. ફાઈલ ઓપરેશન
      fres = f_open(&fil, "test.txt", FA_WRITE | FA_OPEN_ALWAYS | FA_OPEN_APPEND);
      if (fres == FR_OK) 
      {
          char myData[] = "SD OK! via RTOS\n"; 
          f_write(&fil, myData, strlen(myData), &bytesWrote);
          f_close(&fil); 
          osDelay(3000); // દર ૧૦ સેકન્ડે ડેટા લખો
      }
      osDelay(10000); // દર ૧૦ સેકન્ડે ડેટા લખો
  }
  osDelay(2000);
  /* USER CODE END StartSdTask */
}

/* USER CODE BEGIN Header_StartBatteryTask */
/**
* @brief Function implementing the Battery_Task thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartBatteryTask */
void StartBatteryTask(void *argument)
{
  /* USER CODE BEGIN StartBatteryTask */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END StartBatteryTask */
}

/* USER CODE BEGIN Header_StartEspTask */
/**
* @brief Function implementing the ESP_Task thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartEspTask */
void StartEspTask(void *argument)
{
  /* USER CODE BEGIN StartEspTask */
  char at_command[] = "AT\r\n";
  uint8_t rx_buffer[15];
    /* Infinite loop */
  for(;;)
  {
      memset(rx_buffer, 0, sizeof(rx_buffer));
      HAL_UART_Transmit(&huart3, (uint8_t*)at_command, strlen(at_command), 100);
      HAL_StatusTypeDef status = HAL_UART_Receive(&huart3, rx_buffer, 5, 1000);
      osDelay(10000);
  }
  /* USER CODE END StartEspTask */
}

/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM1 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM1)
  {
    HAL_IncTick();
  }
  /* USER CODE BEGIN Callback 1 */

  /* USER CODE END Callback 1 */
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
