#include "main.h"
#include "Block_Sifu.h"
#include "Block_Synhro.h"
#include "AI_Normalisation.h"
#include "DI_Block.h"

void Generate_Thyristor_Pulse(uint8_t pulse_num, uint32_t current_ccr){
    DBMain.b96.PulseStage1 = 0;
    DBMain.b96.PulseStage2 = 0;
    DBMain.b96.PulseStage3 = 0;
    DBMain.b96.PulseStage4 = 0;
    DBMain.b96.PulseStage5 = 0;
    DBMain.b96.PulseStage6 = 0;
	switch(pulse_num) {
	        case 1:
	            // УИ1 основной (PA7), УИ6 подтверждающий
	            LL_GPIO_SetOutputPin(GPIOA, LL_GPIO_PIN_7); // УИ1
	            LL_GPIO_SetOutputPin(GPIOD, LL_GPIO_PIN_15); // УИ6
	            Sifu_DisableYI(pulse_num);
	            Sifu_StartPulseWidth();
	            //DBMain.b96.PulseStage1 = 1;
	            //DBMain.b96.PulseStage6 = 1;
//	            LL_TIM_OC_SetCompareCH2(TIM3, current_ccr + PhaseA.T_Pulse_width);
//	            Sifu_EnableYI(pulse_num);
	            break;
	        case 2:
	            // УИ2 основной, УИ1 подтверждающий
	            LL_GPIO_SetOutputPin(GPIOC, LL_GPIO_PIN_7);  // УИ2
	            LL_GPIO_SetOutputPin(GPIOA, LL_GPIO_PIN_7); // УИ1
	            Sifu_DisableYI(pulse_num);
	            Sifu_StartPulseWidth();
	            //DBMain.b96.PulseStage2 = 1;
	            //DBMain.b96.PulseStage1 = 1;
	            //LL_TIM_OC_SetCompareCH2(TIM8, current_ccr + PhaseC.T_Pulse_width);
	            //Sifu_EnableYI(pulse_num);
	            break;
	        case 3:
	            // УИ3 основной, УИ2 подтверждающий
	           	LL_GPIO_SetOutputPin(GPIOD, LL_GPIO_PIN_13); // УИ3
	        	LL_GPIO_SetOutputPin(GPIOC, LL_GPIO_PIN_7);  // УИ2
	            Sifu_DisableYI(pulse_num);
	            Sifu_StartPulseWidth();
	            //DBMain.b96.PulseStage3 = 1;
	            //DBMain.b96.PulseStage2 = 1;
	        	//LL_TIM_OC_SetCompareCH2(TIM4, current_ccr + PhaseB.T_Pulse_width);
	        	//Sifu_EnableYI(pulse_num);
	            break;
	        case 4:
	            // УИ4 основной, УИ3 подтверждающий
	            LL_GPIO_SetOutputPin(GPIOB, LL_GPIO_PIN_0); // УИ4
	        	LL_GPIO_SetOutputPin(GPIOD, LL_GPIO_PIN_13); // УИ3
	            Sifu_DisableYI(pulse_num);
	            Sifu_StartPulseWidth();
	            //DBMain.b96.PulseStage4 = 1;
	            //DBMain.b96.PulseStage3 = 1;
	        	//LL_TIM_OC_SetCompareCH3(TIM3, current_ccr + PhaseA.T_Pulse_width);
	        	//Sifu_EnableYI(pulse_num);
	            break;
	        case 5:
	            // УИ5 основной, УИ4 подтверждающий
	        	LL_GPIO_SetOutputPin(GPIOC, LL_GPIO_PIN_6);  // УИ5
	        	LL_GPIO_SetOutputPin(GPIOB, LL_GPIO_PIN_0); // УИ4
	            Sifu_DisableYI(pulse_num);
	            Sifu_StartPulseWidth();
	            //DBMain.b96.PulseStage5 = 1;
	            //DBMain.b96.PulseStage4 = 1;
	        	//LL_TIM_OC_SetCompareCH1(TIM8, current_ccr + PhaseC.T_Pulse_width);
	        	//Sifu_EnableYI(pulse_num);
	            break;
	        case 6:
	            // УИ6 основной, УИ5 подтверждающий
	        	LL_GPIO_SetOutputPin(GPIOD, LL_GPIO_PIN_15); // УИ6
	        	LL_GPIO_SetOutputPin(GPIOC, LL_GPIO_PIN_6);  // УИ5
	            Sifu_DisableYI(pulse_num);
	            Sifu_StartPulseWidth();
	            //DBMain.b96.PulseStage6 = 1;
	            //DBMain.b96.PulseStage5 = 1;
	        	//LL_TIM_OC_SetCompareCH4(TIM4, current_ccr + PhaseB.T_Pulse_width);
	        	//Sifu_EnableYI(pulse_num);
	            break;
	        default:
	        	  // Принудительный сброс физических уровней на всех пинах УИ в 0 (аппаратная блокировка)
	        	  // Группа Фазы А
	        	  LL_GPIO_ResetOutputPin(GPIOA, LL_GPIO_PIN_7); // УИ1
	        	  LL_GPIO_ResetOutputPin(GPIOB, LL_GPIO_PIN_0); // УИ4

	        	  // Группа Фазы B
	        	  LL_GPIO_ResetOutputPin(GPIOD, LL_GPIO_PIN_13); // УИ3
	        	  LL_GPIO_ResetOutputPin(GPIOD, LL_GPIO_PIN_15); // УИ6

	        	  // Группа Фазы C
	        	  LL_GPIO_ResetOutputPin(GPIOC, LL_GPIO_PIN_6);  // УИ5
	        	  LL_GPIO_ResetOutputPin(GPIOC, LL_GPIO_PIN_7);  // УИ2
		            DBMain.b96.PulseStage1 = 0;
		            DBMain.b96.PulseStage2 = 0;
		            DBMain.b96.PulseStage3 = 0;
		            DBMain.b96.PulseStage4 = 0;
		            DBMain.b96.PulseStage5 = 0;
		            DBMain.b96.PulseStage6 = 0;

	        	break;
	}
	// Здесь нужно запустить таймер или системный тик для снятия импульса через 500-1000 мкс
	//Start_Pulse_Width_Timer(500);
}

/**
  * @brief Включение СИФУ: разрешение прерываний сравнения
  */
void Sifu_EnableAll(void)
{
  // Очищаем флаги прерываний перед включением, чтобы не было "старых" импульсов
  LL_TIM_ClearFlag_CC2(TIM3);
  LL_TIM_ClearFlag_CC3(TIM3);
  LL_TIM_ClearFlag_CC2(TIM4);
  LL_TIM_ClearFlag_CC4(TIM4);
  LL_TIM_ClearFlag_CC1(TIM8);
  LL_TIM_ClearFlag_CC2(TIM8);

  // Разрешаем прерывания
  LL_TIM_EnableIT_CC2(TIM3);
  LL_TIM_EnableIT_CC3(TIM3);
  LL_TIM_EnableIT_CC2(TIM4);
  LL_TIM_EnableIT_CC4(TIM4);
  LL_TIM_EnableIT_CC1(TIM8);
  LL_TIM_EnableIT_CC2(TIM8);

}

void Sifu_EnableYI1(void){
	  // Очищаем флаги прерываний перед включением, чтобы не было "старых" импульсов
	  LL_TIM_ClearFlag_CC2(TIM3);
//	  LL_TIM_ClearFlag_CC3(TIM3);
//	  LL_TIM_ClearFlag_CC2(TIM4);
//	  LL_TIM_ClearFlag_CC4(TIM4);
//	  LL_TIM_ClearFlag_CC1(TIM8);
//	  LL_TIM_ClearFlag_CC2(TIM8);

	  // Разрешаем прерывания
	  LL_TIM_EnableIT_CC2(TIM3);  // УИ1

	  // Запрещаем прерывания
//	  LL_TIM_DisableIT_CC3(TIM3); // УИ4
//	  LL_TIM_DisableIT_CC2(TIM4); // УИ3
//	  LL_TIM_DisableIT_CC4(TIM4); // УИ6
//	  LL_TIM_DisableIT_CC1(TIM8); // УИ5
//	  LL_TIM_DisableIT_CC2(TIM8); // УИ2
}

void Sifu_EnableYI2(void){
	  // Очищаем флаги прерываний перед включением, чтобы не было "старых" импульсов
//	  LL_TIM_ClearFlag_CC2(TIM3);
//	  LL_TIM_ClearFlag_CC3(TIM3);
//	  LL_TIM_ClearFlag_CC2(TIM4);
//	  LL_TIM_ClearFlag_CC4(TIM4);
//	  LL_TIM_ClearFlag_CC1(TIM8);
	  LL_TIM_ClearFlag_CC2(TIM8);

	  // Запрещаем прерывания
//	  LL_TIM_DisableIT_CC2(TIM3); // УИ1
//	  LL_TIM_DisableIT_CC3(TIM3); // УИ4
//	  LL_TIM_DisableIT_CC2(TIM4); // УИ3
//	  LL_TIM_DisableIT_CC4(TIM4); // УИ6
//	  LL_TIM_DisableIT_CC1(TIM8); // УИ5
	  // Разрешаем прерывание
	  LL_TIM_EnableIT_CC2(TIM8); // УИ2
}

void Sifu_EnableYI3(void){
	  // Очищаем флаги прерываний перед включением, чтобы не было "старых" импульсов
//	  LL_TIM_ClearFlag_CC2(TIM3);
//	  LL_TIM_ClearFlag_CC3(TIM3);
	  LL_TIM_ClearFlag_CC2(TIM4);
//	  LL_TIM_ClearFlag_CC4(TIM4);
//	  LL_TIM_ClearFlag_CC1(TIM8);
//	  LL_TIM_ClearFlag_CC2(TIM8);

	  // Запрещаем прерывания
//	  LL_TIM_DisableIT_CC2(TIM3); // УИ1
//	  LL_TIM_DisableIT_CC3(TIM3); // УИ4
	  // Разрешаем прерывание
	  LL_TIM_EnableIT_CC2(TIM4); // УИ3
	  // Запрещаем прерывание
//	  LL_TIM_DisableIT_CC4(TIM4); // УИ6
//	  LL_TIM_DisableIT_CC1(TIM8); // УИ5
//	  LL_TIM_DisableIT_CC2(TIM8); // УИ2
}

void Sifu_EnableYI4(void){
	  // Очищаем флаги прерываний перед включением, чтобы не было "старых" импульсов
//	  LL_TIM_ClearFlag_CC2(TIM3);
	  LL_TIM_ClearFlag_CC3(TIM3);
//	  LL_TIM_ClearFlag_CC2(TIM4);
//	  LL_TIM_ClearFlag_CC4(TIM4);
//	  LL_TIM_ClearFlag_CC1(TIM8);
//	  LL_TIM_ClearFlag_CC2(TIM8);

	  // Запрещаем прерывания
//	  LL_TIM_DisableIT_CC2(TIM3); // УИ1
	  // Разрешаем прерывание
	  LL_TIM_EnableIT_CC3(TIM3); // УИ4
	  // Запрещаем прерывание
//	  LL_TIM_DisableIT_CC2(TIM4); // УИ3
//	  LL_TIM_DisableIT_CC4(TIM4); // УИ6
//	  LL_TIM_DisableIT_CC1(TIM8); // УИ5
//	  LL_TIM_DisableIT_CC2(TIM8); // УИ2
}

void Sifu_EnableYI5(void){
	  // Очищаем флаги прерываний перед включением, чтобы не было "старых" импульсов
//	  LL_TIM_ClearFlag_CC2(TIM3);
//	  LL_TIM_ClearFlag_CC3(TIM3);
//	  LL_TIM_ClearFlag_CC2(TIM4);
//	  LL_TIM_ClearFlag_CC4(TIM4);
	  LL_TIM_ClearFlag_CC1(TIM8);
//	  LL_TIM_ClearFlag_CC2(TIM8);

	  // Запрещаем прерывания
//	  LL_TIM_DisableIT_CC2(TIM3); // УИ1
//	  LL_TIM_DisableIT_CC3(TIM3); // УИ4
//	  LL_TIM_DisableIT_CC2(TIM4); // УИ3
//	  LL_TIM_DisableIT_CC4(TIM4); // УИ6
	  // Разрешаем прерывание
	  LL_TIM_EnableIT_CC1(TIM8); // УИ5
	  // Запрещаем прерывание
//	  LL_TIM_DisableIT_CC2(TIM8); // УИ2
}

void Sifu_EnableYI6(void){
	  // Очищаем флаги прерываний перед включением, чтобы не было "старых" импульсов
//	  LL_TIM_ClearFlag_CC2(TIM3);
//	  LL_TIM_ClearFlag_CC3(TIM3);
//	  LL_TIM_ClearFlag_CC2(TIM4);
	  LL_TIM_ClearFlag_CC4(TIM4);
//	  LL_TIM_ClearFlag_CC1(TIM8);
//	  LL_TIM_ClearFlag_CC2(TIM8);

	  // Запрещаем прерывания
//	  LL_TIM_DisableIT_CC2(TIM3); // УИ1
//	  LL_TIM_DisableIT_CC3(TIM3); // УИ4
//	  LL_TIM_DisableIT_CC2(TIM4); // УИ3
	  // Разрешаем прерывание
	  LL_TIM_EnableIT_CC4(TIM4); // УИ6
	  // Запрещаем прерывание
//	  LL_TIM_DisableIT_CC1(TIM8); // УИ5
//	  LL_TIM_DisableIT_CC2(TIM8); // УИ2
}

/**
  * @brief Мгновенная остановка СИФУ: запрет прерываний и сброс всех УИ
  */
void Sifu_DisableAll(void)
{
  // 1. Запрещаем прерывания сравнения во всех таймерах (программная блокировка)
  LL_TIM_DisableIT_CC2(TIM3); // УИ1
  LL_TIM_DisableIT_CC3(TIM3); // УИ4

  LL_TIM_DisableIT_CC2(TIM4); // УИ3
  LL_TIM_DisableIT_CC4(TIM4); // УИ6

  LL_TIM_DisableIT_CC1(TIM8); // УИ5
  LL_TIM_DisableIT_CC2(TIM8); // УИ2

  // 2. Принудительный сброс физических уровней на всех пинах УИ в 0 (аппаратная блокировка)
  // Группа Фазы А
  LL_GPIO_ResetOutputPin(GPIOA, LL_GPIO_PIN_7); // УИ1
  LL_GPIO_ResetOutputPin(GPIOB, LL_GPIO_PIN_0); // УИ4

  // Группа Фазы B
  LL_GPIO_ResetOutputPin(GPIOD, LL_GPIO_PIN_13); // УИ3
  LL_GPIO_ResetOutputPin(GPIOD, LL_GPIO_PIN_15); // УИ6

  // Группа Фазы C
  LL_GPIO_ResetOutputPin(GPIOC, LL_GPIO_PIN_6);  // УИ5
  LL_GPIO_ResetOutputPin(GPIOC, LL_GPIO_PIN_7);  // УИ2

}

void Sifu_DisableYI14(void){
	  // 1. Запрещаем прерывания сравнения во всех таймерах (программная блокировка)
	  LL_TIM_DisableIT_CC2(TIM3); // УИ1
	  LL_TIM_DisableIT_CC3(TIM3); // УИ4
	  // 2. Принудительный сброс физических уровней на всех пинах УИ в 0 (аппаратная блокировка)
	  // Группа Фазы А
	  LL_GPIO_ResetOutputPin(GPIOA, LL_GPIO_PIN_7); // УИ1
	  LL_GPIO_ResetOutputPin(GPIOB, LL_GPIO_PIN_0); // УИ4
}

void Sifu_DisableYI36(void){
	  // 1. Запрещаем прерывания сравнения во всех таймерах (программная блокировка)
	  LL_TIM_DisableIT_CC2(TIM4); // УИ3
	  LL_TIM_DisableIT_CC4(TIM4); // УИ6
	  // 2. Принудительный сброс физических уровней на всех пинах УИ в 0 (аппаратная блокировка)
	  // Группа Фазы B
	  LL_GPIO_ResetOutputPin(GPIOD, LL_GPIO_PIN_13); // УИ3
	  LL_GPIO_ResetOutputPin(GPIOD, LL_GPIO_PIN_15); // УИ6
}

void Sifu_DisableYI52(void){
	  // 1. Запрещаем прерывания сравнения во всех таймерах (программная блокировка)
	  LL_TIM_DisableIT_CC1(TIM8); // УИ5
	  LL_TIM_DisableIT_CC2(TIM8); // УИ2
	  // 2. Принудительный сброс физических уровней на всех пинах УИ в 0 (аппаратная блокировка)
	  // Группа Фазы C
	  LL_GPIO_ResetOutputPin(GPIOC, LL_GPIO_PIN_6);  // УИ5
	  LL_GPIO_ResetOutputPin(GPIOC, LL_GPIO_PIN_7);  // УИ2
}

void Sifu_DisableYI1(void){
	LL_TIM_ClearFlag_CC2(TIM3);
	//Запрещаем прерывания сравнения
	LL_TIM_DisableIT_CC2(TIM3); // УИ1
}

void Sifu_DisableYI2(void){
	LL_TIM_ClearFlag_CC2(TIM8);
	//Запрещаем прерывания сравнения
	LL_TIM_DisableIT_CC2(TIM8); // УИ2
}

void Sifu_DisableYI3(void){
	LL_TIM_ClearFlag_CC2(TIM4);
	//Запрещаем прерывания сравнения
	LL_TIM_DisableIT_CC2(TIM4); // УИ3
}

void Sifu_DisableYI4(void){
	LL_TIM_ClearFlag_CC3(TIM3);
	//Запрещаем прерывания сравнения
	LL_TIM_DisableIT_CC3(TIM3); // УИ4
}

void Sifu_DisableYI5(void){
	LL_TIM_ClearFlag_CC1(TIM8);
	//Запрещаем прерывания сравнения
	LL_TIM_DisableIT_CC1(TIM8); // УИ5
}

void Sifu_DisableYI6(void){
	LL_TIM_ClearFlag_CC4(TIM4);
	//Запрещаем прерывания сравнения
	LL_TIM_DisableIT_CC4(TIM4); // УИ6
}

void CalculateNextImpuls(uint8_t pulse_num){
	float ticks_per_degree;
	uint32_t alpha_ticks;
	uint32_t half_period;
	switch(pulse_num) {
	        case 1:
	             ticks_per_degree = PhaseA.PeriodFiltered / 360.0f;
	             alpha_ticks = (uint32_t)(ticks_per_degree * DBMain.f50.Alfa_ref);
	             //half_period = (uint32_t)(PhaseA.PeriodFiltered / 2.0f);

	             LL_TIM_OC_SetCompareCH2(TIM3, PhaseA.T_zero + alpha_ticks);              // УИ1 (PA7)
	            // LL_TIM_OC_SetCompareCH3(TIM3, PhaseA.T_zero + alpha_ticks + half_period); // УИ4 (PB0)
	             Sifu_EnableYI(pulse_num);
	            break;
	        case 2:
	            ticks_per_degree = PhaseC.PeriodFiltered / 360.0f;
	            alpha_ticks = (uint32_t)(ticks_per_degree * DBMain.f50.Alfa_ref);
	            half_period = (uint32_t)(PhaseC.PeriodFiltered / 2.0f);

	            //LL_TIM_OC_SetCompareCH1(TIM8, PhaseC.T_zero + alpha_ticks);              // УИ5 (PC6)
	            LL_TIM_OC_SetCompareCH2(TIM8, PhaseC.T_zero + alpha_ticks + half_period); // УИ2 (PC7)
	            Sifu_EnableYI(pulse_num);
	            break;
	        case 3:
	            ticks_per_degree = PhaseB.PeriodFiltered / 360.0f;
	            alpha_ticks = (uint32_t)(ticks_per_degree * DBMain.f50.Alfa_ref);
	            //half_period = (uint32_t)(PhaseB.PeriodFiltered / 2.0f);

	            LL_TIM_OC_SetCompareCH2(TIM4, PhaseB.T_zero + alpha_ticks);              // УИ3 (PD13)
	            //LL_TIM_OC_SetCompareCH4(TIM4, PhaseB.T_zero + alpha_ticks + half_period); // УИ6 (PD15)
	            Sifu_EnableYI(pulse_num);
	            break;
	        case 4:
	            ticks_per_degree = PhaseA.PeriodFiltered / 360.0f;
	             alpha_ticks = (uint32_t)(ticks_per_degree * DBMain.f50.Alfa_ref);
	             half_period = (uint32_t)(PhaseA.PeriodFiltered / 2.0f);

	            // LL_TIM_OC_SetCompareCH2(TIM3, PhaseA.T_zero + alpha_ticks);              // УИ1 (PA7)
	             LL_TIM_OC_SetCompareCH3(TIM3, PhaseA.T_zero + alpha_ticks + half_period); // УИ4 (PB0)
	             Sifu_EnableYI(pulse_num);
	            break;
	        case 5:
	            ticks_per_degree = PhaseC.PeriodFiltered / 360.0f;
	            alpha_ticks = (uint32_t)(ticks_per_degree * DBMain.f50.Alfa_ref);
	           // uint32_t half_period = (uint32_t)(PhaseC.PeriodFiltered / 2.0f);

	            LL_TIM_OC_SetCompareCH1(TIM8, PhaseC.T_zero + alpha_ticks);              // УИ5 (PC6)
	           // LL_TIM_OC_SetCompareCH2(TIM8, PhaseC.T_zero + alpha_ticks + half_period); // УИ2 (PC7)
	            Sifu_EnableYI(pulse_num);
	            break;
	        case 6:
	            ticks_per_degree = PhaseB.PeriodFiltered / 360.0f;
	            alpha_ticks = (uint32_t)(ticks_per_degree * DBMain.f50.Alfa_ref);
	            half_period = (uint32_t)(PhaseB.PeriodFiltered / 2.0f);

	            //LL_TIM_OC_SetCompareCH2(TIM4, PhaseB.T_zero + alpha_ticks);              // УИ3 (PD13)
	            LL_TIM_OC_SetCompareCH4(TIM4, PhaseB.T_zero + alpha_ticks + half_period); // УИ6 (PD15)
	            Sifu_EnableYI(pulse_num);
	            break;
	        default:
	        	Sifu_DisableAll();
	            if (DBMain.b64.EnableSifu == 0){
	                float ticks_per_degreeA = PhaseA.PeriodFiltered / 360.0f;
	                uint32_t alpha_ticksA = (uint32_t)(ticks_per_degreeA * DBMain.f50.Alfa_ref);
	                uint32_t half_periodA = (uint32_t)(PhaseA.PeriodFiltered / 2.0f);

	                LL_TIM_OC_SetCompareCH2(TIM3, PhaseA.T_zero + alpha_ticksA);              // УИ1 (PA7)
	                LL_TIM_OC_SetCompareCH3(TIM3, PhaseA.T_zero + alpha_ticksA + half_periodA); // УИ4 (PB0)

	                float ticks_per_degreeB = PhaseB.PeriodFiltered / 360.0f;
	                uint32_t alpha_ticksB = (uint32_t)(ticks_per_degreeB * DBMain.f50.Alfa_ref);
	                uint32_t half_periodB = (uint32_t)(PhaseB.PeriodFiltered / 2.0f);

	                LL_TIM_OC_SetCompareCH2(TIM4, PhaseB.T_zero + alpha_ticksB);              // УИ3 (PD13)
	                LL_TIM_OC_SetCompareCH4(TIM4, PhaseB.T_zero + alpha_ticksB + half_periodB); // УИ6 (PD15)

	                float ticks_per_degreeC = PhaseC.PeriodFiltered / 360.0f;
	                uint32_t alpha_ticksC = (uint32_t)(ticks_per_degreeC * DBMain.f50.Alfa_ref);
	                uint32_t half_periodC = (uint32_t)(PhaseC.PeriodFiltered / 2.0f);

	                LL_TIM_OC_SetCompareCH1(TIM8, PhaseC.T_zero + alpha_ticksC);              // УИ5 (PC6)
	                LL_TIM_OC_SetCompareCH2(TIM8, PhaseC.T_zero + alpha_ticksC + half_periodC); // УИ2 (PC7)
	            }

	        	break;
	}

}

void Sifu_EnableYI(uint8_t pulse_num){
	switch(pulse_num) {
		        case 1:
		        	Sifu_EnableYI1();
		            break;
		        case 2:
		        	Sifu_EnableYI2();
		            break;
		        case 3:
		        	Sifu_EnableYI3();
		            break;
		        case 4:
		        	Sifu_EnableYI4();
		            break;
		        case 5:
		        	Sifu_EnableYI5();
		            break;
		        case 6:
		        	Sifu_EnableYI6();
		            break;
		        default:
		        	Sifu_DisableAll();
		        	break;
		}

}

void Sifu_StartPulseWidth(void){
	// 2. ЗАДАЕМ ДЛИТЕЛЬНОСТЬ ИЗ КОДА
	        // Допустим, параметр P50_4 хранит длительность в мкс (например, 500)
	        uint32_t width_us = (uint32_t)DBParameters.f100.P50_4;

	        // Защита: длительность не может быть 0 или слишком большой (ARR - 16 бит)
	        if (width_us < 10) width_us = 10;
	        if (width_us > 65000) width_us = 65000;

	        // Записываем новое значение в регистр автоперегрузки
	        LL_TIM_SetAutoReload(TIM7, width_us - 1);

	        // 3. ЗАПУСК ТАЙМЕРА
	        LL_TIM_SetCounter(TIM7, 0); // Сброс счетчика в 0
	        LL_TIM_EnableCounter(TIM7); // Старт отсчета
}

void Sifu_DisableYI(uint8_t pulse_num){
	switch(pulse_num) {
		        case 1:
		        	Sifu_DisableYI1();
		            break;
		        case 2:
		        	Sifu_DisableYI2();
		            break;
		        case 3:
		        	Sifu_DisableYI3();
		            break;
		        case 4:
		        	Sifu_DisableYI4();
		            break;
		        case 5:
		        	Sifu_DisableYI5();
		            break;
		        case 6:
		        	Sifu_DisableYI6();
		            break;
		        default:
		        	Sifu_DisableAll();
		        	break;
		}

}
