/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    stm32f7xx_it.c
  * @brief   Interrupt Service Routines.
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
#include "stm32f7xx_it.h"
/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "DI_Block.h"
#include "AI_Normalisation.h"
#include "Block_Synhro.h"
#include "Block_Sifu.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN TD */

/* USER CODE END TD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */
extern uint16_t aADCxConvertedData[];
extern uint32_t ADC_Accumulator[];
extern uint32_t ADC_SamplesCount;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/* External variables --------------------------------------------------------*/
extern CAN_HandleTypeDef hcan1;
extern DMA_HandleTypeDef hdma_usart3_rx;
extern DMA_HandleTypeDef hdma_usart3_tx;
extern DMA_HandleTypeDef hdma_usart6_rx;
extern DMA_HandleTypeDef hdma_usart6_tx;
extern UART_HandleTypeDef huart3;
extern UART_HandleTypeDef huart6;
/* USER CODE BEGIN EV */

/* USER CODE END EV */

/******************************************************************************/
/*           Cortex-M7 Processor Interruption and Exception Handlers          */
/******************************************************************************/
/**
  * @brief This function handles Non maskable interrupt.
  */
void NMI_Handler(void)
{
  /* USER CODE BEGIN NonMaskableInt_IRQn 0 */

  /* USER CODE END NonMaskableInt_IRQn 0 */
  /* USER CODE BEGIN NonMaskableInt_IRQn 1 */
   while (1)
  {
  }
  /* USER CODE END NonMaskableInt_IRQn 1 */
}

/**
  * @brief This function handles Hard fault interrupt.
  */
void HardFault_Handler(void)
{
  /* USER CODE BEGIN HardFault_IRQn 0 */

  /* USER CODE END HardFault_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_HardFault_IRQn 0 */
    /* USER CODE END W1_HardFault_IRQn 0 */
  }
}

/**
  * @brief This function handles Memory management fault.
  */
void MemManage_Handler(void)
{
  /* USER CODE BEGIN MemoryManagement_IRQn 0 */

  /* USER CODE END MemoryManagement_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_MemoryManagement_IRQn 0 */
    /* USER CODE END W1_MemoryManagement_IRQn 0 */
  }
}

/**
  * @brief This function handles Pre-fetch fault, memory access fault.
  */
void BusFault_Handler(void)
{
  /* USER CODE BEGIN BusFault_IRQn 0 */

  /* USER CODE END BusFault_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_BusFault_IRQn 0 */
    /* USER CODE END W1_BusFault_IRQn 0 */
  }
}

/**
  * @brief This function handles Undefined instruction or illegal state.
  */
void UsageFault_Handler(void)
{
  /* USER CODE BEGIN UsageFault_IRQn 0 */

  /* USER CODE END UsageFault_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_UsageFault_IRQn 0 */
    /* USER CODE END W1_UsageFault_IRQn 0 */
  }
}

/**
  * @brief This function handles System service call via SWI instruction.
  */
void SVC_Handler(void)
{
  /* USER CODE BEGIN SVCall_IRQn 0 */

  /* USER CODE END SVCall_IRQn 0 */
  /* USER CODE BEGIN SVCall_IRQn 1 */

  /* USER CODE END SVCall_IRQn 1 */
}

/**
  * @brief This function handles Debug monitor.
  */
void DebugMon_Handler(void)
{
  /* USER CODE BEGIN DebugMonitor_IRQn 0 */

  /* USER CODE END DebugMonitor_IRQn 0 */
  /* USER CODE BEGIN DebugMonitor_IRQn 1 */

  /* USER CODE END DebugMonitor_IRQn 1 */
}

/**
  * @brief This function handles Pendable request for system service.
  */
void PendSV_Handler(void)
{
  /* USER CODE BEGIN PendSV_IRQn 0 */

  /* USER CODE END PendSV_IRQn 0 */
  /* USER CODE BEGIN PendSV_IRQn 1 */

  /* USER CODE END PendSV_IRQn 1 */
}

/**
  * @brief This function handles System tick timer.
  */
void SysTick_Handler(void)
{
  /* USER CODE BEGIN SysTick_IRQn 0 */

  /* USER CODE END SysTick_IRQn 0 */
  HAL_IncTick();
  /* USER CODE BEGIN SysTick_IRQn 1 */
  // Проверяем состояние фаз каждую миллисекунду
    Sync_Watchdog_Check();
  // проверяем чередование и формируем сигнал готовности сети
  //  Sync_CheckSequence();

  /* USER CODE END SysTick_IRQn 1 */
}

/******************************************************************************/
/* STM32F7xx Peripheral Interrupt Handlers                                    */
/* Add here the Interrupt Handlers for the used peripherals.                  */
/* For the available peripheral interrupt handler names,                      */
/* please refer to the startup file (startup_stm32f7xx.s).                    */
/******************************************************************************/

/**
  * @brief This function handles DMA1 stream1 global interrupt.
  */
void DMA1_Stream1_IRQHandler(void)
{
  /* USER CODE BEGIN DMA1_Stream1_IRQn 0 */

  /* USER CODE END DMA1_Stream1_IRQn 0 */
  HAL_DMA_IRQHandler(&hdma_usart3_rx);
  /* USER CODE BEGIN DMA1_Stream1_IRQn 1 */

  /* USER CODE END DMA1_Stream1_IRQn 1 */
}

/**
  * @brief This function handles DMA1 stream3 global interrupt.
  */
void DMA1_Stream3_IRQHandler(void)
{
  /* USER CODE BEGIN DMA1_Stream3_IRQn 0 */

  /* USER CODE END DMA1_Stream3_IRQn 0 */
  HAL_DMA_IRQHandler(&hdma_usart3_tx);
  /* USER CODE BEGIN DMA1_Stream3_IRQn 1 */

  /* USER CODE END DMA1_Stream3_IRQn 1 */
}

/**
  * @brief This function handles CAN1 RX0 interrupts.
  */
void CAN1_RX0_IRQHandler(void)
{
  /* USER CODE BEGIN CAN1_RX0_IRQn 0 */

  /* USER CODE END CAN1_RX0_IRQn 0 */
  HAL_CAN_IRQHandler(&hcan1);
  /* USER CODE BEGIN CAN1_RX0_IRQn 1 */

  /* USER CODE END CAN1_RX0_IRQn 1 */
}

/**
  * @brief This function handles USART3 global interrupt.
  */
void USART3_IRQHandler(void)
{
  /* USER CODE BEGIN USART3_IRQn 0 */

  /* USER CODE END USART3_IRQn 0 */
  HAL_UART_IRQHandler(&huart3);
  /* USER CODE BEGIN USART3_IRQn 1 */

  /* USER CODE END USART3_IRQn 1 */
}

/**
  * @brief This function handles TIM6 global interrupt, DAC1 and DAC2 underrun error interrupts.
  */
void TIM6_DAC_IRQHandler(void)
{
  /* USER CODE BEGIN TIM6_DAC_IRQn 0 */

	// 1. Аппаратно проверяем: прерывание случилось именно от переполнения TIM6?
	  if (LL_TIM_IsActiveFlag_UPDATE(TIM6))
	  {
	    // 2. ОБЯЗАТЕЛЬНО сбрасываем флаг, иначе процессор зациклится в прерывании навсегда
	    LL_TIM_ClearFlag_UPDATE(TIM6);

	    static uint8_t step = 0;

	    // Имитация трехфазного меандра 50 Гц со сдвигом 120° (шаги таймера по 60°)
	    // Нам важны моменты установки в 1 (Rising Edge) — это точки синхронизации СИФУ.
	    //
	    // Назначение пинов имитатора (соединить проводками на плате Nucleo):
	    // PE4 (Имитация Фазы А) -> подключить к PA6 (Вход синхронизации TIM3_CH1)
	    // PE5 (Имитация Фазы B) -> подключить к PD14 (Вход синхронизации TIM4_CH3)
	    // PE6 (Имитация Фазы C) -> подключить к PC9 (Вход синхронизации TIM8_CH4)
	    switch (step)
	    {
	      case 0: // 0°
	        LL_GPIO_SetOutputPin(GPIOE, LL_GPIO_PIN_4);   // Фаза А: ВВЕРХ (Захват нуля А!)
	        LL_GPIO_ResetOutputPin(GPIOE, LL_GPIO_PIN_5); // Фаза B: НИЗ
	        LL_GPIO_SetOutputPin(GPIOE, LL_GPIO_PIN_6);   // Фаза C: ВВЕРХ
	        break;

	      case 1: // 60°
	        LL_GPIO_ResetOutputPin(GPIOE, LL_GPIO_PIN_6); // Фаза C: НИЗ
	        break;

	      case 2: // 120°
	        LL_GPIO_SetOutputPin(GPIOE, LL_GPIO_PIN_5);   // Фаза B: ВВЕРХ (Захват нуля B!)
	        break;

	      case 3: // 180°
	        LL_GPIO_ResetOutputPin(GPIOE, LL_GPIO_PIN_4); // Фаза А: НИЗ
	        break;

	      case 4: // 240°
	        LL_GPIO_SetOutputPin(GPIOE, LL_GPIO_PIN_6);   // Фаза C: ВВЕРХ (Захват нуля C!)
	        break;

	      case 5: // 300°
	        LL_GPIO_ResetOutputPin(GPIOE, LL_GPIO_PIN_5); // Фаза B: НИЗ
	        break;
	    }

	    step = (step + 1) % 6; // Циклический счетчик шагов коммутации моста
	  }

  /* USER CODE END TIM6_DAC_IRQn 0 */
  /* USER CODE BEGIN TIM6_DAC_IRQn 1 */

  /* USER CODE END TIM6_DAC_IRQn 1 */
}

/**
  * @brief This function handles DMA2 stream1 global interrupt.
  */
void DMA2_Stream1_IRQHandler(void)
{
  /* USER CODE BEGIN DMA2_Stream1_IRQn 0 */

  /* USER CODE END DMA2_Stream1_IRQn 0 */
  HAL_DMA_IRQHandler(&hdma_usart6_rx);
  /* USER CODE BEGIN DMA2_Stream1_IRQn 1 */

  /* USER CODE END DMA2_Stream1_IRQn 1 */
}

/**
  * @brief This function handles DMA2 stream6 global interrupt.
  */
void DMA2_Stream6_IRQHandler(void)
{
  /* USER CODE BEGIN DMA2_Stream6_IRQn 0 */

  /* USER CODE END DMA2_Stream6_IRQn 0 */
  HAL_DMA_IRQHandler(&hdma_usart6_tx);
  /* USER CODE BEGIN DMA2_Stream6_IRQn 1 */

  /* USER CODE END DMA2_Stream6_IRQn 1 */
}

/**
  * @brief This function handles USART6 global interrupt.
  */
void USART6_IRQHandler(void)
{
  /* USER CODE BEGIN USART6_IRQn 0 */

  /* USER CODE END USART6_IRQn 0 */
  HAL_UART_IRQHandler(&huart6);
  /* USER CODE BEGIN USART6_IRQn 1 */

  /* USER CODE END USART6_IRQn 1 */
}

/* USER CODE BEGIN 1 */
/**
  * @brief Обработка внешних прерываний EXTI 0, 1, 2
  */
void EXTI0_IRQHandler(void)
{
  if (LL_EXTI_IsActiveFlag_0_31(LL_EXTI_LINE_0) != RESET)
  {
    LL_EXTI_ClearFlag_0_31(LL_EXTI_LINE_0);
    Process_Frequency_Input(0);
  }
}

void EXTI1_IRQHandler(void)
{
  if (LL_EXTI_IsActiveFlag_0_31(LL_EXTI_LINE_1) != RESET)
  {
    LL_EXTI_ClearFlag_0_31(LL_EXTI_LINE_1);
    Process_Frequency_Input(1);
  }
}

void EXTI2_IRQHandler(void)
{
  if (LL_EXTI_IsActiveFlag_0_31(LL_EXTI_LINE_2) != RESET)
  {
    LL_EXTI_ClearFlag_0_31(LL_EXTI_LINE_2);
    Process_Frequency_Input(2);
  }
}

// Этот обработчик вызывается при прерываниях на пинах с 10 по 15
void EXTI15_10_IRQHandler(void)
{
  // Проверяем, что прерывание пришло именно от 13-го пина (нашей кнопки)
  if (LL_EXTI_IsActiveFlag_0_31(LL_EXTI_LINE_13) != RESET)
  {
    LL_EXTI_ClearFlag_0_31(LL_EXTI_LINE_13); // Очищаем флаг

    // Вызываем ваш рабочий обработчик для Канала 0 (который раньше висел на PE0)
    Process_Frequency_Input(0);
  }
}

void DMA2_Stream0_IRQHandler(void)
{
  /* Проверяем флаг завершения передачи Stream 0 */
  if (LL_DMA_IsActiveFlag_TC0(DMA2))
  {
    LL_DMA_ClearFlag_TC0(DMA2); // Сбрасываем флаг

    /* ИНТЕГРИРУЮЩИЙ МЕТОД: Накапливаем сумму по каждому каналу */
//    for (int i = 0; i < 6; i++) {
//      ADC_Accumulator[i] += aADCxConvertedData[i];
//    }
//    ADC_SamplesCount++; // Считаем количество выборок
    AI_Accumulate(aADCxConvertedData);
  }
}

/**
  * @brief Фаза А (TIM3, вход PA6)
  */
void TIM3_IRQHandler(void)
{
  if (LL_TIM_IsActiveFlag_CC1(TIM3))
  {
    LL_TIM_ClearFlag_CC1(TIM3);
    uint32_t capture = LL_TIM_IC_GetCaptureCH1(TIM3);

    // Вызов общей логики
    PhaseA.GlobalCapture = DWT->CYCCNT;
    Sync_Process_Phase(&PhaseA, capture);

    // Расчет и установка Output Compare (УИ1 и УИ4)
    if (DBMain.b64.EnableSifu == 0){
    	Sifu_DisableYI14();
        float ticks_per_degree = PhaseA.PeriodFiltered / 360.0f;
        uint32_t alpha_ticks = (uint32_t)(ticks_per_degree * DBMain.f50.Alfa_ref);
        uint32_t half_period = (uint32_t)(PhaseA.PeriodFiltered / 2.0f);

        LL_TIM_OC_SetCompareCH2(TIM3, PhaseA.T_zero + alpha_ticks);              // УИ1 (PA7)
        LL_TIM_OC_SetCompareCH3(TIM3, PhaseA.T_zero + alpha_ticks + half_period); // УИ4 (PB0)
    }
    else{
    	if(DBMain.b96.EnableSifuOld==0){
    		CalculateNextImpuls(1);
    	}

    }
    DBMain.b96.EnableSifuOld = DBMain.b64.EnableSifu;

  }
  /* === 2. СОБЫТИЕ СРАВНЕНИЯ КАНАЛ 2 (УИ1 - PA7) === */
  if (LL_TIM_IsActiveFlag_CC2(TIM3))
  {
      LL_TIM_ClearFlag_CC2(TIM3); // Обязательный сброс
      //Берем текущее значение, при котором сработало прерывание
      uint32_t current_ccr = LL_TIM_OC_GetCompareCH2(TIM3);
      if(DBMain.b96.PulseStage1==1){
    	  LL_GPIO_ResetOutputPin(GPIOB, LL_GPIO_PIN_4); // УИ1
    	  LL_GPIO_ResetOutputPin(GPIOD, LL_GPIO_PIN_15); // УИ6
    	  Sifu_DisableYI1();
    	  DBMain.b96.PulseStage1=0;
      }
      else
      {
      if (DBMain.b64.EnableSifu == 1) {
    	  Generate_Thyristor_Pulse(1, current_ccr);
      }
      else
      {
    	  Generate_Thyristor_Pulse(0, current_ccr);
      }
      //Process_Analog_Measurements();
      //ProtectSystem();
      //TehnologSystem();
      //RegulationSystem();
      CalculateNextImpuls(2);
      }
  }
  /* === 3. СОБЫТИЕ СРАВНЕНИЯ КАНАЛ 3 (УИ4 - PB0) === */
  if (LL_TIM_IsActiveFlag_CC3(TIM3))
  {
      LL_TIM_ClearFlag_CC3(TIM3);
      uint32_t current_ccr = LL_TIM_OC_GetCompareCH3(TIM3);
      if(DBMain.b96.PulseStage4==1){
    	  LL_GPIO_ResetOutputPin(GPIOB, LL_GPIO_PIN_5); // УИ4
    	  LL_GPIO_ResetOutputPin(GPIOD, LL_GPIO_PIN_13); // УИ3
    	  Sifu_DisableYI4();
    	  DBMain.b96.PulseStage4=0;
      }
      else
      {
      if (DBMain.b64.EnableSifu == 1) {
    	  Generate_Thyristor_Pulse(4, current_ccr);
      }
      else
      {
    	  Generate_Thyristor_Pulse(0, current_ccr);
      }
      //Process_Analog_Measurements();
      //ProtectSystem();
      //TehnologSystem();
      //RegulationSystem();
      CalculateNextImpuls(5);
      }
  }
}
/**
  * @brief Фаза B (TIM4, вход PD14)
  */
void TIM4_IRQHandler(void)
{
  if (LL_TIM_IsActiveFlag_CC3(TIM4))
  {
    LL_TIM_ClearFlag_CC3(TIM4);
    uint32_t capture = LL_TIM_IC_GetCaptureCH3(TIM4);

    PhaseB.GlobalCapture = DWT->CYCCNT;
    deltaAB = PhaseB.GlobalCapture - PhaseA.GlobalCapture;
    angleAB = (float)deltaAB * 360.0f / period;
    Sync_Process_Phase(&PhaseB, capture);

    if (DBMain.b64.EnableSifu == 0){
    	Sifu_DisableYI36();
        float ticks_per_degree = PhaseB.PeriodFiltered / 360.0f;
        uint32_t alpha_ticks = (uint32_t)(ticks_per_degree * DBMain.f50.Alfa_ref);
        uint32_t half_period = (uint32_t)(PhaseB.PeriodFiltered / 2.0f);

        LL_TIM_OC_SetCompareCH2(TIM4, PhaseB.T_zero + alpha_ticks);              // УИ3 (PD13)
        LL_TIM_OC_SetCompareCH4(TIM4, PhaseB.T_zero + alpha_ticks + half_period); // УИ6 (PD15)
    }
    else{
        	if(DBMain.b96.EnableSifuOld==0){
        		CalculateNextImpuls(3);

        	}

    }
    DBMain.b96.EnableSifuOld = DBMain.b64.EnableSifu;

  }
  /* === 2. СОБЫТИЕ СРАВНЕНИЯ КАНАЛ 2 (УИ3 - PD13) === */
  if (LL_TIM_IsActiveFlag_CC2(TIM4))
  {
        LL_TIM_ClearFlag_CC2(TIM4); //
        uint32_t current_ccr = LL_TIM_OC_GetCompareCH2(TIM4);
        if(DBMain.b96.PulseStage3==1){
          LL_GPIO_ResetOutputPin(GPIOD, LL_GPIO_PIN_13); // УИ3
      	  LL_GPIO_ResetOutputPin(GPIOC, LL_GPIO_PIN_7);  // УИ2
      	  Sifu_DisableYI3();
      	  DBMain.b96.PulseStage3=0;
        }
        else
        {
        if (DBMain.b64.EnableSifu == 1) {
        	Generate_Thyristor_Pulse(3, current_ccr);
        }
        else
        {
      	  Generate_Thyristor_Pulse(0, current_ccr);
        }
        //Process_Analog_Measurements();
        //ProtectSystem();
        //TehnologSystem();
        //RegulationSystem();
        CalculateNextImpuls(4);
        }
  }
  /* === 3. СОБЫТИЕ СРАВНЕНИЯ КАНАЛ 4 (УИ6 - PD15) === */
  if (LL_TIM_IsActiveFlag_CC4(TIM4))
  {
        LL_TIM_ClearFlag_CC4(TIM4);
        uint32_t current_ccr = LL_TIM_OC_GetCompareCH4(TIM4);
        if(DBMain.b96.PulseStage6==1){
      	  LL_GPIO_ResetOutputPin(GPIOD, LL_GPIO_PIN_15); // УИ6
      	  LL_GPIO_ResetOutputPin(GPIOC, LL_GPIO_PIN_6);  // УИ5
      	  Sifu_DisableYI6();
      	  DBMain.b96.PulseStage6=0;
        }
        else
        {
        if (DBMain.b64.EnableSifu == 1) {
        	Generate_Thyristor_Pulse(6, current_ccr);
        }
        else
        {
      	  Generate_Thyristor_Pulse(0, current_ccr);
        }
        //Process_Analog_Measurements();
        //ProtectSystem();
        //TehnologSystem();
        //RegulationSystem();
        CalculateNextImpuls(1);
        }
  }
}
/**
  * @brief Фаза C (TIM8, вход PC9)
  */
void TIM8_CC_IRQHandler(void)
{
  if (LL_TIM_IsActiveFlag_CC4(TIM8))
  {
    LL_TIM_ClearFlag_CC4(TIM8);
    uint32_t capture = LL_TIM_IC_GetCaptureCH4(TIM8);

    PhaseC.GlobalCapture = DWT->CYCCNT;
    deltaAC = PhaseC.GlobalCapture - PhaseA.GlobalCapture;
    angleAC = (float)deltaAC * 360.0f / period;
    Sync_Process_Phase(&PhaseC, capture);

    if (DBMain.b64.EnableSifu == 0){
    	Sifu_DisableYI52();
        float ticks_per_degree = PhaseC.PeriodFiltered / 360.0f;
        uint32_t alpha_ticks = (uint32_t)(ticks_per_degree * DBMain.f50.Alfa_ref);
        uint32_t half_period = (uint32_t)(PhaseC.PeriodFiltered / 2.0f);

        LL_TIM_OC_SetCompareCH1(TIM8, PhaseC.T_zero + alpha_ticks);              // УИ5 (PC6)
        LL_TIM_OC_SetCompareCH2(TIM8, PhaseC.T_zero + alpha_ticks + half_period); // УИ2 (PC7)
    }
    else{
        	if(DBMain.b96.EnableSifuOld==0){
        		CalculateNextImpuls(5);
        	}

    }
    DBMain.b96.EnableSifuOld = DBMain.b64.EnableSifu;
  }
  /* === 2. СОБЫТИЕ СРАВНЕНИЯ КАНАЛ 1 (УИ5 - PC6) === */
  if (LL_TIM_IsActiveFlag_CC1(TIM8))
  {
          LL_TIM_ClearFlag_CC1(TIM8); //
          uint32_t current_ccr = LL_TIM_OC_GetCompareCH1(TIM8);
          if(DBMain.b96.PulseStage5==1){
        	  LL_GPIO_ResetOutputPin(GPIOC, LL_GPIO_PIN_6);  // УИ5
        	  LL_GPIO_ResetOutputPin(GPIOB, LL_GPIO_PIN_5); // УИ4
        	  Sifu_DisableYI5();
        	  DBMain.b96.PulseStage5=0;
          }
          else
          {
          if (DBMain.b64.EnableSifu == 1) {
        	  Generate_Thyristor_Pulse(5, current_ccr);
          }
          else
          {
        	  Generate_Thyristor_Pulse(0, current_ccr);
          }
          //Process_Analog_Measurements();
          //ProtectSystem();
          //TehnologSystem();
          //RegulationSystem();
          CalculateNextImpuls(6);
          }
  }
  /* === 3. СОБЫТИЕ СРАВНЕНИЯ КАНАЛ 2 (УИ2 - PC7) === */
  if (LL_TIM_IsActiveFlag_CC2(TIM8))
  {
          LL_TIM_ClearFlag_CC2(TIM8);
          uint32_t current_ccr = LL_TIM_OC_GetCompareCH2(TIM8);
          if(DBMain.b96.PulseStage2==1){
        	  LL_GPIO_ResetOutputPin(GPIOC, LL_GPIO_PIN_7);  // УИ2
        	  LL_GPIO_ResetOutputPin(GPIOB, LL_GPIO_PIN_4); // УИ1
        	  Sifu_DisableYI2();
        	  DBMain.b96.PulseStage2=0;
          }
          else
          {
          if (DBMain.b64.EnableSifu == 1) {
        	  Generate_Thyristor_Pulse(2, current_ccr);
          }
          else
          {
        	  Generate_Thyristor_Pulse(0, current_ccr);
          }
          //Process_Analog_Measurements();
          //ProtectSystem();
          //TehnologSystem();
          //RegulationSystem();
          CalculateNextImpuls(3);
          }
  }

  /* === КОРЕННАЯ ЗАЧИСТКА ХВОСТОВ === */
    // Если прерывание вызвано не обработанным флагом (например CC3),
    // эта строчка принудительно его погасит и не даст процессору зависнуть.
  //  TIM8->SR = ~(TIM_SR_CC1IF | TIM_SR_CC2IF | TIM_SR_CC3IF | TIM_SR_CC4IF | TIM_SR_CC1OF | TIM_SR_CC2OF | TIM_SR_CC3OF | TIM_SR_CC4OF);

}

void TIM7_IRQHandler(void)
{
    if (LL_TIM_IsActiveFlag_UPDATE(TIM7))
    {
        LL_TIM_ClearFlag_UPDATE(TIM7);

        // Мгновенный сброс всех пинов УИ в 0
        LL_GPIO_ResetOutputPin(GPIOB, LL_GPIO_PIN_4); // УИ1
        LL_GPIO_ResetOutputPin(GPIOB, LL_GPIO_PIN_5); // УИ4
        LL_GPIO_ResetOutputPin(GPIOD, LL_GPIO_PIN_13); // УИ3
        LL_GPIO_ResetOutputPin(GPIOD, LL_GPIO_PIN_15); // УИ6
        LL_GPIO_ResetOutputPin(GPIOC, LL_GPIO_PIN_6);  // УИ5
        LL_GPIO_ResetOutputPin(GPIOC, LL_GPIO_PIN_7);  // УИ2
    }
}


/* USER CODE END 1 */
