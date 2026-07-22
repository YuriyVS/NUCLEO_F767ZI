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
#include "DB_Constants.h"
#include "DI_Block.h"
#include "AI_Normalisation.h"
#include "Reg_System.h"
#include "DO_Block.h"
#include "AO_Normalisation.h"
#include "lwip/apps/httpd.h" // Обязательно для httpd_init()
#include "web_server.h"
#include "Block_Synhro.h"
#include "Block_Sifu.h"
#include "event_log.h"
#include "trace_buffer.h"
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
DMA_HandleTypeDef hdma_usart3_tx;
DMA_HandleTypeDef hdma_usart6_rx;
DMA_HandleTypeDef hdma_usart6_tx;

PCD_HandleTypeDef hpcd_USB_OTG_FS;

/* USER CODE BEGIN PV */
extern UART_HandleTypeDef huart3; // Кажемо компілятору: "huart3 оголошена десь в іншому місці"
extern UART_HandleTypeDef huart6; // Кажемо компілятору: "huart6 оголошена десь в іншому місці"
uint16_t Modbus_Registers[10] = {0}; // Наші Holding Registers
uint8_t RTU_Rx_Buf[256];             // Буфер для прийому по UART
uint8_t rx_buf_usart6[256];
//uint8_t usart3_tx_buf[256];
//uint8_t usart6_tx_buf[256];
uint8_t usart3_tx_buf[256] __attribute__((aligned(32)));
uint8_t usart6_tx_buf[256] __attribute__((aligned(32)));
volatile uint16_t Modbus6_Rx_Len;
volatile bool Modbus6_New_Packet_Flag;
uint32_t DI_Timer = 0; // Переменная для хранения времени последнего опроса входов
bool filterDI = 1;

#define ADC_CHANNELS_COUNT 6
volatile uint16_t aADCxConvertedData[ADC_CHANNELS_COUNT] __attribute__((section(".dtcm_data")));

volatile uint32_t ADC_Accumulator[ADC_CHANNELS_COUNT] = {0};
volatile uint32_t ADC_SamplesCount = 0;

bool test = 0;
float F_cpu;
float F_tim;
uint32_t last_tick_ai = 0, new_tick_ai= 0, delta_ai =0, min =400000, max = 0, samples_count_max=0;

/* Символы из Linker Script для ITCM (Код) */
extern uint32_t _sitcm_text; /* Начало во Flash (LMA) */
extern uint32_t _sitcm_ram;  /* Начало в RAM (VMA) */
extern uint32_t _eitcm_ram;  /* Конец в RAM */

/* Символы из Linker Script для DTCM (Данные) */
extern uint32_t _sdtcm_data; /* Начало во Flash (LMA) */
extern uint32_t _sdtcm_ram;  /* Начало в RAM (VMA) */
extern uint32_t _edtcm_ram;  /* Конец в RAM */

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
static void MX_ADC1_Init(void);
static void MX_TIM7_Init(void);
static void MX_DAC_Init(void);
static void MX_TIM6_Init(void);
static void MX_TIM4_Init(void);
static void MX_TIM8_Init(void);
static void MX_TIM3_Init(void);
/* USER CODE BEGIN PFP */
void MPU_Config(void);
uint16_t Modbus_CRC16(uint8_t *buffer, uint16_t length);
//void Modbus_RTU_Parse(uint8_t *rx_data, uint16_t length);
// Вспомогательная функция отправки ошибок Modbus RTU
void Modbus_SendException(UART_HandleTypeDef *huart, uint8_t *tx_buf, uint8_t f_code, uint8_t exception_code);
void Modbus_RTU_Parse(UART_HandleTypeDef *huart, uint8_t *rx_data, uint16_t length);
void CAN1_Init_User(void);

static void MX_DMA_ADC_Init(void);
static void MX_TIM2_Init(void);

void RAM_Sections_Init(void);
void Backup_SRAM_Init(void);


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
  RAM_Sections_Init();
  Backup_SRAM_Init();
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
  MX_ADC1_Init();
  MX_TIM7_Init();
  MX_DAC_Init();
  MX_TIM6_Init();
  MX_TIM4_Init();
  MX_TIM8_Init();
  MX_TIM3_Init();
  /* USER CODE BEGIN 2 */
  LL_TIM_ClearFlag_UPDATE(TIM6);     // Очищаем стартовый флаг
  LL_TIM_EnableIT_UPDATE(TIM6);      // Разрешаем прерывание по переполнению
  LL_TIM_EnableCounter(TIM6);        // Старт! TIM6 начал отсчет имитации 50Гц
  MX_DMA_ADC_Init();  // 1. Сначала тактируем и взводим DMA
//  MX_ADC1_Init();     // 2. Настраиваем АЦП (он связывается с DMA)
  MX_TIM2_Init();     // 3. Последним запускаем таймер-метроном

  Interrupt_PE012_Init();
  CAN1_Init_User();
  ModbusTCP_Init();
  // ЗАПУСК ВЕБ-СЕРВЕРА - проверьте наличие этой строки!
  httpd_init();

  // Запуск прийому по UART3 з використанням DMA та визначенням паузи (Idle Line)
  HAL_UARTEx_ReceiveToIdle_DMA(&huart3, RTU_Rx_Buf, 256);
  __HAL_DMA_DISABLE_IT(&hdma_usart3_rx, DMA_IT_HT); // Вимикаємо переривання Half Transfer
  // Запуск для ESP32-C3
    HAL_UARTEx_ReceiveToIdle_DMA(&huart6, rx_buf_usart6, sizeof(rx_buf_usart6));
    /* --- БЛОК СИНХРОНИЗАЦИИ --- */
    MX_SyncTimers_NVIC_Init(); // Настраиваем приоритеты NVIC
    MX_SyncTimers_Start();  // Включаем прерывания в регистрах таймеров

    // Инициализация оперативного журнала
    Log_Init();
    // Инициализация буфера следа
    Trace_Init();

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
  uint32_t last_tick_ao = 0;
  DBMain.b96.Save_to_Flash = 0;
  DBMain.b96.Read_from_Flash = 0;
  Init_FreqChannels();

  //float dt_ai = 3.3f; // Расчет каждые 3.3 мс
  LL_DAC_Enable(DAC1, LL_DAC_CHANNEL_1);
  LL_DAC_Enable(DAC1, LL_DAC_CHANNEL_2);
  DI_Timer = HAL_GetTick();
  new_tick_ai = DWT->CYCCNT;
  last_tick_ai = new_tick_ai;
  float K = (float)(TIM3->PSC + 1);

      // 2. Проверка чередования фаз
      // Берем текущий период Фазы А как эталон (360 градусов)
      period_cyccnt = 20000 * K;
      period_cyccnt_inv = 1.0f * 360.0f / period_cyccnt;
      one_degree_inv = 1.0f / 360.0f;
      ccr1_raw=0;
      ccr2_raw=0;
      ccr3_raw=0;
      ccr4_raw=0;
      ccr5_raw=0;
      ccr6_raw=0;
      current_ccr1=0;
      current_ccr2=0;
      current_ccr3=0;
      current_ccr4=0;
      current_ccr5=0;
      current_ccr6=0;
      Sifu_DisableAll();


  while (1)
  {
	    if(DBParameters.b96.bit2==1)
	    {
	    	Trace_Stop();
	    }
	    else if(DBParameters.b96.bit1==1)
	    {
	    	Trace_Start();
	    }

	     if(filterDI){
	    	  if (HAL_GetTick() - DI_Timer >= 1) {
	    	  	      DI_Timer +=1;//DI_Timer = HAL_GetTick();
	    	          Read_DI_Input_Filtered(); // Выполняется в основном потоке раз в 1 мс
	    	          DI_XOR();
	    	          //Update_Calculated_Frequency();
	    	  }
	  	  }
	  	  else{
	  		  Read_DI_Input();
	  		  DI_XOR();
	  		//Update_Calculated_Frequency();
	  	  }
//	      // 1. ГОЛОВНЕ: Обробка мережевого стека LwIP.
//	      // Без цієї функції Modbus TCP не буде відповідати!
	      MX_LWIP_Process();
//
	      // 2. (Опціонально) "Heartbeat" - миготіння світлодіодом LD1 (зелений),
	      // щоб візуально бачити, що програма не зависла.
	      static uint32_t last_heartbeat = 0;
	      if (HAL_GetTick() - last_heartbeat > 1000) {
	    	  Update_Calculated_Frequency();
	    	  Modbus_Registers[1]++; // Збільшуємо значення
	          LL_GPIO_TogglePin(LD1_GPIO_Port, LD1_Pin);
	          last_heartbeat = HAL_GetTick();
	          //UART3_SendString("System working...\r\n");
	          char msg[32];
	          //sprintf(msg, "Heartbeat Reg[1]: %d\r\n", Modbus_Registers[1]);
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
//
	      if (DBMain.b96.Save_to_Flash) {
	      	      DB_SaveToFlash();
	      	      DBMain.b96.Save_to_Flash = false;
	      }
	      if (DBMain.b96.Read_from_Flash) {
	    	  	  DB_ReadFromFlash();
	      	      DBMain.b96.Read_from_Flash = false;
	      }
//
	      Process_Alarm_Log();

		  //Аналогові входи. Например, расчет каждые 3.3 мс
//	      new_tick_ai = HAL_GetTick();
//	      if (new_tick_ai - last_tick_ai >= (uint32_t)(dt_ai))
//	      {
//	    	  last_tick_ai = new_tick_ai;
	      // Вместо HAL_GetTick() проверяем глобальный счетчик, который инкрементируется в прерывании DMA
//	      if (samples_count >= 44) // 44 выборки * 75 мкс = ровно 3.3 мс
//	      {
	    	  new_tick_ai = DWT->CYCCNT;
	       if (new_tick_ai - last_tick_ai >= 316800)
	       {
	    	   // проверяем чередование и формируем сигнал готовности сети
	    	   	      Sync_CheckSequence();
	    	   //last_tick_ai = new_tick_ai;
	    	  delta_ai = new_tick_ai - last_tick_ai;
	    	  if(delta_ai < min)min = delta_ai;
	    	  if(delta_ai > max)max = delta_ai;
	    	  if(delta_ai>2000000){
	    		  max=max;
	    	  }
	    	  last_tick_ai = new_tick_ai;
	    	  if(samples_count > samples_count_max)samples_count_max=samples_count;
	    	  // Отключаем прерывания на момент копирования, чтобы данные не изменились
	    	  if (DBMain.b96.StartSifu==0){
	    		  Process_AI_All();
	    		  Reg_System_Proces(DBConstants.f50.TaktSystReg);
	    	  }



	//	          if (samples > 0) {
	//	              // Среднее значение канала 0 (Напряжение сети)
	//	              float Useti_AI = (float)sum[0] / samples;
	//	              float Iakb_AI = (float)sum[1] / samples;
	//	              float Rezerv1_AI = (float)sum[2] / samples;
	//	              float Rezerv2_AI = (float)sum[3] / samples;
	//	              float Rezerv3_AI = (float)sum[4] / samples;
	//	              float Rezerv4_AI = (float)sum[5] / samples;
	//
	//	              // Теперь подставляем в формулу из вашего DOC-файла:
	//	              // UsetiV = P20.1 * Useti_AI + P20.2;
	//	              //float UsetiV = DBParameters.f50.P20_1 * Useti_AI + DBParameters.f50.P20_2;
	//	              //DBMain.f50.Useti = UsetiV * 100.0f / DBParameters.f50.P20_3;
	//	          }
		      }

		  DBMain.b32.i380V_Podano_DOxor = test;
		  DO_XOR();
		  Write_DO_Output();

		  //Аналогові виходи. Нормалізація сигналів
		  if (HAL_GetTick() - last_tick_ao >= (uint32_t)DBConstants.f50.TaktFilterAO) {
			      last_tick_ao = HAL_GetTick();

		          // Частота генератора
		          Process_AO(&AO_GenFreq, DBMain.f50.GenFreq, DBParameters.f100.P80_2, DBParameters.f100.P80_3, DBParameters.f100.P80_5);
		          DBMain.f50.GenFreqHz = DBMain.f50.GenFreq * DBParameters.f100.P80_4 * 0.01f; //Hz

		          // Ток АКБ
		          Process_AO(&AO_Iakb, DBMain.f50.Iakb, DBParameters.f100.P81_2, DBParameters.f100.P81_3, DBParameters.f100.P81_5);
		          DBMain.f50.IakbA = DBMain.f50.Iakb * DBParameters.f100.P81_4 * 0.01f; //A
		  }

		  if (Modbus6_New_Packet_Flag) {
		      Modbus_RTU_Parse(&huart6, rx_buf_usart6, Modbus6_Rx_Len);
		      Modbus6_New_Packet_Flag = 0;
		      // Перезапускаем прием
		      HAL_UARTEx_ReceiveToIdle_DMA(&huart6, rx_buf_usart6, sizeof(rx_buf_usart6));
		  }
//
//
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
//	      c = a / b; // Проверьте в Disassembly, должна быть инструкция VMUL.F32
//	      c_d = a_d / b_d; // Если здесь будет инструкция VMUL.F64 — значит DP работает.
//	      start_f = DWT->CYCCNT;
//	      CalculateThyristorPhasef(); //c = sinf(a);//c = a / b; // Проверьте в Disassembly, должна быть инструкция VMUL.F32
//	      end_f = DWT->CYCCNT;
//	      delta_f = end_f - start_f;
//
//	      start_d = DWT->CYCCNT;
//	      c_d = a_d / b_d; //CalculateThyristorPhased(); //c_d = sin(a_d);//c_d = a_d / b_d; // Если здесь будет инструкция VMUL.F64 — значит DP работает.
//	                       // Если будет BL __aeabi_dmul — это медленная эмуляция.
//	      end_d = DWT->CYCCNT;
//	      delta_d = end_d - start_d;
//	      delta_d = delta_d;
//
//	      //DWT->EXCCNT = 0; // Сброс
//	      //start_ex = DWT->EXCCNT;
//
//	      //HAL_Delay(1); // Ждем 10 мс. За это время произойдет ровно 10 прерываний SysTick
//
//	      //end_ex = DWT->EXCCNT;
//	      if(DWT->EXCCNT > end_ex)
//	      {
//	    	  end_ex = DWT->EXCCNT;
//	      }
//	      if(DWT->EXCCNT > 0)
//	      {
//	    	  if(DWT->EXCCNT < start_ex){start_ex = DWT->EXCCNT;}
//	    	  total_overhead += DWT->EXCCNT;
//	    	  count += 1;
//	    	  average = total_overhead / count;
//	    	  DWT->EXCCNT = 0;
//	    	  Modbus_Registers[0] = average;
//	    	  Modbus_Registers[2] = end_ex;
//	    	  //Modbus_Registers[3] = start_ex;
//
//	      }
//	      F_cpu = (float)HAL_RCC_GetHCLKFreq();
//	      F_tim = (float)(HAL_RCC_GetPCLK1Freq() * 2); // Обычно частота таймеров в 2 раза выше PCLK

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
  * @brief ADC1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC1_Init(void)
{

  /* USER CODE BEGIN ADC1_Init 0 */

  /* USER CODE END ADC1_Init 0 */

  LL_ADC_InitTypeDef ADC_InitStruct = {0};
  LL_ADC_REG_InitTypeDef ADC_REG_InitStruct = {0};
  LL_ADC_CommonInitTypeDef ADC_CommonInitStruct = {0};

  LL_GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* Peripheral clock enable */
  LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_ADC1);

  LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOC);
  LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOA);
  /**ADC1 GPIO Configuration
  PC0   ------> ADC1_IN10
  PA0/WKUP   ------> ADC1_IN0
  PA3   ------> ADC1_IN3
  */
  GPIO_InitStruct.Pin = AI5_Pin;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  LL_GPIO_Init(AI5_GPIO_Port, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = AI1_Pin;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  LL_GPIO_Init(AI1_GPIO_Port, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = AI4_Pin;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  LL_GPIO_Init(AI4_GPIO_Port, &GPIO_InitStruct);

  /* USER CODE BEGIN ADC1_Init 1 */

  /* USER CODE END ADC1_Init 1 */

  /** Common config
  */
  ADC_InitStruct.Resolution = LL_ADC_RESOLUTION_12B;
  ADC_InitStruct.DataAlignment = LL_ADC_DATA_ALIGN_RIGHT;
  ADC_InitStruct.SequencersScanMode = LL_ADC_SEQ_SCAN_DISABLE;
  LL_ADC_Init(ADC1, &ADC_InitStruct);
  ADC_REG_InitStruct.TriggerSource = LL_ADC_REG_TRIG_SOFTWARE;
  ADC_REG_InitStruct.SequencerLength = LL_ADC_REG_SEQ_SCAN_DISABLE;
  ADC_REG_InitStruct.SequencerDiscont = LL_ADC_REG_SEQ_DISCONT_DISABLE;
  ADC_REG_InitStruct.ContinuousMode = LL_ADC_REG_CONV_SINGLE;
  ADC_REG_InitStruct.DMATransfer = LL_ADC_REG_DMA_TRANSFER_NONE;
  LL_ADC_REG_Init(ADC1, &ADC_REG_InitStruct);
  LL_ADC_REG_SetFlagEndOfConversion(ADC1, LL_ADC_REG_FLAG_EOC_UNITARY_CONV);
  ADC_CommonInitStruct.CommonClock = LL_ADC_CLOCK_SYNC_PCLK_DIV4;
  ADC_CommonInitStruct.Multimode = LL_ADC_MULTI_INDEPENDENT;
  LL_ADC_CommonInit(__LL_ADC_COMMON_INSTANCE(ADC1), &ADC_CommonInitStruct);

  /** Configure Regular Channel
  */
  LL_ADC_REG_SetSequencerRanks(ADC1, LL_ADC_REG_RANK_1, LL_ADC_CHANNEL_0);
  LL_ADC_SetChannelSamplingTime(ADC1, LL_ADC_CHANNEL_0, LL_ADC_SAMPLINGTIME_3CYCLES);
  /* USER CODE BEGIN ADC1_Init 2 */
    // 1. Перенастраиваем регулярную группу (Триггер TIM2 + DMA + 6 канала)
    ADC_REG_InitStruct.TriggerSource    = LL_ADC_REG_TRIG_EXT_TIM2_TRGO;
    ADC_REG_InitStruct.SequencerLength  = LL_ADC_REG_SEQ_SCAN_ENABLE_6RANKS;
    ADC_REG_InitStruct.ContinuousMode   = LL_ADC_REG_CONV_SINGLE;
    ADC_REG_InitStruct.DMATransfer      = LL_ADC_REG_DMA_TRANSFER_UNLIMITED;
    LL_ADC_REG_Init(ADC1, &ADC_REG_InitStruct);

    // 2. Включаем скан-режим (CubeIDE мог его выключить выше)
    LL_ADC_SetSequencersScanMode(ADC1, LL_ADC_SEQ_SCAN_ENABLE);

    // 3. Устанавливаем фронт триггера (RISING EDGE) через регистр CR2
    // Это заменяет неисправную функцию LL_ADC_REG_SetTriggerEdge
    MODIFY_REG(ADC1->CR2, ADC_CR2_EXTEN, LL_ADC_REG_TRIG_EXT_RISING);

    // 4. Настраиваем последовательность каналов AI1-AI6
    LL_ADC_REG_SetSequencerRanks(ADC1, LL_ADC_REG_RANK_1, LL_ADC_CHANNEL_0);
    LL_ADC_REG_SetSequencerRanks(ADC1, LL_ADC_REG_RANK_2, LL_ADC_CHANNEL_0);
    LL_ADC_REG_SetSequencerRanks(ADC1, LL_ADC_REG_RANK_3, LL_ADC_CHANNEL_0);
    LL_ADC_REG_SetSequencerRanks(ADC1, LL_ADC_REG_RANK_4, LL_ADC_CHANNEL_3);
    LL_ADC_REG_SetSequencerRanks(ADC1, LL_ADC_REG_RANK_5, LL_ADC_CHANNEL_10);
    LL_ADC_REG_SetSequencerRanks(ADC1, LL_ADC_REG_RANK_6, LL_ADC_CHANNEL_10);

    LL_ADC_REG_SetSequencerRanks(ADC1, LL_ADC_REG_RANK_1, LL_ADC_CHANNEL_0);
//    LL_ADC_REG_SetSequencerRanks(ADC1, LL_ADC_REG_RANK_2, LL_ADC_CHANNEL_1);
//    LL_ADC_REG_SetSequencerRanks(ADC1, LL_ADC_REG_RANK_3, LL_ADC_CHANNEL_2);
    LL_ADC_REG_SetSequencerRanks(ADC1, LL_ADC_REG_RANK_2, LL_ADC_CHANNEL_0);
    LL_ADC_REG_SetSequencerRanks(ADC1, LL_ADC_REG_RANK_3, LL_ADC_CHANNEL_0);
    LL_ADC_REG_SetSequencerRanks(ADC1, LL_ADC_REG_RANK_4, LL_ADC_CHANNEL_3);
    LL_ADC_REG_SetSequencerRanks(ADC1, LL_ADC_REG_RANK_5, LL_ADC_CHANNEL_10);
//    LL_ADC_REG_SetSequencerRanks(ADC1, LL_ADC_REG_RANK_6, LL_ADC_CHANNEL_11);
    LL_ADC_REG_SetSequencerRanks(ADC1, LL_ADC_REG_RANK_6, LL_ADC_CHANNEL_10);

    // 5. Устанавливаем время выборки (15 циклов для всех каналов)
    uint32_t smp = LL_ADC_SAMPLINGTIME_15CYCLES;

    LL_ADC_SetChannelSamplingTime(ADC1, LL_ADC_CHANNEL_0, smp);
//    LL_ADC_SetChannelSamplingTime(ADC1, LL_ADC_CHANNEL_1, smp);
//    LL_ADC_SetChannelSamplingTime(ADC1, LL_ADC_CHANNEL_2, smp);
    LL_ADC_SetChannelSamplingTime(ADC1, LL_ADC_CHANNEL_3, smp);
    LL_ADC_SetChannelSamplingTime(ADC1, LL_ADC_CHANNEL_10, smp);
//    LL_ADC_SetChannelSamplingTime(ADC1, LL_ADC_CHANNEL_11, smp);

    // 6. Устанавливаем флаг окончания всей последовательности (для DMA)
    LL_ADC_REG_SetFlagEndOfConversion(ADC1, LL_ADC_REG_FLAG_EOC_SEQUENCE_CONV);

    // 7. Включаем АЦП
    LL_ADC_Enable(ADC1);

  /* USER CODE END ADC1_Init 2 */

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
  * @brief DAC Initialization Function
  * @param None
  * @retval None
  */
static void MX_DAC_Init(void)
{

  /* USER CODE BEGIN DAC_Init 0 */

  /* USER CODE END DAC_Init 0 */

  LL_DAC_InitTypeDef DAC_InitStruct = {0};

  LL_GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* Peripheral clock enable */
  LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_DAC1);

  LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOA);
  /**DAC GPIO Configuration
  PA4   ------> DAC_OUT1
  PA5   ------> DAC_OUT2
  */
  GPIO_InitStruct.Pin = AO1_Pin;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  LL_GPIO_Init(AO1_GPIO_Port, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = AO2_Pin;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  LL_GPIO_Init(AO2_GPIO_Port, &GPIO_InitStruct);

  /* DAC interrupt Init */
  NVIC_SetPriority(TIM6_DAC_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),0, 0));
  NVIC_EnableIRQ(TIM6_DAC_IRQn);

  /* USER CODE BEGIN DAC_Init 1 */

  /* USER CODE END DAC_Init 1 */

  /** DAC channel OUT1 config
  */
  DAC_InitStruct.TriggerSource = LL_DAC_TRIG_SOFTWARE;
  DAC_InitStruct.WaveAutoGeneration = LL_DAC_WAVE_AUTO_GENERATION_NONE;
  DAC_InitStruct.OutputBuffer = LL_DAC_OUTPUT_BUFFER_ENABLE;
  LL_DAC_Init(DAC, LL_DAC_CHANNEL_1, &DAC_InitStruct);

  /** DAC channel OUT2 config
  */
  LL_DAC_Init(DAC, LL_DAC_CHANNEL_2, &DAC_InitStruct);
  /* USER CODE BEGIN DAC_Init 2 */

  /* USER CODE END DAC_Init 2 */

}

/**
  * @brief TIM3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM3_Init(void)
{

  /* USER CODE BEGIN TIM3_Init 0 */

  /* USER CODE END TIM3_Init 0 */

  LL_TIM_InitTypeDef TIM_InitStruct = {0};
  LL_TIM_OC_InitTypeDef TIM_OC_InitStruct = {0};

  LL_GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* Peripheral clock enable */
  LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_TIM3);

  LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOA);
  /**TIM3 GPIO Configuration
  PA6   ------> TIM3_CH1
  */
  GPIO_InitStruct.Pin = LL_GPIO_PIN_6;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_HIGH;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  GPIO_InitStruct.Alternate = LL_GPIO_AF_2;
  LL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /* USER CODE BEGIN TIM3_Init 1 */

  /* USER CODE END TIM3_Init 1 */
  TIM_InitStruct.Prescaler = 95;
  TIM_InitStruct.CounterMode = LL_TIM_COUNTERMODE_UP;
  TIM_InitStruct.Autoreload = 65535;
  TIM_InitStruct.ClockDivision = LL_TIM_CLOCKDIVISION_DIV1;
  LL_TIM_Init(TIM3, &TIM_InitStruct);
  LL_TIM_DisableARRPreload(TIM3);
  LL_TIM_SetClockSource(TIM3, LL_TIM_CLOCKSOURCE_INTERNAL);
  TIM_OC_InitStruct.OCMode = LL_TIM_OCMODE_FROZEN;
  TIM_OC_InitStruct.OCState = LL_TIM_OCSTATE_DISABLE;
  TIM_OC_InitStruct.OCNState = LL_TIM_OCSTATE_DISABLE;
  TIM_OC_InitStruct.CompareValue = 0;
  TIM_OC_InitStruct.OCPolarity = LL_TIM_OCPOLARITY_HIGH;
  LL_TIM_OC_Init(TIM3, LL_TIM_CHANNEL_CH2, &TIM_OC_InitStruct);
  LL_TIM_OC_DisableFast(TIM3, LL_TIM_CHANNEL_CH2);
  LL_TIM_OC_Init(TIM3, LL_TIM_CHANNEL_CH3, &TIM_OC_InitStruct);
  LL_TIM_OC_DisableFast(TIM3, LL_TIM_CHANNEL_CH3);
  LL_TIM_SetTriggerOutput(TIM3, LL_TIM_TRGO_RESET);
  LL_TIM_DisableMasterSlaveMode(TIM3);
  LL_TIM_IC_SetActiveInput(TIM3, LL_TIM_CHANNEL_CH1, LL_TIM_ACTIVEINPUT_DIRECTTI);
  LL_TIM_IC_SetPrescaler(TIM3, LL_TIM_CHANNEL_CH1, LL_TIM_ICPSC_DIV1);
  LL_TIM_IC_SetFilter(TIM3, LL_TIM_CHANNEL_CH1, LL_TIM_IC_FILTER_FDIV8_N6);
  LL_TIM_IC_SetPolarity(TIM3, LL_TIM_CHANNEL_CH1, LL_TIM_IC_POLARITY_RISING);
  /* USER CODE BEGIN TIM3_Init 2 */

  /* USER CODE END TIM3_Init 2 */

}

/**
  * @brief TIM4 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM4_Init(void)
{

  /* USER CODE BEGIN TIM4_Init 0 */

  /* USER CODE END TIM4_Init 0 */

  LL_TIM_InitTypeDef TIM_InitStruct = {0};
  LL_TIM_OC_InitTypeDef TIM_OC_InitStruct = {0};

  LL_GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* Peripheral clock enable */
  LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_TIM4);

  LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOD);
  /**TIM4 GPIO Configuration
  PD14   ------> TIM4_CH3
  */
  GPIO_InitStruct.Pin = LL_GPIO_PIN_14;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  GPIO_InitStruct.Alternate = LL_GPIO_AF_2;
  LL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  /* USER CODE BEGIN TIM4_Init 1 */

  /* USER CODE END TIM4_Init 1 */
  TIM_InitStruct.Prescaler = 95;
  TIM_InitStruct.CounterMode = LL_TIM_COUNTERMODE_UP;
  TIM_InitStruct.Autoreload = 65535;
  TIM_InitStruct.ClockDivision = LL_TIM_CLOCKDIVISION_DIV1;
  LL_TIM_Init(TIM4, &TIM_InitStruct);
  LL_TIM_DisableARRPreload(TIM4);
  LL_TIM_SetClockSource(TIM4, LL_TIM_CLOCKSOURCE_INTERNAL);
  TIM_OC_InitStruct.OCMode = LL_TIM_OCMODE_FROZEN;
  TIM_OC_InitStruct.OCState = LL_TIM_OCSTATE_DISABLE;
  TIM_OC_InitStruct.OCNState = LL_TIM_OCSTATE_DISABLE;
  TIM_OC_InitStruct.CompareValue = 0;
  TIM_OC_InitStruct.OCPolarity = LL_TIM_OCPOLARITY_HIGH;
  LL_TIM_OC_Init(TIM4, LL_TIM_CHANNEL_CH2, &TIM_OC_InitStruct);
  LL_TIM_OC_DisableFast(TIM4, LL_TIM_CHANNEL_CH2);
  LL_TIM_OC_Init(TIM4, LL_TIM_CHANNEL_CH4, &TIM_OC_InitStruct);
  LL_TIM_OC_DisableFast(TIM4, LL_TIM_CHANNEL_CH4);
  LL_TIM_SetTriggerOutput(TIM4, LL_TIM_TRGO_RESET);
  LL_TIM_DisableMasterSlaveMode(TIM4);
  LL_TIM_IC_SetActiveInput(TIM4, LL_TIM_CHANNEL_CH3, LL_TIM_ACTIVEINPUT_DIRECTTI);
  LL_TIM_IC_SetPrescaler(TIM4, LL_TIM_CHANNEL_CH3, LL_TIM_ICPSC_DIV1);
  LL_TIM_IC_SetFilter(TIM4, LL_TIM_CHANNEL_CH3, LL_TIM_IC_FILTER_FDIV8_N6);
  LL_TIM_IC_SetPolarity(TIM4, LL_TIM_CHANNEL_CH3, LL_TIM_IC_POLARITY_RISING);
  /* USER CODE BEGIN TIM4_Init 2 */

  /* USER CODE END TIM4_Init 2 */

}

/**
  * @brief TIM6 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM6_Init(void)
{

  /* USER CODE BEGIN TIM6_Init 0 */

  /* USER CODE END TIM6_Init 0 */

  LL_TIM_InitTypeDef TIM_InitStruct = {0};

  /* Peripheral clock enable */
  LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_TIM6);

  /* TIM6 interrupt Init */
  NVIC_SetPriority(TIM6_DAC_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),0, 0));
  NVIC_EnableIRQ(TIM6_DAC_IRQn);

  /* USER CODE BEGIN TIM6_Init 1 */

  /* USER CODE END TIM6_Init 1 */
  TIM_InitStruct.Prescaler = 319;
  TIM_InitStruct.CounterMode = LL_TIM_COUNTERMODE_UP;
  TIM_InitStruct.Autoreload = 999;
  LL_TIM_Init(TIM6, &TIM_InitStruct);
  LL_TIM_DisableARRPreload(TIM6);
  LL_TIM_SetTriggerOutput(TIM6, LL_TIM_TRGO_RESET);
  LL_TIM_DisableMasterSlaveMode(TIM6);
  /* USER CODE BEGIN TIM6_Init 2 */

  /* USER CODE END TIM6_Init 2 */

}

/**
  * @brief TIM7 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM7_Init(void)
{

  /* USER CODE BEGIN TIM7_Init 0 */

  /* USER CODE END TIM7_Init 0 */

  LL_TIM_InitTypeDef TIM_InitStruct = {0};

  /* Peripheral clock enable */
  LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_TIM7);

  /* USER CODE BEGIN TIM7_Init 1 */

  /* USER CODE END TIM7_Init 1 */
  TIM_InitStruct.Prescaler = 95;
  TIM_InitStruct.CounterMode = LL_TIM_COUNTERMODE_UP;
  TIM_InitStruct.Autoreload = 65535;
  LL_TIM_Init(TIM7, &TIM_InitStruct);
  LL_TIM_DisableARRPreload(TIM7);
  LL_TIM_SetTriggerOutput(TIM7, LL_TIM_TRGO_RESET);
  LL_TIM_DisableMasterSlaveMode(TIM7);
  /* USER CODE BEGIN TIM7_Init 2 */
  /* 1. Включаем тактирование шины APB1 */
    LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_TIM7);

    /* 2. Настройка частоты: 1 тик = 1 мксек при тактировании 108 МГц */
    TIM_InitStruct.Prescaler = 107;
    TIM_InitStruct.CounterMode = LL_TIM_COUNTERMODE_UP;

    /* 3. Период: 500 тиков = 550 мксек */
    TIM_InitStruct.Autoreload = 549;

    LL_TIM_Init(TIM7, &TIM_InitStruct);

    /* 4. Настройки для СИФУ */
    LL_TIM_DisableARRPreload(TIM7);

    // Режим одного импульса (таймер сам остановится после 500 мксек)
    LL_TIM_SetOnePulseMode(TIM7, LL_TIM_ONEPULSEMODE_SINGLE);

    // Разрешаем прерывание по окончании счета
    LL_TIM_EnableIT_UPDATE(TIM7);

    /* 5. Настройка приоритета в NVIC (должен быть ниже, чем у TIM3,4,8) */
    NVIC_SetPriority(TIM7_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 5, 0));
    NVIC_EnableIRQ(TIM7_IRQn);

  /* USER CODE END TIM7_Init 2 */

}

/**
  * @brief TIM8 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM8_Init(void)
{

  /* USER CODE BEGIN TIM8_Init 0 */

  /* USER CODE END TIM8_Init 0 */

  LL_TIM_InitTypeDef TIM_InitStruct = {0};
  LL_TIM_OC_InitTypeDef TIM_OC_InitStruct = {0};
  LL_TIM_BDTR_InitTypeDef TIM_BDTRInitStruct = {0};

  LL_GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* Peripheral clock enable */
  LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_TIM8);

  LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOC);
  /**TIM8 GPIO Configuration
  PC9   ------> TIM8_CH4
  */
  GPIO_InitStruct.Pin = LL_GPIO_PIN_9;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  GPIO_InitStruct.Alternate = LL_GPIO_AF_3;
  LL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /* USER CODE BEGIN TIM8_Init 1 */

  /* USER CODE END TIM8_Init 1 */
  TIM_InitStruct.Prescaler = 95;
  TIM_InitStruct.CounterMode = LL_TIM_COUNTERMODE_UP;
  TIM_InitStruct.Autoreload = 65535;
  TIM_InitStruct.ClockDivision = LL_TIM_CLOCKDIVISION_DIV1;
  TIM_InitStruct.RepetitionCounter = 0;
  LL_TIM_Init(TIM8, &TIM_InitStruct);
  LL_TIM_DisableARRPreload(TIM8);
  LL_TIM_SetClockSource(TIM8, LL_TIM_CLOCKSOURCE_INTERNAL);
  TIM_OC_InitStruct.OCMode = LL_TIM_OCMODE_FROZEN;
  TIM_OC_InitStruct.OCState = LL_TIM_OCSTATE_DISABLE;
  TIM_OC_InitStruct.OCNState = LL_TIM_OCSTATE_DISABLE;
  TIM_OC_InitStruct.CompareValue = 0;
  TIM_OC_InitStruct.OCPolarity = LL_TIM_OCPOLARITY_HIGH;
  TIM_OC_InitStruct.OCNPolarity = LL_TIM_OCPOLARITY_HIGH;
  TIM_OC_InitStruct.OCIdleState = LL_TIM_OCIDLESTATE_LOW;
  TIM_OC_InitStruct.OCNIdleState = LL_TIM_OCIDLESTATE_LOW;
  LL_TIM_OC_Init(TIM8, LL_TIM_CHANNEL_CH1, &TIM_OC_InitStruct);
  LL_TIM_OC_DisableFast(TIM8, LL_TIM_CHANNEL_CH1);
  LL_TIM_OC_Init(TIM8, LL_TIM_CHANNEL_CH2, &TIM_OC_InitStruct);
  LL_TIM_OC_DisableFast(TIM8, LL_TIM_CHANNEL_CH2);
  LL_TIM_SetTriggerOutput(TIM8, LL_TIM_TRGO_RESET);
  LL_TIM_SetTriggerOutput2(TIM8, LL_TIM_TRGO2_RESET);
  LL_TIM_DisableMasterSlaveMode(TIM8);
  LL_TIM_IC_SetActiveInput(TIM8, LL_TIM_CHANNEL_CH4, LL_TIM_ACTIVEINPUT_DIRECTTI);
  LL_TIM_IC_SetPrescaler(TIM8, LL_TIM_CHANNEL_CH4, LL_TIM_ICPSC_DIV1);
  LL_TIM_IC_SetFilter(TIM8, LL_TIM_CHANNEL_CH4, LL_TIM_IC_FILTER_FDIV8_N6);
  LL_TIM_IC_SetPolarity(TIM8, LL_TIM_CHANNEL_CH4, LL_TIM_IC_POLARITY_RISING);
  TIM_BDTRInitStruct.OSSRState = LL_TIM_OSSR_DISABLE;
  TIM_BDTRInitStruct.OSSIState = LL_TIM_OSSI_DISABLE;
  TIM_BDTRInitStruct.LockLevel = LL_TIM_LOCKLEVEL_OFF;
  TIM_BDTRInitStruct.DeadTime = 0;
  TIM_BDTRInitStruct.BreakState = LL_TIM_BREAK_DISABLE;
  TIM_BDTRInitStruct.BreakPolarity = LL_TIM_BREAK_POLARITY_HIGH;
  TIM_BDTRInitStruct.BreakFilter = LL_TIM_BREAK_FILTER_FDIV1;
  TIM_BDTRInitStruct.Break2State = LL_TIM_BREAK2_DISABLE;
  TIM_BDTRInitStruct.Break2Polarity = LL_TIM_BREAK2_POLARITY_HIGH;
  TIM_BDTRInitStruct.Break2Filter = LL_TIM_BREAK2_FILTER_FDIV1;
  TIM_BDTRInitStruct.AutomaticOutput = LL_TIM_AUTOMATICOUTPUT_DISABLE;
  LL_TIM_BDTR_Init(TIM8, &TIM_BDTRInitStruct);
  /* USER CODE BEGIN TIM8_Init 2 */

  /* USER CODE END TIM8_Init 2 */

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
  HAL_NVIC_SetPriority(DMA1_Stream1_IRQn, 4, 0);
  HAL_NVIC_EnableIRQ(DMA1_Stream1_IRQn);
  /* DMA1_Stream3_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Stream3_IRQn, 4, 0);
  HAL_NVIC_EnableIRQ(DMA1_Stream3_IRQn);
  /* DMA2_Stream1_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA2_Stream1_IRQn, 4, 0);
  HAL_NVIC_EnableIRQ(DMA2_Stream1_IRQn);
  /* DMA2_Stream6_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA2_Stream6_IRQn, 4, 0);
  HAL_NVIC_EnableIRQ(DMA2_Stream6_IRQn);

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
  LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOE);
  LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOC);
  LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOF);
  LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOH);
  LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOA);
  LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOB);
  LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOG);
  LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOD);

  /**/
  LL_GPIO_ResetOutputPin(GPIOE, LL_GPIO_PIN_4);

  /**/
  LL_GPIO_ResetOutputPin(GPIOE, LL_GPIO_PIN_5);

  /**/
  LL_GPIO_ResetOutputPin(GPIOE, LL_GPIO_PIN_6);

  /**/
  LL_GPIO_ResetOutputPin(LD1_GPIO_Port, LD1_Pin);

  /**/
  LL_GPIO_ResetOutputPin(DO1_GPIO_Port, DO1_Pin);

  /**/
  LL_GPIO_ResetOutputPin(DO2_GPIO_Port, DO2_Pin);

  /**/
  LL_GPIO_ResetOutputPin(LD3_GPIO_Port, LD3_Pin);

  /**/
  LL_GPIO_ResetOutputPin(YI3_GPIO_Port, YI3_Pin);

  /**/
  LL_GPIO_ResetOutputPin(YI6_GPIO_Port, YI6_Pin);

  /**/
  LL_GPIO_ResetOutputPin(DO3_GPIO_Port, DO3_Pin);

  /**/
  LL_GPIO_ResetOutputPin(DO4_GPIO_Port, DO4_Pin);

  /**/
  LL_GPIO_ResetOutputPin(DO5_GPIO_Port, DO5_Pin);

  /**/
  LL_GPIO_ResetOutputPin(DO6_GPIO_Port, DO6_Pin);

  /**/
  LL_GPIO_ResetOutputPin(USB_PowerSwitchOn_GPIO_Port, USB_PowerSwitchOn_Pin);

  /**/
  LL_GPIO_ResetOutputPin(DO9_GPIO_Port, DO9_Pin);

  /**/
  LL_GPIO_ResetOutputPin(YI5_GPIO_Port, YI5_Pin);

  /**/
  LL_GPIO_ResetOutputPin(YI2_GPIO_Port, YI2_Pin);

  /**/
  LL_GPIO_ResetOutputPin(DO11_GPIO_Port, DO11_Pin);

  /**/
  LL_GPIO_ResetOutputPin(DO13_GPIO_Port, DO13_Pin);

  /**/
  LL_GPIO_ResetOutputPin(YI1_GPIO_Port, YI1_Pin);

  /**/
  LL_GPIO_ResetOutputPin(YI4_GPIO_Port, YI4_Pin);

  /**/
  LL_GPIO_ResetOutputPin(LD2_GPIO_Port, LD2_Pin);

  /**/
  LL_SYSCFG_SetEXTISource(LL_SYSCFG_EXTI_PORTE, LL_SYSCFG_EXTI_LINE2);

  /**/
  LL_SYSCFG_SetEXTISource(LL_SYSCFG_EXTI_PORTC, LL_SYSCFG_EXTI_LINE13);

  /**/
  LL_SYSCFG_SetEXTISource(LL_SYSCFG_EXTI_PORTE, LL_SYSCFG_EXTI_LINE0);

  /**/
  LL_SYSCFG_SetEXTISource(LL_SYSCFG_EXTI_PORTE, LL_SYSCFG_EXTI_LINE1);

  /**/
  EXTI_InitStruct.Line_0_31 = LL_EXTI_LINE_2;
  EXTI_InitStruct.LineCommand = ENABLE;
  EXTI_InitStruct.Mode = LL_EXTI_MODE_IT;
  EXTI_InitStruct.Trigger = LL_EXTI_TRIGGER_RISING;
  LL_EXTI_Init(&EXTI_InitStruct);

  /**/
  EXTI_InitStruct.Line_0_31 = LL_EXTI_LINE_13;
  EXTI_InitStruct.LineCommand = ENABLE;
  EXTI_InitStruct.Mode = LL_EXTI_MODE_IT;
  EXTI_InitStruct.Trigger = LL_EXTI_TRIGGER_RISING;
  LL_EXTI_Init(&EXTI_InitStruct);

  /**/
  EXTI_InitStruct.Line_0_31 = LL_EXTI_LINE_0;
  EXTI_InitStruct.LineCommand = ENABLE;
  EXTI_InitStruct.Mode = LL_EXTI_MODE_IT;
  EXTI_InitStruct.Trigger = LL_EXTI_TRIGGER_RISING;
  LL_EXTI_Init(&EXTI_InitStruct);

  /**/
  EXTI_InitStruct.Line_0_31 = LL_EXTI_LINE_1;
  EXTI_InitStruct.LineCommand = ENABLE;
  EXTI_InitStruct.Mode = LL_EXTI_MODE_IT;
  EXTI_InitStruct.Trigger = LL_EXTI_TRIGGER_RISING;
  LL_EXTI_Init(&EXTI_InitStruct);

  /**/
  LL_GPIO_SetPinPull(DI_CountImpuls3_GPIO_Port, DI_CountImpuls3_Pin, LL_GPIO_PULL_NO);

  /**/
  LL_GPIO_SetPinPull(USER_Btn_GPIO_Port, USER_Btn_Pin, LL_GPIO_PULL_NO);

  /**/
  LL_GPIO_SetPinPull(DI_CountImpuls1_GPIO_Port, DI_CountImpuls1_Pin, LL_GPIO_PULL_NO);

  /**/
  LL_GPIO_SetPinPull(DI_CountImpuls2_GPIO_Port, DI_CountImpuls2_Pin, LL_GPIO_PULL_NO);

  /**/
  LL_GPIO_SetPinMode(DI_CountImpuls3_GPIO_Port, DI_CountImpuls3_Pin, LL_GPIO_MODE_INPUT);

  /**/
  LL_GPIO_SetPinMode(USER_Btn_GPIO_Port, USER_Btn_Pin, LL_GPIO_MODE_INPUT);

  /**/
  LL_GPIO_SetPinMode(DI_CountImpuls1_GPIO_Port, DI_CountImpuls1_Pin, LL_GPIO_MODE_INPUT);

  /**/
  LL_GPIO_SetPinMode(DI_CountImpuls2_GPIO_Port, DI_CountImpuls2_Pin, LL_GPIO_MODE_INPUT);

  /**/
  GPIO_InitStruct.Pin = LL_GPIO_PIN_4;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_HIGH;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  LL_GPIO_Init(GPIOE, &GPIO_InitStruct);

  /**/
  GPIO_InitStruct.Pin = LL_GPIO_PIN_5;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_HIGH;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  LL_GPIO_Init(GPIOE, &GPIO_InitStruct);

  /**/
  GPIO_InitStruct.Pin = LL_GPIO_PIN_6;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_HIGH;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  LL_GPIO_Init(GPIOE, &GPIO_InitStruct);

  /**/
  GPIO_InitStruct.Pin = DI1_Pin;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  LL_GPIO_Init(DI1_GPIO_Port, &GPIO_InitStruct);

  /**/
  GPIO_InitStruct.Pin = DI2_Pin;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  LL_GPIO_Init(DI2_GPIO_Port, &GPIO_InitStruct);

  /**/
  GPIO_InitStruct.Pin = DI3_Pin;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_DOWN;
  LL_GPIO_Init(DI3_GPIO_Port, &GPIO_InitStruct);

  /**/
  GPIO_InitStruct.Pin = DI4_Pin;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_DOWN;
  LL_GPIO_Init(DI4_GPIO_Port, &GPIO_InitStruct);

  /**/
  GPIO_InitStruct.Pin = DI5_Pin;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_DOWN;
  LL_GPIO_Init(DI5_GPIO_Port, &GPIO_InitStruct);

  /**/
  GPIO_InitStruct.Pin = DI6_Pin;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_DOWN;
  LL_GPIO_Init(DI6_GPIO_Port, &GPIO_InitStruct);

  /**/
  GPIO_InitStruct.Pin = DI7_Pin;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_DOWN;
  LL_GPIO_Init(DI7_GPIO_Port, &GPIO_InitStruct);

  /**/
  GPIO_InitStruct.Pin = DI8_Pin;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_DOWN;
  LL_GPIO_Init(DI8_GPIO_Port, &GPIO_InitStruct);

  /**/
  GPIO_InitStruct.Pin = DI9_Pin;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_DOWN;
  LL_GPIO_Init(DI9_GPIO_Port, &GPIO_InitStruct);

  /**/
  GPIO_InitStruct.Pin = DI10_Pin;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_DOWN;
  LL_GPIO_Init(DI10_GPIO_Port, &GPIO_InitStruct);

  /**/
  GPIO_InitStruct.Pin = DI11_Pin;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_DOWN;
  LL_GPIO_Init(DI11_GPIO_Port, &GPIO_InitStruct);

  /**/
  GPIO_InitStruct.Pin = LD1_Pin;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  LL_GPIO_Init(LD1_GPIO_Port, &GPIO_InitStruct);

  /**/
  GPIO_InitStruct.Pin = DI12_Pin;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_DOWN;
  LL_GPIO_Init(DI12_GPIO_Port, &GPIO_InitStruct);

  /**/
  GPIO_InitStruct.Pin = DI13_Pin;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_DOWN;
  LL_GPIO_Init(DI13_GPIO_Port, &GPIO_InitStruct);

  /**/
  GPIO_InitStruct.Pin = DO1_Pin;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  LL_GPIO_Init(DO1_GPIO_Port, &GPIO_InitStruct);

  /**/
  GPIO_InitStruct.Pin = DO2_Pin;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  LL_GPIO_Init(DO2_GPIO_Port, &GPIO_InitStruct);

  /**/
  GPIO_InitStruct.Pin = LD3_Pin;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  LL_GPIO_Init(LD3_GPIO_Port, &GPIO_InitStruct);

  /**/
  GPIO_InitStruct.Pin = YI3_Pin;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_HIGH;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  LL_GPIO_Init(YI3_GPIO_Port, &GPIO_InitStruct);

  /**/
  GPIO_InitStruct.Pin = YI6_Pin;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_HIGH;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  LL_GPIO_Init(YI6_GPIO_Port, &GPIO_InitStruct);

  /**/
  GPIO_InitStruct.Pin = DO3_Pin;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  LL_GPIO_Init(DO3_GPIO_Port, &GPIO_InitStruct);

  /**/
  GPIO_InitStruct.Pin = DO4_Pin;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  LL_GPIO_Init(DO4_GPIO_Port, &GPIO_InitStruct);

  /**/
  GPIO_InitStruct.Pin = DO5_Pin;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  LL_GPIO_Init(DO5_GPIO_Port, &GPIO_InitStruct);

  /**/
  GPIO_InitStruct.Pin = DO6_Pin;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  LL_GPIO_Init(DO6_GPIO_Port, &GPIO_InitStruct);

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
  GPIO_InitStruct.Pin = DO9_Pin;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  LL_GPIO_Init(DO9_GPIO_Port, &GPIO_InitStruct);

  /**/
  GPIO_InitStruct.Pin = YI5_Pin;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_HIGH;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  LL_GPIO_Init(YI5_GPIO_Port, &GPIO_InitStruct);

  /**/
  GPIO_InitStruct.Pin = YI2_Pin;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_HIGH;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  LL_GPIO_Init(YI2_GPIO_Port, &GPIO_InitStruct);

  /**/
  GPIO_InitStruct.Pin = DO11_Pin;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  LL_GPIO_Init(DO11_GPIO_Port, &GPIO_InitStruct);

  /**/
  GPIO_InitStruct.Pin = DO13_Pin;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  LL_GPIO_Init(DO13_GPIO_Port, &GPIO_InitStruct);

  /**/
  GPIO_InitStruct.Pin = YI1_Pin;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_HIGH;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  LL_GPIO_Init(YI1_GPIO_Port, &GPIO_InitStruct);

  /**/
  GPIO_InitStruct.Pin = YI4_Pin;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_HIGH;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  LL_GPIO_Init(YI4_GPIO_Port, &GPIO_InitStruct);

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

// Вспомогательная функция отправки ошибок Modbus RTU
void Modbus_SendException(UART_HandleTypeDef *huart, uint8_t *tx_buf, uint8_t f_code, uint8_t exception_code) {
    tx_buf[0] = 1;                         // Slave ID
    tx_buf[1] = f_code | 0x80;             // Код ошибки
    tx_buf[2] = exception_code;            // Код исключения

    uint16_t crc = Modbus_CRC16(tx_buf, 3);
    tx_buf[3] = crc & 0xFF;
    tx_buf[4] = (crc >> 8) & 0xFF;

    // Выталкиваем из кэша (выравниваем размер до 32 байт для безопасности D-Cache)
    SCB_CleanDCache_by_Addr((uint32_t *)tx_buf, 32);
    HAL_UART_Transmit_DMA(huart, tx_buf, 5);
}

void Modbus_RTU_Parse(UART_HandleTypeDef *huart, uint8_t *rx_data, uint16_t length) {
    if (length < 8) return;

    // 1. Проверка CRC16 и Slave ID
    uint16_t rx_crc = (rx_data[length-1] << 8) | rx_data[length-2];
    if (Modbus_CRC16(rx_data, length - 2) != rx_crc) return;
    if (rx_data[0] != 1) return; // Наш адрес Modbus = 1

    uint8_t f_code = rx_data[1];
    uint16_t start_addr = (rx_data[2] << 8) | rx_data[3];
    uint16_t reg_count = (rx_data[4] << 8) | rx_data[5];

    uint8_t *tx_buf = NULL;
    uint16_t tx_len = 0;
    uint16_t *base_ptr = NULL;
    uint16_t db_max_regs = 0;

    // Флаги типа выбранного адреса
    bool is_param_db = false;
    bool is_log_db   = false;
    bool is_trace_db = false;

    // 2. Аппаратно разделяем буферы отправки DMA
    if (huart->Instance == USART3) {
        tx_buf = usart3_tx_buf;
    } else if (huart->Instance == USART6) {
        tx_buf = usart6_tx_buf;
    } else {
        return;
    }

    if (huart->gState != HAL_UART_STATE_READY) {
        return;
    }

    // 3. Маршрутизация по адресам памяти
    if (start_addr < 1000) {
        // --- Зона 0..999: DBParameters (Уставки) ---
        base_ptr = (uint16_t *)&DBParameters;
        db_max_regs = sizeof(DBParameters) / sizeof(uint16_t);
        is_param_db = true;
    }
    else if (start_addr >= 1000 && start_addr < 2000) {
        // --- Зона 1000..1999: DBMain (Оперативные данные) ---
        base_ptr = (uint16_t *)&DBMain;
        db_max_regs = sizeof(DBMain) / sizeof(uint16_t);
        start_addr -= 1000;
        is_param_db = false;
    }
    else if (start_addr >= 2000 && start_addr < 3000) {
        // --- Зона 2000..2999: Журнал Событий (EventLogBuffer) ---
        is_log_db = true;
    }
    else {
        // --- Зона 3000+: Высокоскоростной След (TraceBuffer) ---
        is_trace_db = true;
    }

    // =========================================================================
    // --- ФУНКЦИЯ 03 (0x03): Чтение Holding Registers ---
    // =========================================================================
    if (f_code == 0x03) {

        // ---------------------------------------------------------------------
        // ВЕТКА А: Чтение из Журнала Событий (EventLogBuffer)
        // ---------------------------------------------------------------------
        if (is_log_db) {
            // 1. Запрос статистики журнала (Адрес 2000, 3 регистра)
            if (start_addr == 2000 && reg_count == 3) {
                tx_buf[0] = 1;
                tx_buf[1] = 0x03;
                tx_buf[2] = 6;

                uint32_t total = EventLogBuffer.total_recorded;
                uint16_t count = (uint16_t)EventLogBuffer.count;

                tx_buf[3] = (total >> 24) & 0xFF;
                tx_buf[4] = (total >> 16) & 0xFF;
                tx_buf[5] = (total >> 8)  & 0xFF;
                tx_buf[6] = total & 0xFF;

                tx_buf[7] = (count >> 8) & 0xFF;
                tx_buf[8] = count & 0xFF;

                tx_len = 9;
            }
            // 2. Запрос пачки записей журнала (Адрес 2010+, reg_count кратен 8)
            else if (start_addr >= 2010 && (reg_count % 8) == 0) {
                uint32_t start_depth = (start_addr - 2010) / 8;
                uint16_t entries_requested = reg_count / 8;

                if (entries_requested > 15 || (3 + (reg_count * 2) + 2) > 256) {
                    Modbus_SendException(huart, tx_buf, f_code, 0x03);
                    return;
                }

                tx_buf[0] = 1;
                tx_buf[1] = 0x03;
                tx_buf[2] = entries_requested * 16;

                uint16_t tx_idx = 3;

                for (uint16_t i = 0; i < entries_requested; i++) {
                    LogEntry_t entry;
                    uint32_t current_depth = start_depth + i;

                    if (Log_GetEntry(current_depth, &entry)) {
                        tx_buf[tx_idx++] = (entry.timestamp >> 24) & 0xFF;
                        tx_buf[tx_idx++] = (entry.timestamp >> 16) & 0xFF;
                        tx_buf[tx_idx++] = (entry.timestamp >> 8)  & 0xFF;
                        tx_buf[tx_idx++] = entry.timestamp & 0xFF;

                        tx_buf[tx_idx++] = (entry.event_id >> 8) & 0xFF;
                        tx_buf[tx_idx++] = entry.event_id & 0xFF;

                        tx_buf[tx_idx++] = entry.severity;
                        tx_buf[tx_idx++] = entry.flags;

                        uint32_t val_raw;
                        memcpy(&val_raw, &entry.value, sizeof(float));
                        tx_buf[tx_idx++] = (val_raw >> 24) & 0xFF;
                        tx_buf[tx_idx++] = (val_raw >> 16) & 0xFF;
                        tx_buf[tx_idx++] = (val_raw >> 8)  & 0xFF;
                        tx_buf[tx_idx++] = val_raw & 0xFF;

                        tx_buf[tx_idx++] = (entry.param_id >> 24) & 0xFF;
                        tx_buf[tx_idx++] = (entry.param_id >> 16) & 0xFF;
                        tx_buf[tx_idx++] = (entry.param_id >> 8)  & 0xFF;
                        tx_buf[tx_idx++] = entry.param_id & 0xFF;
                    } else {
                        memset(&tx_buf[tx_idx], 0, 16);
                        tx_idx += 16;
                    }
                }
                tx_len = tx_idx;
            }
            else {
                Modbus_SendException(huart, tx_buf, f_code, 0x02);
                return;
            }
        }

        // ---------------------------------------------------------------------
        // ВЕТКА Б: Чтение из Высокоскоростного Следа (TraceBuffer + Snapshot)
        // ---------------------------------------------------------------------
        else if (is_trace_db) {

            // 1. Запрос статуса следа (Адрес 3000, 4 регистра = 8 байт)
            if (start_addr == 3000 && reg_count == 4) {

                // АТОМАРНЫЙ СНИМОК: Копируем весь кольцевой буфер в g_traceSnapshot за ~5 мкс
                __disable_irq();
                memcpy((void*)&g_traceSnapshot, (void*)&g_traceBuffer, sizeof(TraceBuffer_t));
                __enable_irq();

                tx_buf[0] = 1;
                tx_buf[1] = 0x03;
                tx_buf[2] = 8; // 4 регистра * 2 байта

                uint16_t head       = g_traceSnapshot.head;
                uint8_t  is_running = g_traceSnapshot.is_running;
                uint8_t  is_full    = g_traceSnapshot.is_full;
                uint32_t samples    = g_traceSnapshot.sample_count;

                    // Reg 3000: head (uint16_t)
                tx_buf[3] = (head >> 8) & 0xFF;
                tx_buf[4] = head & 0xFF;

                    // Reg 3001: is_running (High byte) + is_full (Low byte)
                tx_buf[5] = is_running;
                tx_buf[6] = is_full;

                // Reg 3002..3003: sample_count (uint32_t)
                tx_buf[7] = (samples >> 24) & 0xFF;
                tx_buf[8] = (samples >> 16) & 0xFF;
                tx_buf[9] = (samples >> 8)  & 0xFF;
                tx_buf[10] = samples & 0xFF;

                tx_len = 11;
            }

            // 2. Запрос пачки точек следа (Адрес 3010+, reg_count кратен 4)
            else if (start_addr >= 3010 && (reg_count % 4) == 0) {
                uint32_t start_point = (start_addr - 3010) / 4; // Индекс точки (0..999)
                uint16_t points_requested = reg_count / 4;     // Кол-во точек

                // Ограничение: макс 30 точек за 1 запрос (120 регистров = 240 байт)
                if (points_requested > 30 || (start_point + points_requested) > TRACE_SAMPLES) {
                    Modbus_SendException(huart, tx_buf, f_code, 0x03);
                    return;
                }

                tx_buf[0] = 1;
                tx_buf[1] = 0x03;
                tx_buf[2] = points_requested * 8; // 8 байт на точку (2x float)

                uint16_t tx_idx = 3;

                for (uint16_t i = 0; i < points_requested; i++) {
                    uint32_t idx = start_point + i;

                    // ВЫЧИТЫВАЕМ ИЗ СНИМКА (g_traceSnapshot), исключая data tearing!
                    float u_val = g_traceSnapshot.data[idx].useti;
                    float i_val = g_traceSnapshot.data[idx].iakb;

                    uint32_t u_raw, i_raw;
                    memcpy(&u_raw, &u_val, sizeof(float));
                    memcpy(&i_raw, &i_val, sizeof(float));

                    // Useti (float -> 4 байта)
                    tx_buf[tx_idx++] = (u_raw >> 24) & 0xFF;
                    tx_buf[tx_idx++] = (u_raw >> 16) & 0xFF;
                    tx_buf[tx_idx++] = (u_raw >> 8)  & 0xFF;
                    tx_buf[tx_idx++] = u_raw & 0xFF;

                    // Iakb (float -> 4 байта)
                    tx_buf[tx_idx++] = (i_raw >> 24) & 0xFF;
                    tx_buf[tx_idx++] = (i_raw >> 16) & 0xFF;
                    tx_buf[tx_idx++] = (i_raw >> 8)  & 0xFF;
                    tx_buf[tx_idx++] = i_raw & 0xFF;
                }

                tx_len = tx_idx;
            }
            else {
                Modbus_SendException(huart, tx_buf, f_code, 0x02); // Illegal Address
                return;
            }
        }

        // ---------------------------------------------------------------------
        // ВЕТКА В: Обычное чтение структуры DBParameters или DBMain
        // ---------------------------------------------------------------------
        else {
            if ((start_addr + reg_count) > db_max_regs || reg_count == 0) {
                Modbus_SendException(huart, tx_buf, f_code, 0x02);
                return;
            }
            if (3 + (reg_count * 2) + 2 > 256) {
                Modbus_SendException(huart, tx_buf, f_code, 0x03);
                return;
            }

            tx_buf[0] = 1;
            tx_buf[1] = 0x03;
            tx_buf[2] = reg_count * 2;

            for (int i = 0; i < reg_count; i++) {
                uint16_t val = base_ptr[start_addr + i];
                tx_buf[3 + i*2] = (val >> 8) & 0xFF;
                tx_buf[4 + i*2] = val & 0xFF;
            }
            tx_len = 3 + (reg_count * 2);
        }
    }

    // =========================================================================
    // --- ФУНКЦИЯ 16 (0x10): Запись нескольких регистров ---
    // =========================================================================
    else if (f_code == 0x10) {

        // В Журнал Событий запись запрещена!
        if (is_log_db) {
            Modbus_SendException(huart, tx_buf, f_code, 0x02);
            return;
        }

        // --- Управление Следом (Команды Старт / Стоп) ---
        if (is_trace_db) {
            if (start_addr == 3000 && reg_count == 1) {
                uint16_t cmd = (rx_data[7] << 8) | rx_data[8];
                if (cmd == 1) {
                    Trace_Start();
                } else if (cmd == 0) {
                    Trace_Stop();
                }

                // Эхо-ответ стандарта Modbus
                memcpy(tx_buf, rx_data, 6);
                tx_len = 6;
            } else {
                Modbus_SendException(huart, tx_buf, f_code, 0x02);
                return;
            }
        }
        // --- Обычная запись в DBParameters / DBMain ---
        else {
            if ((start_addr + reg_count) > db_max_regs || reg_count == 0) {
                Modbus_SendException(huart, tx_buf, f_code, 0x02);
                return;
            }

            for (int i = 0; i < reg_count; i++) {
                base_ptr[start_addr + i] = (rx_data[7 + i*2] << 8) | rx_data[8 + i*2];
            }

            if (is_param_db) {
                DB_UpdateCRC();
                Log_Write(LOG_EV_PARAM_CHANGED, LOG_SEV_INFO, DBParameters.f50.as_array[start_addr/2], start_addr);
            }

            memcpy(tx_buf, rx_data, 6);
            tx_len = 6;
        }
    }

    // =========================================================================
    // --- НЕПОДДЕРЖИВАЕМАЯ ФУНКЦИЯ ---
    // =========================================================================
    else {
        Modbus_SendException(huart, tx_buf, f_code, 0x01);
        return;
    }

    // 4. Расчет CRC16 и отправка ответа по DMA
    if (tx_len > 0) {
        uint16_t res_crc = Modbus_CRC16(tx_buf, tx_len);
        tx_buf[tx_len++] = res_crc & 0xFF;
        tx_buf[tx_len++] = (res_crc >> 8) & 0xFF;

        // Выравнивание длины до 32 байт для корректного сброса D-Cache
        uint32_t aligned_len = (tx_len + 31) & ~31;
        SCB_CleanDCache_by_Addr((uint32_t *)tx_buf, aligned_len);

        HAL_UART_Transmit_DMA(huart, tx_buf, tx_len);
    }
}

//void Modbus_RTU_Parse(UART_HandleTypeDef *huart, uint8_t *rx_data, uint16_t length) {
//    if (length < 8) return;
//
//    // 1. Проверка CRC16 и Slave ID
//    uint16_t rx_crc = (rx_data[length-1] << 8) | rx_data[length-2];
//    if (Modbus_CRC16(rx_data, length - 2) != rx_crc) return;
//    if (rx_data[0] != 1) return; // Наш адрес Modbus = 1
//
//    uint8_t f_code = rx_data[1];
//    uint16_t start_addr = (rx_data[2] << 8) | rx_data[3];
//    uint16_t reg_count = (rx_data[4] << 8) | rx_data[5];
//
//    uint8_t *tx_buf = NULL;
//    uint16_t tx_len = 0;
//    uint16_t *base_ptr = NULL;
//    uint16_t db_max_regs = 0; // Максимальный размер выбранной БД (в словах)
//
//    // Флаги типа выбранного адреса
//    bool is_param_db = false;
//    bool is_log_db   = false;
//
//    // 2. Аппаратно разделяем буферы отправки DMA
//    if (huart->Instance == USART3) {
//        tx_buf = usart3_tx_buf;
//    } else if (huart->Instance == USART6) {
//        tx_buf = usart6_tx_buf;
//    } else {
//        return; // Неизвестный интерфейс
//    }
//
//    // Проверяем, свободен ли передатчик DMA
//    if (huart->gState != HAL_UART_STATE_READY) {
//        return;
//    }
//
//    // 3. Маршрутизация по адресам памяти
//    if (start_addr < 1000) {
//        // --- Зона 0..999: DBParameters (Уставки) ---
//        base_ptr = (uint16_t *)&DBParameters;
//        db_max_regs = sizeof(DBParameters) / sizeof(uint16_t);
//        is_param_db = true;
//    }
//    else if (start_addr >= 1000 && start_addr < 2000) {
//        // --- Зона 1000..1999: DBMain (Оперативные данные) ---
//        base_ptr = (uint16_t *)&DBMain;
//        db_max_regs = sizeof(DBMain) / sizeof(uint16_t);
//        start_addr -= 1000;
//        is_param_db = false;
//    }
//    else {
//        // --- Зона 2000+: Журнал Событий (EventLogBuffer) ---
//        is_log_db = true;
//    }
//
//    // =========================================================================
//    // --- ФУНКЦИЯ 03 (0x03): Чтение Holding Registers ---
//    // =========================================================================
//    if (f_code == 0x03) {
//
//        // ---------------------------------------------------------------------
//        // ВЕТКА А: Чтение из Журнала Событий (Стратегия 1: Логическая адресация)
//        // ---------------------------------------------------------------------
//        if (is_log_db) {
//
//            // 1. Запрос статистики журнала (Адрес 2000, 3 регистра)
//            if (start_addr == 2000 && reg_count == 3) {
//                tx_buf[0] = 1;
//                tx_buf[1] = 0x03;
//                tx_buf[2] = 6; // 3 регистра * 2 байта
//
//                uint32_t total = EventLogBuffer.total_recorded;
//                uint16_t count = (uint16_t)EventLogBuffer.count;
//
//                // Упаковка total_recorded (uint32_t -> 4 байта)
//                tx_buf[3] = (total >> 24) & 0xFF;
//                tx_buf[4] = (total >> 16) & 0xFF;
//                tx_buf[5] = (total >> 8)  & 0xFF;
//                tx_buf[6] = total & 0xFF;
//
//                // Упаковка count (uint16_t -> 2 байта)
//                tx_buf[7] = (count >> 8) & 0xFF;
//                tx_buf[8] = count & 0xFF;
//
//                tx_len = 9;
//            }
//
//            // 2. Запрос пачки записей журнала (Адреса 2010+, reg_count кратен 8)
//            else if (start_addr >= 2010 && (reg_count % 8) == 0) {
//                uint32_t start_depth = (start_addr - 2010) / 8; // Глубина поиска
//                uint16_t entries_requested = reg_count / 8;     // Кол-во записей
//
//                // Ограничение размера пакета (максимум 15 записей = 120 регистров)
//                if (entries_requested > 15 || (3 + (reg_count * 2) + 2) > 256) {
//                    Modbus_SendException(huart, tx_buf, f_code, 0x03); // Illegal Data Value
//                    return;
//                }
//
//                tx_buf[0] = 1;
//                tx_buf[1] = 0x03;
//                tx_buf[2] = entries_requested * 16; // По 16 байт на запись
//
//                uint16_t tx_idx = 3;
//
//                for (uint16_t i = 0; i < entries_requested; i++) {
//                    LogEntry_t entry;
//                    uint32_t current_depth = start_depth + i;
//
//                    // Вычитываем запись хронологически из ring-буфера
//                    if (Log_GetEntry(current_depth, &entry)) {
//
//                        // timestamp (uint32_t)
//                        tx_buf[tx_idx++] = (entry.timestamp >> 24) & 0xFF;
//                        tx_buf[tx_idx++] = (entry.timestamp >> 16) & 0xFF;
//                        tx_buf[tx_idx++] = (entry.timestamp >> 8)  & 0xFF;
//                        tx_buf[tx_idx++] = entry.timestamp & 0xFF;
//
//                        // event_id (uint16_t)
//                        tx_buf[tx_idx++] = (entry.event_id >> 8) & 0xFF;
//                        tx_buf[tx_idx++] = entry.event_id & 0xFF;
//
//                        // severity & flags (uint8_t)
//                        tx_buf[tx_idx++] = entry.severity;
//                        tx_buf[tx_idx++] = entry.flags;
//
//                        // value (float)
//                        uint32_t val_raw;
//                        memcpy(&val_raw, &entry.value, sizeof(float));
//                        tx_buf[tx_idx++] = (val_raw >> 24) & 0xFF;
//                        tx_buf[tx_idx++] = (val_raw >> 16) & 0xFF;
//                        tx_buf[tx_idx++] = (val_raw >> 8)  & 0xFF;
//                        tx_buf[tx_idx++] = val_raw & 0xFF;
//
//                        // param_id (uint32_t)
//                        tx_buf[tx_idx++] = (entry.param_id >> 24) & 0xFF;
//                        tx_buf[tx_idx++] = (entry.param_id >> 16) & 0xFF;
//                        tx_buf[tx_idx++] = (entry.param_id >> 8)  & 0xFF;
//                        tx_buf[tx_idx++] = entry.param_id & 0xFF;
//
//                    } else {
//                        // Если зашли за пределы имеющихся записей — забиваем нулями
//                        memset(&tx_buf[tx_idx], 0, 16);
//                        tx_idx += 16;
//                    }
//                }
//
//                tx_len = tx_idx;
//            }
//            else {
//                Modbus_SendException(huart, tx_buf, f_code, 0x02); // Illegal Data Address
//                return;
//            }
//        }
//
//        // ---------------------------------------------------------------------
//        // ВЕТКА Б: Обычное чтение структуры DBParameters или DBMain
//        // ---------------------------------------------------------------------
//        else {
//            if ((start_addr + reg_count) > db_max_regs || reg_count == 0) {
//                Modbus_SendException(huart, tx_buf, f_code, 0x02); // Illegal Data Address
//                return;
//            }
//            if (3 + (reg_count * 2) + 2 > 256) {
//                Modbus_SendException(huart, tx_buf, f_code, 0x03); // Illegal Data Value
//                return;
//            }
//
//            tx_buf[0] = 1;
//            tx_buf[1] = 0x03;
//            tx_buf[2] = reg_count * 2;
//
//            for (int i = 0; i < reg_count; i++) {
//                uint16_t val = base_ptr[start_addr + i];
//                tx_buf[3 + i*2] = (val >> 8) & 0xFF;
//                tx_buf[4 + i*2] = val & 0xFF;
//            }
//            tx_len = 3 + (reg_count * 2);
//        }
//    }
//
//    // =========================================================================
//    // --- ФУНКЦИЯ 16 (0x10): Запись нескольких регистров ---
//    // =========================================================================
//    else if (f_code == 0x10) {
//
//        // В Журнал Событий запись по Modbus запрещена!
//        if (is_log_db) {
//            Modbus_SendException(huart, tx_buf, f_code, 0x02); // Illegal Data Address
//            return;
//        }
//
//        // Защита от выхода за границы БД при записи
//        if ((start_addr + reg_count) > db_max_regs || reg_count == 0) {
//            Modbus_SendException(huart, tx_buf, f_code, 0x02); // Illegal Data Address
//            return;
//        }
//
//        // Запись входящих данных в структуру
//        for (int i = 0; i < reg_count; i++) {
//            base_ptr[start_addr + i] = (rx_data[7 + i*2] << 8) | rx_data[8 + i*2];
//        }
//
//        // Если запись производилась в уставки (DBParameters)
//        if (is_param_db) {
//            DB_UpdateCRC(); // Обновляем контрольную сумму EEPROM/Flash
//
//            // Фиксируем факт изменения уставки в журнале событий
//            Log_Write(LOG_EV_PARAM_CHANGED, LOG_SEV_INFO, DBParameters.f50.as_array[start_addr/2], start_addr);
//        }
//
//        // Эхо-ответ стандарта Modbus
//        memcpy(tx_buf, rx_data, 6);
//        tx_len = 6;
//    }
//
//    // =========================================================================
//    // --- НЕПОДДЕРЖИВАЕМАЯ ФУНКЦИЯ ---
//    // =========================================================================
//    else {
//        Modbus_SendException(huart, tx_buf, f_code, 0x01); // Illegal Function
//        return;
//    }
//
//    // 4. Расчет CRC16 и отправка ответа по DMA
//    if (tx_len > 0) {
//        uint16_t res_crc = Modbus_CRC16(tx_buf, tx_len);
//        tx_buf[tx_len++] = res_crc & 0xFF;
//        tx_buf[tx_len++] = (res_crc >> 8) & 0xFF;
//
//        // Выравнивание длины до 32 байт для корректного сброса D-Cache
//        uint32_t aligned_len = (tx_len + 31) & ~31;
//        SCB_CleanDCache_by_Addr((uint32_t *)tx_buf, aligned_len);
//
//        HAL_UART_Transmit_DMA(huart, tx_buf, tx_len);
//    }
//}

//void Modbus_RTU_Parse(UART_HandleTypeDef *huart, uint8_t *rx_data, uint16_t length) {
//    if (length < 8) return;
//
//    // Проверка CRC и Slave ID
//    uint16_t rx_crc = (rx_data[length-1] << 8) | rx_data[length-2];
//    if (Modbus_CRC16(rx_data, length - 2) != rx_crc) return;
//    if (rx_data[0] != 1) return;
//
//    uint8_t f_code = rx_data[1];
//    uint16_t start_addr = (rx_data[2] << 8) | rx_data[3];
//    uint16_t reg_count = (rx_data[4] << 8) | rx_data[5];
//
//    uint8_t *tx_buf = NULL;
//    uint16_t tx_len = 0;
//    uint16_t *base_ptr = NULL;
//    uint16_t db_max_regs = 0; // Максимально доступный размер выбранной БД (в словах)
//
//    // 1. ФЛАГ ДЛЯ ОПРЕДЕЛЕНИЯ ВЫБРАННОЙ БАЗЫ ДАННЫХ
//        bool is_param_db = false;
//
//    // 1. Аппаратно разделяем буферы отправки
//    if (huart->Instance == USART3) {
//        tx_buf = usart3_tx_buf;
//    } else if (huart->Instance == USART6) {
//        tx_buf = usart6_tx_buf;
//    } else {
//        return; // Неизвестный интерфейс
//    }
//
//    // Проверяем, свободен ли передатчик DMA
//    if (huart->gState != HAL_UART_STATE_READY) {
//        return;
//    }
//
//    // 2. Выбор базы данных и определение жестких границ памяти
//    if (start_addr < 1000) {
//        base_ptr = (uint16_t *)&DBParameters;
//        db_max_regs = sizeof(DBParameters) / sizeof(uint16_t);
//        is_param_db = true; // <-- Это база уставок (DBParameters)
//    } else {
//        base_ptr = (uint16_t *)&DBMain;
//        db_max_regs = sizeof(DBMain) / sizeof(uint16_t);
//        start_addr -= 1000;
//        is_param_db = false; // <-- Это оперативная база (DBMain)
//    }
//
//    // --- ФУНКЦИЯ 03: ЧтениеHolding Registers ---
//    if (f_code == 0x03) {
//        // Защита от выхода за границы БД
//        if ((start_addr + reg_count) > db_max_regs || reg_count == 0) {
//            Modbus_SendException(huart, tx_buf, f_code, 0x02); // Illegal Data Address
//            return;
//        }
//        // Защита от переполнения нашего локального tx_buf (обычно 256 байт)
//        if (3 + (reg_count * 2) + 2 > 256) {
//            Modbus_SendException(huart, tx_buf, f_code, 0x03); // Illegal Data Value
//            return;
//        }
//
//        tx_buf[0] = 1;
//        tx_buf[1] = 0x03;
//        tx_buf[2] = reg_count * 2;
//
//        for (int i = 0; i < reg_count; i++) {
//            uint16_t val = base_ptr[start_addr + i];
//            tx_buf[3 + i*2] = (val >> 8) & 0xFF;
//            tx_buf[4 + i*2] = val & 0xFF;
//        }
//        tx_len = 3 + (reg_count * 2);
//    }
//
//    // --- ФУНКЦИЯ 16 (0x10): Запись нескольких регистров ---
//    else if (f_code == 0x10) {
//        // Защита от выхода за границы БД при записи
//        if ((start_addr + reg_count) > db_max_regs || reg_count == 0) {
//            Modbus_SendException(huart, tx_buf, f_code, 0x02); // Illegal Data Address
//            return;
//        }
//
//        // Выполняем безопасную запись данных
//        for (int i = 0; i < reg_count; i++) {
//            base_ptr[start_addr + i] = (rx_data[7 + i*2] << 8) | rx_data[8 + i*2];
//        }
//
//        // Если писали в энергонезависимые параметры — обновляем CRC
//        if (base_ptr == (uint16_t *)&DBParameters) {
//       // if (is_param_db) {
//            DB_UpdateCRC();
//            // Вносим запись в журнал: ИД события, Уровень, Новое значение, Индекс параметра
//            Log_Write(LOG_EV_PARAM_CHANGED, LOG_SEV_INFO, DBParameters.f50.as_array[start_addr/2], start_addr);
//
//        }
//
//        // Формируем эхо-ответ
//        memcpy(tx_buf, rx_data, 6);
//        tx_len = 6;
//    }
//
//    // --- НЕПОДДЕРЖИВАЕМАЯ ФУНКЦИЯ ---
//    else {
//        Modbus_SendException(huart, tx_buf, f_code, 0x01); // Illegal Function
//        return;
//    }
//
//    // 3. Отправка ответа
//    if (tx_len > 0) {
//        uint16_t res_crc = Modbus_CRC16(tx_buf, tx_len);
//        tx_buf[tx_len++] = res_crc & 0xFF;
//        tx_buf[tx_len++] = (res_crc >> 8) & 0xFF;
//
//        // КРИТИЧЕСКИ ВАЖНО: Округляем размер очистки кэша вверх до размера кэш-линии (32 байта)
//        // Это предотвращает побочные эффекты D-Cache для соседних ячеек памяти!
//        uint32_t aligned_len = (tx_len + 31) & ~31;
//        SCB_CleanDCache_by_Addr((uint32_t *)tx_buf, aligned_len);
//
//        HAL_UART_Transmit_DMA(huart, tx_buf, tx_len);
//    }
//}

//void Modbus_RTU_Parse(UART_HandleTypeDef *huart,uint8_t *rx_data, uint16_t length) {
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
//    //uint8_t tx_buf[256];
//    uint8_t *tx_buf = NULL;
//    uint16_t tx_len = 0;
//    uint16_t *base_ptr = NULL;
//
//    // 1. Аппаратно разделяем буферы отправки, чтобы USART3 и USART6 не терли данные друг другу
//        if (huart->Instance == USART3) {
//            tx_buf = usart3_tx_buf;
//        } else if (huart->Instance == USART6) {
//            tx_buf = usart6_tx_buf;
//        } else {
//            return; // Неизвестный UART
//        }
//
//        // Проверяем, не занят ли DMA отправкой предыдущего пакета этого интерфейса!
//        if (huart->gState != HAL_UART_STATE_READY) {
//            return; // Если прошлый пакет еще шлется, игнорируем новый, чтобы не вызвать аппаратную ошибку
//        }
//
//    // Выбор базы данных
//    if (start_addr < 1000) {
//        base_ptr = (uint16_t *)&DBParameters;
//    } else {
//        base_ptr = (uint16_t *)&DBMain;
//        start_addr -= 1000;
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
//        memcpy(tx_buf, rx_data, 6);
//        tx_len = 6;
//    }
//
//    // Отправка ответа
//    if (tx_len > 0) {
//        uint16_t res_crc = Modbus_CRC16(tx_buf, tx_len);
//        tx_buf[tx_len++] = res_crc & 0xFF;
//        tx_buf[tx_len++] = (res_crc >> 8) & 0xFF;
//        //HAL_UART_Transmit(huart, tx_buf, tx_len, 100);
//        // Пишем неблокирующий запуск DMA отправки:
//        // КРИТИЧЕСКИ ВАЖНО ДЛЯ STM32F7: Выталкиваем данные из кэша процессора в физическую RAM memory
//        // без этого DMA отправит старые данные или нули!
//        SCB_CleanDCache_by_Addr((uint32_t *)tx_buf, tx_len);
//
//        HAL_UART_Transmit_DMA(huart, tx_buf, tx_len);
//    }
//}

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
    	        Modbus6_Rx_Len = Size;
    	        Modbus6_New_Packet_Flag = 1;
//                Modbus_RTU_Parse(huart, rx_buf_usart6, Size);
//                HAL_UARTEx_ReceiveToIdle_DMA(&huart6, rx_buf_usart6, sizeof(rx_buf_usart6));
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
	HAL_NVIC_SetPriority(CAN1_RX0_IRQn, 5, 0);
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

static void MX_DMA_ADC_Init(void)
{
  LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_DMA2);

  LL_DMA_ConfigTransfer(DMA2, LL_DMA_STREAM_0,
                        LL_DMA_DIRECTION_PERIPH_TO_MEMORY |
                        LL_DMA_PRIORITY_HIGH              |
                        LL_DMA_MODE_CIRCULAR              |
                        LL_DMA_PERIPH_NOINCREMENT         |
                        LL_DMA_MEMORY_INCREMENT           |
                        LL_DMA_PDATAALIGN_HALFWORD        |
                        LL_DMA_MDATAALIGN_HALFWORD);

  LL_DMA_ConfigAddresses(DMA2, LL_DMA_STREAM_0,
                         LL_ADC_DMA_GetRegAddr(ADC1, LL_ADC_DMA_REG_REGULAR_DATA),
                         (uint32_t)aADCxConvertedData,
                         LL_DMA_DIRECTION_PERIPH_TO_MEMORY);

  LL_DMA_SetDataLength(DMA2, LL_DMA_STREAM_0, ADC_CHANNELS_COUNT);
  LL_DMA_SetChannelSelection(DMA2, LL_DMA_STREAM_0, LL_DMA_CHANNEL_0);

  // РАЗРЕШАЕМ ПРЕРЫВАНИЕ ПО ЗАВЕРШЕНИЮ (Transfer Complete)
  LL_DMA_EnableIT_TC(DMA2, LL_DMA_STREAM_0);

  // Настройка приоритета в NVIC
  NVIC_SetPriority(DMA2_Stream0_IRQn, 0);
  NVIC_EnableIRQ(DMA2_Stream0_IRQn);

  LL_DMA_EnableStream(DMA2, LL_DMA_STREAM_0);
}

static void MX_TIM2_Init(void)
{
  LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_TIM2);

  LL_TIM_SetPrescaler(TIM2, 0);
  LL_TIM_SetAutoReload(TIM2, 7199); // 15 kHz @ 108MHz
  LL_TIM_SetCounterMode(TIM2, LL_TIM_COUNTERMODE_UP);

  // Настройка Master Mode для генерации TRGO на событие Update
  LL_TIM_SetTriggerOutput(TIM2, LL_TIM_TRGO_UPDATE);

  LL_TIM_EnableCounter(TIM2);
}

void RAM_Sections_Init(void) {
    uint32_t *pSrc, *pDest;

    /* 1. Копируем критический код в ITCM-RAM (инструкции) */
    pSrc = &_sitcm_text;
    pDest = &_sitcm_ram;
    while (pDest < &_eitcm_ram) {
        *pDest++ = *pSrc++;
    }

    /* 2. Копируем быстрые данные в DTCM-RAM (переменные) */
    pSrc = &_sdtcm_data;
    pDest = &_sdtcm_ram;
    while (pDest < &_edtcm_ram) {
        *pDest++ = *pSrc++;
    }

    /* Опционально: барьер памяти, чтобы CPU "увидел" новые инструкции */
    __DSB(); // Data Synchronization Barrier
    __ISB(); // Instruction Synchronization Barrier
}

void Backup_SRAM_Init(void) {
    /* 1. Включаем тактирование модуля управления питанием (PWR) */
    __HAL_RCC_PWR_CLK_ENABLE();

    /* 2. Разрешаем доступ к Backup-домену (RTC и Backup SRAM) */
    HAL_PWR_EnableBkUpAccess();

    /* 3. Включаем тактирование самой Backup SRAM */
    __HAL_RCC_BKPSRAM_CLK_ENABLE();

    /* 4. (Опционально) Включаем регулятор низкого энергопотребления для Backup SRAM,
       чтобы данные сохранялись в режиме ожидания */
    HAL_PWREx_EnableBkUpReg();
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
