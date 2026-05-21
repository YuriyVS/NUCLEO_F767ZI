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
#include "lwip.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "lwip/tcp.h"
#include <string.h>
#include <stdio.h>
#include "DB_Parameters.h"
#include "DB_Main.h"
#include "lwip/apps/httpd.h" // Обязательно для httpd_init()
#include "web_server.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
CAN_HandleTypeDef hcan1;

CRC_HandleTypeDef hcrc;

UART_HandleTypeDef huart3;
UART_HandleTypeDef huart6;
DMA_HandleTypeDef hdma_usart3_rx;
DMA_HandleTypeDef hdma_usart6_rx;

PCD_HandleTypeDef hpcd_USB_OTG_FS;

/* USER CODE BEGIN PV */
extern UART_HandleTypeDef huart3; // Кажемо компілятору: "huart3 оголошена десь в іншому місці"
extern UART_HandleTypeDef huart6; // Кажемо компілятору: "huart3 оголошена десь в іншому місці"
uint16_t Modbus_Registers[10] = {0}; // Наші Holding Registers
uint8_t RTU_Rx_Buf[256];             // Буфер для прийому по UART
uint8_t rx_buf_usart6[256];
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_USART3_UART_Init(void);
static void MX_USB_OTG_FS_PCD_Init(void);
static void MX_CAN1_Init(void);
static void MX_CRC_Init(void);
static void MX_USART6_UART_Init(void);
/* USER CODE BEGIN PFP */
void MPU_Config(void);
uint16_t Modbus_CRC16(uint8_t *buffer, uint16_t length);
//void Modbus_RTU_Parse(uint8_t *rx_data, uint16_t length);
void Modbus_RTU_Parse(UART_HandleTypeDef *huart, uint8_t *rx_data, uint16_t length);
void CAN1_Init_User(void);


/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

void UART3_SendString(char* str);
err_t modbus_recv_callback(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err);
err_t modbus_accept_callback(void *arg, struct tcp_pcb *newpcb, err_t err);
void ModbusTCP_Init(void); // Запуск сервера

__attribute__((section(".dtcm_data"))) volatile float a = 1.23f, b = 4.56f, c;
__attribute__((section(".dtcm_data"))) volatile double a_d = 1.23, b_d = 4.56, c_d;
  volatile uint32_t start_f, end_f, delta_f;
  volatile uint32_t start_d, end_d, delta_d;
  volatile uint32_t start_f, end_f, delta_f;
    volatile uint32_t start_d, end_d, delta_d;
    volatile uint32_t total_overhead, end_ex, start_ex = 255, count =0, average;

    static inline void CalculateThyristorPhasef(void) __attribute__((always_inline));
    static inline void CalculateThyristorPhasef(void) {
    	c = a / b; //c = sinf(a);//c = a / b;
    }

    __attribute__((section(".itcm_text")))
        void CalculateThyristorPhased(void) {
    	c_d = a_d / b_d; //c_d = sinf(a_d);//c = a / b;
        }

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */
	MPU_Config();

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_USART3_UART_Init();
  MX_USB_OTG_FS_PCD_Init();
  MX_LWIP_Init();
  MX_CAN1_Init();
  MX_CRC_Init();
  MX_USART6_UART_Init();
  /* USER CODE BEGIN 2 */
  CAN1_Init_User();
  ModbusTCP_Init();
  // ЗАПУСК ВЕБ-СЕРВЕРА - проверьте наличие этой строки!
  httpd_init();

  // Запуск прийому по UART3 з використанням DMA та визначенням паузи (Idle Line)
  HAL_UARTEx_ReceiveToIdle_DMA(&huart3, RTU_Rx_Buf, 256);
  __HAL_DMA_DISABLE_IT(&hdma_usart3_rx, DMA_IT_HT); // Вимикаємо переривання Half Transfer
  // Запуск для ESP32-C3
    HAL_UARTEx_ReceiveToIdle_DMA(&huart6, rx_buf_usart6, sizeof(rx_buf_usart6));
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  CAN_TxHeaderTypeDef TxHeader;
  uint8_t TxData[8];
  uint32_t TxMailbox;
  uint32_t last_can_tick = 0;

  // Налаштовуємо заголовок тестового пакету
  TxHeader.StdId = 0x123;           // ID нашого повідомлення
  TxHeader.RTR = CAN_RTR_DATA;      // Це дані, а не запит
  TxHeader.IDE = CAN_ID_STD;        // Стандартний ID (11 біт)
  TxHeader.DLC = 8;                 // Довжина даних - 8 байт
  TxHeader.TransmitGlobalTime = DISABLE;
  //UART3_SendString("System Started. Waiting for Modbus TCP/RTU...\r\n");
  // 1. Разрешаем доступ к TRC (это у вас уже было верно)
  CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
  __DSB(); // Ждем завершения операции

  // 2. РАЗБЛОКИРОВКА DWT (тот самый секретный шаг для M7)
  // 0xC5ACCE55 — это "Unlock Key" для всех блоков CoreSight
  DWT->LAR = 0xC5ACCE55;

  // 2. Сбрасываем счетчик тактов
  DWT->CYCCNT = 0;

  DWT->CPICNT = 0;
  DWT->EXCCNT = 0;
  DWT->LSUCNT = 0;
  DWT->FOLDCNT = 0;

  // 3. Включаем счетчик (используем правильные имена CTRL и DWT_CTRL_...)
  DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;

  DWT->CTRL |= DWT_CTRL_CPIEVTENA_Msk   | // Было CPIENA
          DWT_CTRL_EXCTRCENA_Msk | DWT_CTRL_EXCEVTENA_Msk   | //
          DWT_CTRL_LSUEVTENA_Msk   | // Было LSUENA
          DWT_CTRL_FOLDEVTENA_Msk;   // Было FOLDENA

  DBParameters = DBParametersFactory;
  DBMain.b96.Save_to_Flash = 0;
  DBMain.b96.Read_from_Flash = 0;
  while (1)
  {

	  // 1. ГОЛОВНЕ: Обробка мережевого стека LwIP.
	      // Без цієї функції Modbus TCP не буде відповідати!
	      MX_LWIP_Process();

	      // 2. (Опціонально) "Heartbeat" - миготіння світлодіодом LD1 (зелений),
	      // щоб візуально бачити, що програма не зависла.
	      static uint32_t last_heartbeat = 0;
	      if (HAL_GetTick() - last_heartbeat > 1000) {
	    	  Modbus_Registers[1]++; // Збільшуємо значення
	          LL_GPIO_TogglePin(LD1_GPIO_Port, LD1_Pin);
	          last_heartbeat = HAL_GetTick();
	          //UART3_SendString("System working...\r\n");
	          char msg[32];
	          sprintf(msg, "Heartbeat Reg[1]: %d\r\n", Modbus_Registers[1]);
	          //UART3_SendString(msg);

	      }
	      // --- ВІДПРАВКА В CAN ---
	      // 3. Таймер №2: Відправка в CAN - раз на 500 мс (використовуємо ту саму змінну)
	      if (HAL_GetTick() - last_can_tick >= 500) {
	      // Підготуємо дані (наприклад, копіюємо інкремент у CAN)
	         TxData[0] = (Modbus_Registers[1] >> 8) & 0xFF;
	         TxData[1] = Modbus_Registers[1] & 0xFF;
	         TxData[2] = 0xAA; // Просто мітка для тесту
	      	 TxData[3] = 0xBB;

	      	 // Відправляємо в чергу CAN
	      	 if (HAL_CAN_AddTxMessage(&hcan1, &TxHeader, TxData, &TxMailbox) != HAL_OK) {
	      	                      // Якщо помилка (черга повна), можна вивести в лог
	      	 }
	      	 last_can_tick = HAL_GetTick(); // Тепер змінна використовується!
	      }

	      if (DBMain.b96.Save_to_Flash) {
	      	      DB_SaveToFlash();
	      	      DBMain.b96.Save_to_Flash = false;
	      }
	      if (DBMain.b96.Read_from_Flash) {
	    	  	  DB_ReadFromFlash();
	      	      DBMain.b96.Read_from_Flash = false;
	      }

	      Process_Alarm_Log();
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
	      c = a / b; // Проверьте в Disassembly, должна быть инструкция VMUL.F32
	      c_d = a_d / b_d; // Если здесь будет инструкция VMUL.F64 — значит DP работает.
	      start_f = DWT->CYCCNT;
	      CalculateThyristorPhasef(); //c = sinf(a);//c = a / b; // Проверьте в Disassembly, должна быть инструкция VMUL.F32
	      end_f = DWT->CYCCNT;
	      delta_f = end_f - start_f;

	      start_d = DWT->CYCCNT;
	      c_d = a_d / b_d; //CalculateThyristorPhased(); //c_d = sin(a_d);//c_d = a_d / b_d; // Если здесь будет инструкция VMUL.F64 — значит DP работает.
	                       // Если будет BL __aeabi_dmul — это медленная эмуляция.
	      end_d = DWT->CYCCNT;
	      delta_d = end_d - start_d;
	      delta_d = delta_d;

	      //DWT->EXCCNT = 0; // Сброс
	      //start_ex = DWT->EXCCNT;

	      //HAL_Delay(1); // Ждем 10 мс. За это время произойдет ровно 10 прерываний SysTick

	      //end_ex = DWT->EXCCNT;
	      if(DWT->EXCCNT > end_ex)
	      {
	    	  end_ex = DWT->EXCCNT;
	      }
	      if(DWT->EXCCNT > 0)
	      {
	    	  if(DWT->EXCCNT < start_ex){start_ex = DWT->EXCCNT;}
	    	  total_overhead += DWT->EXCCNT;
	    	  count += 1;
	    	  average = total_overhead / count;
	    	  DWT->EXCCNT = 0;
	    	  Modbus_Registers[0] = average;
	    	  Modbus_Registers[2] = end_ex;
	    	  //Modbus_Registers[3] = start_ex;

	      }

  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  LL_FLASH_SetLatency(LL_FLASH_LATENCY_3);
  while(LL_FLASH_GetLatency()!= LL_FLASH_LATENCY_3)
  {
  }
  LL_PWR_SetRegulVoltageScaling(LL_PWR_REGU_VOLTAGE_SCALE3);
  LL_PWR_EnableOverDriveMode();
  LL_RCC_HSE_EnableBypass();
  LL_RCC_HSE_Enable();

   /* Wait till HSE is ready */
  while(LL_RCC_HSE_IsReady() != 1)
  {

  }
  LL_RCC_PLL_ConfigDomain_SYS(LL_RCC_PLLSOURCE_HSE, LL_RCC_PLLM_DIV_4, 96, LL_RCC_PLLP_DIV_2);
  LL_RCC_PLL_ConfigDomain_48M(LL_RCC_PLLSOURCE_HSE, LL_RCC_PLLM_DIV_4, 96, LL_RCC_PLLQ_DIV_4);
  LL_RCC_PLL_Enable();

   /* Wait till PLL is ready */
  while(LL_RCC_PLL_IsReady() != 1)
  {

  }
  while (LL_PWR_IsActiveFlag_VOS() == 0)
  {
  }
  LL_RCC_SetAHBPrescaler(LL_RCC_SYSCLK_DIV_1);
  LL_RCC_SetAPB1Prescaler(LL_RCC_APB1_DIV_2);
  LL_RCC_SetAPB2Prescaler(LL_RCC_APB2_DIV_1);
  LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_PLL);

   /* Wait till System clock is ready */
  while(LL_RCC_GetSysClkSource() != LL_RCC_SYS_CLKSOURCE_STATUS_PLL)
  {

  }
  LL_SetSystemCoreClock(96000000);

   /* Update the time base */
  if (HAL_InitTick (TICK_INT_PRIORITY) != HAL_OK)
  {
    Error_Handler();
  }

   /* Set Timers Clock Prescalers */
  LL_RCC_SetTIMPrescaler(LL_RCC_TIM_PRESCALER_TWICE);
}

/**
  * @brief CAN1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_CAN1_Init(void)
{

  /* USER CODE BEGIN CAN1_Init 0 */

  /* USER CODE END CAN1_Init 0 */

  /* USER CODE BEGIN CAN1_Init 1 */

  /* USER CODE END CAN1_Init 1 */
  hcan1.Instance = CAN1;
  hcan1.Init.Prescaler = 6;
  hcan1.Init.Mode = CAN_MODE_LOOPBACK;
  hcan1.Init.SyncJumpWidth = CAN_SJW_1TQ;
  hcan1.Init.TimeSeg1 = CAN_BS1_13TQ;
  hcan1.Init.TimeSeg2 = CAN_BS2_2TQ;
  hcan1.Init.TimeTriggeredMode = DISABLE;
  hcan1.Init.AutoBusOff = ENABLE;
  hcan1.Init.AutoWakeUp = DISABLE;
  hcan1.Init.AutoRetransmission = ENABLE;
  hcan1.Init.ReceiveFifoLocked = DISABLE;
  hcan1.Init.TransmitFifoPriority = DISABLE;
  if (HAL_CAN_Init(&hcan1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN CAN1_Init 2 */

  /* USER CODE END CAN1_Init 2 */

}

/**
  * @brief CRC Initialization Function
  * @param None
  * @retval None
  */
static void MX_CRC_Init(void)
{

  /* USER CODE BEGIN CRC_Init 0 */

  /* USER CODE END CRC_Init 0 */

  /* USER CODE BEGIN CRC_Init 1 */

  /* USER CODE END CRC_Init 1 */
  hcrc.Instance = CRC;
  hcrc.Init.DefaultPolynomialUse = DEFAULT_POLYNOMIAL_ENABLE;
  hcrc.Init.DefaultInitValueUse = DEFAULT_INIT_VALUE_ENABLE;
  hcrc.Init.InputDataInversionMode = CRC_INPUTDATA_INVERSION_NONE;
  hcrc.Init.OutputDataInversionMode = CRC_OUTPUTDATA_INVERSION_DISABLE;
  hcrc.InputDataFormat = CRC_INPUTDATA_FORMAT_BYTES;
  if (HAL_CRC_Init(&hcrc) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN CRC_Init 2 */

  /* USER CODE END CRC_Init 2 */

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
  huart3.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart3) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART3_Init 2 */

  /* USER CODE END USART3_Init 2 */

}

/**
  * @brief USART6 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART6_UART_Init(void)
{

  /* USER CODE BEGIN USART6_Init 0 */

  /* USER CODE END USART6_Init 0 */

  /* USER CODE BEGIN USART6_Init 1 */

  /* USER CODE END USART6_Init 1 */
  huart6.Instance = USART6;
  huart6.Init.BaudRate = 115200;
  huart6.Init.WordLength = UART_WORDLENGTH_8B;
  huart6.Init.StopBits = UART_STOPBITS_1;
  huart6.Init.Parity = UART_PARITY_NONE;
  huart6.Init.Mode = UART_MODE_TX_RX;
  huart6.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart6.Init.OverSampling = UART_OVERSAMPLING_16;
  huart6.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart6.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart6) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART6_Init 2 */

  /* USER CODE END USART6_Init 2 */

}

/**
  * @brief USB_OTG_FS Initialization Function
  * @param None
  * @retval None
  */
static void MX_USB_OTG_FS_PCD_Init(void)
{

  /* USER CODE BEGIN USB_OTG_FS_Init 0 */

  /* USER CODE END USB_OTG_FS_Init 0 */

  /* USER CODE BEGIN USB_OTG_FS_Init 1 */

  /* USER CODE END USB_OTG_FS_Init 1 */
  hpcd_USB_OTG_FS.Instance = USB_OTG_FS;
  hpcd_USB_OTG_FS.Init.dev_endpoints = 6;
  hpcd_USB_OTG_FS.Init.speed = PCD_SPEED_FULL;
  hpcd_USB_OTG_FS.Init.dma_enable = DISABLE;
  hpcd_USB_OTG_FS.Init.phy_itface = PCD_PHY_EMBEDDED;
  hpcd_USB_OTG_FS.Init.Sof_enable = ENABLE;
  hpcd_USB_OTG_FS.Init.low_power_enable = DISABLE;
  hpcd_USB_OTG_FS.Init.lpm_enable = DISABLE;
  hpcd_USB_OTG_FS.Init.vbus_sensing_enable = ENABLE;
  hpcd_USB_OTG_FS.Init.use_dedicated_ep1 = DISABLE;
  if (HAL_PCD_Init(&hpcd_USB_OTG_FS) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USB_OTG_FS_Init 2 */

  /* USER CODE END USB_OTG_FS_Init 2 */

}

/**
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void)
{

  /* DMA controller clock enable */
  __HAL_RCC_DMA1_CLK_ENABLE();
  __HAL_RCC_DMA2_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA1_Stream1_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Stream1_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Stream1_IRQn);
  /* DMA2_Stream1_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA2_Stream1_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA2_Stream1_IRQn);

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  LL_EXTI_InitTypeDef EXTI_InitStruct = {0};
  LL_GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOC);
  LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOH);
  LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOA);
  LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOB);
  LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOD);
  LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOG);

  /**/
  LL_GPIO_ResetOutputPin(LD1_GPIO_Port, LD1_Pin);

  /**/
  LL_GPIO_ResetOutputPin(LD3_GPIO_Port, LD3_Pin);

  /**/
  LL_GPIO_ResetOutputPin(USB_PowerSwitchOn_GPIO_Port, USB_PowerSwitchOn_Pin);

  /**/
  LL_GPIO_ResetOutputPin(LD2_GPIO_Port, LD2_Pin);

  /**/
  LL_SYSCFG_SetEXTISource(LL_SYSCFG_EXTI_PORTC, LL_SYSCFG_EXTI_LINE13);

  /**/
  EXTI_InitStruct.Line_0_31 = LL_EXTI_LINE_13;
  EXTI_InitStruct.LineCommand = ENABLE;
  EXTI_InitStruct.Mode = LL_EXTI_MODE_IT;
  EXTI_InitStruct.Trigger = LL_EXTI_TRIGGER_RISING;
  LL_EXTI_Init(&EXTI_InitStruct);

  /**/
  LL_GPIO_SetPinPull(USER_Btn_GPIO_Port, USER_Btn_Pin, LL_GPIO_PULL_NO);

  /**/
  LL_GPIO_SetPinMode(USER_Btn_GPIO_Port, USER_Btn_Pin, LL_GPIO_MODE_INPUT);

  /**/
  GPIO_InitStruct.Pin = LD1_Pin;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  LL_GPIO_Init(LD1_GPIO_Port, &GPIO_InitStruct);

  /**/
  GPIO_InitStruct.Pin = LD3_Pin;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  LL_GPIO_Init(LD3_GPIO_Port, &GPIO_InitStruct);

  /**/
  GPIO_InitStruct.Pin = USB_PowerSwitchOn_Pin;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  LL_GPIO_Init(USB_PowerSwitchOn_GPIO_Port, &GPIO_InitStruct);

  /**/
  GPIO_InitStruct.Pin = USB_OverCurrent_Pin;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  LL_GPIO_Init(USB_OverCurrent_GPIO_Port, &GPIO_InitStruct);

  /**/
  GPIO_InitStruct.Pin = LD2_Pin;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  LL_GPIO_Init(LD2_GPIO_Port, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */
void MPU_Config(void)
{
  MPU_Region_InitTypeDef MPU_InitStruct = {0};

  // Вимикаємо MPU
  HAL_MPU_Disable();

  // Налаштовуємо регіон для Ethernet дескрипторів (SRAM)
  // Вони мають бути Non-cacheable
  MPU_InitStruct.Enable = MPU_REGION_ENABLE;
  MPU_InitStruct.Number = MPU_REGION_NUMBER0;
  MPU_InitStruct.BaseAddress = 0x2007C000; // Адреса з вашого шаблону
  MPU_InitStruct.Size = MPU_REGION_SIZE_16KB;
  MPU_InitStruct.SubRegionDisable = 0x00;
  MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;
  MPU_InitStruct.DisableExec = MPU_ACCESS_NOT_SHAREABLE;
  MPU_InitStruct.IsShareable = MPU_ACCESS_SHAREABLE;
  MPU_InitStruct.IsCacheable = MPU_ACCESS_NOT_CACHEABLE;
  MPU_InitStruct.IsBufferable = MPU_ACCESS_NOT_BUFFERABLE;

  HAL_MPU_ConfigRegion(&MPU_InitStruct);
  // Вмикаємо MPU назад
  HAL_MPU_Enable(MPU_PRIVILEGED_DEFAULT);
}

uint16_t Modbus_CRC16(uint8_t *buffer, uint16_t length) {
    uint16_t crc = 0xFFFF;
    for (int i = 0; i < length; i++) {
        crc ^= (uint16_t)buffer[i];
        for (int j = 8; j != 0; j--) {
            if ((crc & 0x0001) != 0) {
                crc >>= 1;
                crc ^= 0xA001;
            } else {
                crc >>= 1;
            }
        }
    }
    return crc;
}

//void Modbus_RTU_Parse(uint8_t *rx_data, uint16_t length) {
//    if (length < 8) return; // Мінімальний пакет RTU
//
//    uint16_t msg_crc = (rx_data[length-1] << 8) | rx_data[length-2];
//    if (Modbus_CRC16(rx_data, length - 2) != msg_crc) return;
//
//    if (rx_data[0] != 1) return; // Перевірка Slave ID (наприклад, 1)
//
//    uint8_t f_code = rx_data[1];
//    uint8_t tx_buf[64];
//    uint16_t tx_len = 0;
//
//    if (f_code == 0x03) { // Read Holding Registers
//        uint16_t start_addr = (rx_data[2] << 8) | rx_data[3];
//        uint16_t reg_count = (rx_data[4] << 8) | rx_data[5];
//
//        tx_buf[0] = 1; tx_buf[1] = 0x03; tx_buf[2] = reg_count * 2;
//        for (int i = 0; i < reg_count; i++) {
//            tx_buf[3 + i*2] = (Modbus_Registers[start_addr + i] >> 8) & 0xFF;
//            tx_buf[4 + i*2] = Modbus_Registers[start_addr + i] & 0xFF;
//        }
//        tx_len = 3 + (reg_count * 2);
//    }
//    else if (f_code == 0x06) { // Write Single Register
//        uint16_t reg_addr = (rx_data[2] << 8) | rx_data[3];
//        uint16_t reg_val = (rx_data[4] << 8) | rx_data[5];
//        if (reg_addr < 10) {
//            Modbus_Registers[reg_addr] = reg_val;
//            if (reg_addr == 3) { // Синій світлодіод
//                if (reg_val == 1) LL_GPIO_SetOutputPin(LD2_GPIO_Port, LD2_Pin);
//                else LL_GPIO_ResetOutputPin(LD2_GPIO_Port, LD2_Pin);
//            }
//        }
//        memcpy(tx_buf, rx_data, 6); tx_len = 6;
//    }
//
//    if (tx_len > 0) {
//        uint16_t res_crc = Modbus_CRC16(tx_buf, tx_len);
//        tx_buf[tx_len++] = res_crc & 0xFF;
//        tx_buf[tx_len++] = (res_crc >> 8) & 0xFF;
//        HAL_UART_Transmit(&huart3, tx_buf, tx_len, 100);
//    }
//}

void Modbus_RTU_Parse(UART_HandleTypeDef *huart,uint8_t *rx_data, uint16_t length) {
    if (length < 8) return;

    // Проверка CRC и Slave ID (1)
    if (Modbus_CRC16(rx_data, length - 2) != ((rx_data[length-1] << 8) | rx_data[length-2])) return;
    if (rx_data[0] != 1) return;

    uint8_t f_code = rx_data[1];
    uint16_t start_addr = (rx_data[2] << 8) | rx_data[3];
    uint16_t reg_count = (rx_data[4] << 8) | rx_data[5];

    uint8_t tx_buf[256];
    uint16_t tx_len = 0;
    uint16_t *base_ptr = NULL;

    // Выбор базы данных
    if (start_addr < 1000) {
        base_ptr = (uint16_t *)&DBParameters;
    } else {
        base_ptr = (uint16_t *)&DBMain;
        start_addr -= 1000;
    }

    // --- Функция 03: Чтение ---
    if (f_code == 0x03) {
        tx_buf[0] = 1; tx_buf[1] = 0x03; tx_buf[2] = reg_count * 2;
        for (int i = 0; i < reg_count; i++) {
            uint16_t val = base_ptr[start_addr + i];
            tx_buf[3 + i*2] = (val >> 8) & 0xFF;
            tx_buf[4 + i*2] = val & 0xFF;
        }
        tx_len = 3 + (reg_count * 2);
    }
    // --- Функция 16 (0x10): Запись нескольких регистров ---
    // Weintek шлет именно её для 32-битных данных
    else if (f_code == 0x10) {
        for (int i = 0; i < reg_count; i++) {
            base_ptr[start_addr + i] = (rx_data[7 + i*2] << 8) | rx_data[8 + i*2];
        }

        // Если писали в параметры — обновляем CRC и сохраняем
        if (base_ptr == (uint16_t *)&DBParameters) {
            DB_UpdateCRC();
            // DB_SaveToFlash(); // Опционально: сохранять сразу или по кнопке
        }

        memcpy(tx_buf, rx_data, 6);
        tx_len = 6;
    }

    // Отправка ответа
    if (tx_len > 0) {
        uint16_t res_crc = Modbus_CRC16(tx_buf, tx_len);
        tx_buf[tx_len++] = res_crc & 0xFF;
        tx_buf[tx_len++] = (res_crc >> 8) & 0xFF;
        HAL_UART_Transmit(huart, tx_buf, tx_len, 100);
    }
}

//void Modbus_RTU_Parse(UART_HandleTypeDef *huart, uint8_t *rx_data, uint16_t length) {
//    if (length < 8) return;
//
//    // Проверка CRC и Slave ID (1)
//    if (Modbus_CRC16(rx_data, length - 2) != ((rx_data[length-1] << 8) | rx_data[length-2])) return;
//    if (rx_data[0] != 1) return;
//
//    uint8_t f_code = rx_data[1];
//    uint16_t start_addr = (rx_data[2] << 8) | rx_data[3];
//    uint16_t reg_count = (rx_data[4] << 8) | rx_data[5];
//
//    uint8_t tx_buf[256];
//    uint16_t tx_len = 0;
//    uint16_t *base_ptr = NULL;
//
//    // Выбор базы данных
//    if (start_addr < 1000) {
//        base_ptr = (uint16_t *)&DBParameters;
//    } else if (start_addr >= 1000 && start_addr < 2000) {
//        base_ptr = (uint16_t *)&DBMain;
//        start_addr -= 1000;
//    }
//    else if (start_addr >= 2000) {
//        base_ptr = (uint16_t *)&DBSled;
//        start_addr -= 2000; // Регистрация с 2000 -> индекс 0 в DBSled
//    }
//
//    // --- Функция 03: Чтение ---
//    if (f_code == 0x03) {
//        tx_buf[0] = 1; tx_buf[1] = 0x03; tx_buf[2] = reg_count * 2;
//        for (int i = 0; i < reg_count; i++) {
//            uint16_t val = base_ptr[start_addr + i];
//            tx_buf[3 + i*2] = (val >> 8) & 0xFF;
//            tx_buf[4 + i*2] = val & 0xFF;
//        }
//        tx_len = 3 + (reg_count * 2);
//    }
//    // --- Функция 16 (0x10): Запись нескольких регистров ---
//    // Weintek шлет именно её для 32-битных данных
//    else if (f_code == 0x10) {
//        for (int i = 0; i < reg_count; i++) {
//            base_ptr[start_addr + i] = (rx_data[7 + i*2] << 8) | rx_data[8 + i*2];
//        }
//
//        // Если писали в параметры — обновляем CRC и сохраняем
//        if (base_ptr == (uint16_t *)&DBParameters) {
//            DB_UpdateCRC();
//            // DB_SaveToFlash(); // Опционально: сохранять сразу или по кнопке
//        }
//
//        // Обработка триггера для архива
//                if (base_ptr == (uint16_t *)&DBSled) {
//                    // Если записан Archive_Cmd (смещение 2 в словах от начала структуры)
//                    if (start_addr <= 2 && (start_addr + reg_count) > 2) {
//                        if (DBSled.Archive_Cmd == 1) {
//                            uint32_t addr = ADDR_ARCHIVE + (DBSled.Archive_Point_Idx * LOG_RECORD_SIZE);
//
//                            // Синхронное чтение из FRAM в окно Modbus
//                            FRAM_Read_Data_Polling(addr, (uint8_t*)&DBSled.Archive_Window, LOG_RECORD_SIZE);
//
//                            DBSled.Archive_Cmd = 0; // Сброс команды (подтверждение для ПК)
//                        }
//                    }
//                }
//
//        memcpy(tx_buf, rx_data, 6);
//        tx_len = 6;
//    }
//
//    // Отправка ответа
//    if (tx_len > 0) {
//        uint16_t res_crc = Modbus_CRC16(tx_buf, tx_len);
//        tx_buf[tx_len++] = res_crc & 0xFF;
//        tx_buf[tx_len++] = (res_crc >> 8) & 0xFF;
//        HAL_UART_Transmit(huart, tx_buf, tx_len, 100);
//    }
//}

// Колбек, який спрацьовує, коли пакет в UART закінчився
void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size) {
    if (huart->Instance == USART3) {
        Modbus_RTU_Parse(huart, RTU_Rx_Buf, Size);
        HAL_UARTEx_ReceiveToIdle_DMA(&huart3, RTU_Rx_Buf, sizeof(RTU_Rx_Buf)); // Перезапуск
    }
    else if (huart->Instance == USART6) {
                // Данные от ESP32-C3
                Modbus_RTU_Parse(huart, rx_buf_usart6, Size);
                HAL_UARTEx_ReceiveToIdle_DMA(&huart6, rx_buf_usart6, sizeof(rx_buf_usart6));
        }
}

void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart) {
	if (huart->Instance == USART3) {
	        // Очищаем флаги ошибок, если нужно (хотя HAL делает это внутри)
	        // Но самое важное — перезапустить прием

	        // Опционально: можно добавить лог для отладки
	        // printf("UART6 Error! Code: %ld\n", huart->ErrorCode);

	        // Перезапускаем прием
	        HAL_UARTEx_ReceiveToIdle_DMA(&huart3, RTU_Rx_Buf, sizeof(RTU_Rx_Buf)); // Перезапуск
	    }
	else if (huart->Instance == USART6) {
        // Очищаем флаги ошибок, если нужно (хотя HAL делает это внутри)
        // Но самое важное — перезапустить прием

        // Опционально: можно добавить лог для отладки
        // printf("UART6 Error! Code: %ld\n", huart->ErrorCode);

        // Перезапускаем прием
        HAL_UARTEx_ReceiveToIdle_DMA(&huart6, rx_buf_usart6, sizeof(rx_buf_usart6));
    }
}

void CAN1_Init_User(void){
	HAL_NVIC_SetPriority(CAN1_RX0_IRQn, 0, 0);
	HAL_NVIC_EnableIRQ(CAN1_RX0_IRQn);

	CAN_FilterTypeDef  sFilterConfig;

	// Настраиваем фильтр 0: принимаем абсолютно все пакеты
	sFilterConfig.FilterBank = 0;
	sFilterConfig.FilterMode = CAN_FILTERMODE_IDMASK;
	sFilterConfig.FilterScale = CAN_FILTERSCALE_32BIT;
	sFilterConfig.FilterIdHigh = 0x0000;
	sFilterConfig.FilterIdLow = 0x0000;
	sFilterConfig.FilterMaskIdHigh = 0x0000;
	sFilterConfig.FilterMaskIdLow = 0x0000;
	sFilterConfig.FilterFIFOAssignment = CAN_RX_FIFO0;
	sFilterConfig.FilterActivation = ENABLE;
	sFilterConfig.SlaveStartFilterBank = 14;

	if (HAL_CAN_ConfigFilter(&hcan1, &sFilterConfig) != HAL_OK) {
	    Error_Handler();
	}

	// Стартуем CAN
	if (HAL_CAN_Start(&hcan1) != HAL_OK) {
	    Error_Handler();
	}

	// Включаем прерывание прихода сообщения в FIFO 0
	if (HAL_CAN_ActivateNotification(&hcan1, CAN_IT_RX_FIFO0_MSG_PENDING) != HAL_OK) {
	    Error_Handler();
	}
}



void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan) {
    CAN_RxHeaderTypeDef rxHeader;
    uint8_t rxData[8];

    // Забираем сообщение из очереди
    if (HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &rxHeader, rxData) == HAL_OK) {

        // Пишем ID в Modbus регистр 4
        Modbus_Registers[4] = (uint16_t)rxHeader.StdId;

        // Пишем первые два байта данных в регистр 5 (склеиваем в 16 бит)
        Modbus_Registers[5] = (rxData[0] << 8) | rxData[1];

        // Визуальное подтверждение: мигаем синим светодиодом (LD2)
        LL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);
    }
}

// Функція обробки вхідних даних
err_t modbus_recv_callback(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err) {
    if (p != NULL) {
        uint8_t *data = (uint8_t *)p->payload;

        // --- ОБРОБКА FC03 (Read Holding Registers) ---
        if (data[7] == 0x03) {
            uint16_t start_addr = (data[8] << 8) | data[9];
            uint16_t reg_count = (data[10] << 8) | data[11];

            uint8_t response[64];
            memcpy(response, data, 7); // Копіюємо заголовок (ID, Protocol, Unit)
            response[7] = 0x03;
            response[8] = reg_count * 2;

            for (int i = 0; i < reg_count; i++) {
                if (start_addr + i < 10) {
                    response[9 + i*2] = (Modbus_Registers[start_addr + i] >> 8) & 0xFF;
                    response[10 + i*2] = Modbus_Registers[start_addr + i] & 0xFF;
                }
            }

            uint16_t len = 3 + (reg_count * 2);
            response[4] = (len >> 8) & 0xFF;
            response[5] = len & 0xFF;
            tcp_write(tpcb, response, 6 + len, TCP_WRITE_FLAG_COPY);
        }

        // --- ОБРОБКА FC06 (Write Single Register) ---
        else if (data[7] == 0x06) {
            uint16_t reg_addr = (data[8] << 8) | data[9];
            uint16_t reg_val = (data[10] << 8) | data[11];

            if (reg_addr < 10) {
                Modbus_Registers[reg_addr] = reg_val; // Оновлюємо значення в пам'яті

                // Керування синім світлодіодом через 3-й регістр
                if (reg_addr == 3) {
                    if (reg_val == 1) {
                        LL_GPIO_SetOutputPin(LD2_GPIO_Port, LD2_Pin);
                    } else {
                        LL_GPIO_ResetOutputPin(LD2_GPIO_Port, LD2_Pin);
                    }
                }

                // Скидаємо Watchdog (якщо реалізуєте його нижче)
                // comm_watchdog = 0;

                // Відповідь для FC06 — це віддзеркалення запиту (Echo)
                tcp_write(tpcb, data, p->tot_len, TCP_WRITE_FLAG_COPY);
            }
        }
        // --- ОБРОБКА FC 16 (0x10) - Write Multiple Registers ---
                else if (data[7] == 0x10) {
                    uint16_t start_addr = (data[8] << 8) | data[9];
                    uint16_t reg_count = (data[10] << 8) | data[11];
                    uint8_t byte_count = data[12];

                    if (start_addr < 10) {
                        // Копіюємо отримані дані в наш масив регістрів
                        for (int i = 0; i < reg_count; i++) {
                            if ((start_addr + i) < 10) {
                                Modbus_Registers[start_addr + i] = (data[13 + i*2] << 8) | data[14 + i*2];
                            }
                        }

                        // Логіка для синього світлодіода (якщо він потрапив у діапазон запису)
                        if (start_addr <= 3 && (start_addr + reg_count) > 3) {
                            if (Modbus_Registers[3] == 1) {
                                LL_GPIO_SetOutputPin(LD2_GPIO_Port, LD2_Pin);
                            } else {
                                LL_GPIO_ResetOutputPin(LD2_GPIO_Port, LD2_Pin);
                            }
                        }

                        // ВІДПОВІДЬ для FC 16: [Header(7)][FC(1)][Addr(2)][Count(2)]
                        // Вона коротша за запит (всього 12 байт)
                        uint8_t response[12];
                        memcpy(response, data, 7); // Копіюємо MBAP Header
                        response[4] = 0;           // Оновлюємо довжину (завжди 6 байт для відповіді FC16)
                        response[5] = 6;
                        response[7] = 0x10;        // Function Code
                        response[8] = data[8];     // Start Addr High
                        response[9] = data[9];     // Start Addr Low
                        response[10] = data[10];   // Quantity High
                        response[11] = data[11];   // Quantity Low

                        tcp_write(tpcb, response, 12, TCP_WRITE_FLAG_COPY);
                        tcp_output(tpcb); // Негайно відправляємо
                    }
                }

        tcp_recved(tpcb, p->tot_len);
        pbuf_free(p);
    } else if (err == ERR_OK) {
        return tcp_close(tpcb);
    }
    return ERR_OK;
}

// Коллбек для нового підключення
err_t modbus_accept_callback(void *arg, struct tcp_pcb *newpcb, err_t err) {
    tcp_recv(newpcb, modbus_recv_callback);
    return ERR_OK;
}

void UART3_SendString(char* str) {
    // Використовуємо стандартну функцію HAL для передачі
    // strlen(str) потребує #include <string.h> в Includes
    HAL_UART_Transmit(&huart3, (uint8_t*)str, strlen(str), 100);
}

// Запуск сервера
void ModbusTCP_Init(void) {
    struct tcp_pcb *pcb = tcp_new();
    tcp_bind(pcb, IP_ADDR_ANY, 502); // Слухаємо 502 порт
    pcb = tcp_listen(pcb);
    tcp_accept(pcb, modbus_accept_callback);
}


/* USER CODE END 4 */

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
