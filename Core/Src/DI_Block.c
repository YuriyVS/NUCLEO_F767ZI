#include "main.h"
#include "DI_Block.h"
#include "DB_Parameters.h"

__attribute__((section(".dtcm_data")))
volatile DI_Data_t DInputsGr1, current_pins;
const DI_Data_t DInputsGr1Default ={
		.di1 = 0
};

__attribute__((section(".dtcm_data")))
volatile FreqInput_t FreqChannels[3] = {0};

__attribute__((section(".itcm_text")))
void Read_DI_Input(void) {

	// Чтение группы GPIOF
	DInputsGr1.di1  = LL_GPIO_IsInputPinSet(DI1_GPIO_Port, DI1_Pin);
	DInputsGr1.di2  = LL_GPIO_IsInputPinSet(DI2_GPIO_Port, DI2_Pin);
	DInputsGr1.di3  = LL_GPIO_IsInputPinSet(DI3_GPIO_Port, DI3_Pin);
	DInputsGr1.di4  = LL_GPIO_IsInputPinSet(DI4_GPIO_Port, DI4_Pin);
	DInputsGr1.di5  = LL_GPIO_IsInputPinSet(DI5_GPIO_Port, DI5_Pin);
	DInputsGr1.di6  = LL_GPIO_IsInputPinSet(DI6_GPIO_Port, DI6_Pin);
	DInputsGr1.di7  = LL_GPIO_IsInputPinSet(DI7_GPIO_Port, DI7_Pin);
	DInputsGr1.di8  = LL_GPIO_IsInputPinSet(DI8_GPIO_Port, DI8_Pin);
	DInputsGr1.di9  = LL_GPIO_IsInputPinSet(DI9_GPIO_Port, DI9_Pin);
	DInputsGr1.di10 = LL_GPIO_IsInputPinSet(DI10_GPIO_Port, DI10_Pin);
	DInputsGr1.di11 = LL_GPIO_IsInputPinSet(DI11_GPIO_Port, DI11_Pin);
	DInputsGr1.di12 = LL_GPIO_IsInputPinSet(DI12_GPIO_Port, DI12_Pin);
	DInputsGr1.di13 = LL_GPIO_IsInputPinSet(DI13_GPIO_Port, DI13_Pin);

    // Чтение группы GPIOC
    //DInputsGr1.status.di14 = LL_GPIO_IsInputPinSet(DI14_GPIO_Port, DI14_Pin);
    //DInputsGr1.status.di15 = LL_GPIO_IsInputPinSet(DI15_GPIO_Port, DI15_Pin);
    //DInputsGr1.status.di16 = LL_GPIO_IsInputPinSet(DI16_GPIO_Port, DI16_Pin);
}

uint16_t di_counters[DI_COUNT] = {0};

void Read_DI_Input_Filtered(void) {
    // Считываем состояние всего порта GPIOF (пины 0-12)
    current_pins.all = LL_GPIO_ReadInputPort(GPIOF);
    current_pins.di1  = LL_GPIO_IsInputPinSet(USER_Btn_GPIO_Port, USER_Btn_Pin);

    for (int i = 0; i < DI_COUNT; i++) {
        // 1. Получаем текущее физическое состояние пина
        uint8_t raw_state = (current_pins.all & (1 << i)) ? 1 : 0;

        // 2. Получаем текущее стабильное (отфильтрованное) состояние из структуры
        uint8_t stable_state = (DInputsGr1.all & (1 << i)) ? 1 : 0;

        // 3. Если физика отличается от стабильного состояния
        if (raw_state != stable_state) {
            di_counters[i]++; // Наращиваем индивидуальный счетчик

            // Проверяем достижение индивидуального порога для этого входа
            if (di_counters[i] >= DBParameters.u50.as_array[i]) {
                // Состояние стабильно — обновляем бит в структуре
                if (raw_state) {
                    DInputsGr1.all |= (1 << i);
                } else {
                    DInputsGr1.all &= ~(1 << i);
                }
                di_counters[i] = 0;
            }
        } else {
            // Если физический сигнал вернулся к стабильному — сбрасываем счетчик (помеха)
            di_counters[i] = 0;
        }
    }
}

void DI_XOR(void){
//	for (int i = 0; i < DI_COUNT; i++) {
//		;
//	}
	DBMain.b32.Kn380V_Vtkl_DIxor = Kn380V_Vtkl_DI ^ DBParameters.b32.P11_1;
	DBMain.b32.Kn380V_Otkl_DIxor = Kn380V_Otkl_DI ^ DBParameters.b32.P11_2;
	DBMain.b32.KrishkaVidkrita_DIxor = KrishkaVidkrita_DI ^ DBParameters.b32.P11_3;
	DBMain.b32.DistanVidkl380V_DIxor = DistanVidkl380V_DI ^ DBParameters.b32.P11_4;
	DBMain.b32.QFVvod_DIxor = QFVvod_DI ^ DBParameters.b32.P11_5;
	DBMain.b32.QFVvodAvarStan_DIxor = QFVvodAvarStan_DI ^ DBParameters.b32.P11_6;
	DBMain.b32.Rezerv1_DIxor = Rezerv1_DI ^ DBParameters.b32.P11_7;
	DBMain.b32.Rezerv2_DIxor = Rezerv2_DI ^ DBParameters.b32.P11_8;
	DBMain.b32.Rezerv3_DIxor = Rezerv3_DI ^ DBParameters.b32.P11_9;
	DBMain.b32.Rezerv4_DIxor = Rezerv4_DI ^ DBParameters.b32.P11_10;
	DBMain.b32.Rezerv5_DIxor = Rezerv5_DI ^ DBParameters.b32.P11_11;
	DBMain.b32.Rezerv6_DIxor = Rezerv6_DI ^ DBParameters.b32.P11_12;
	DBMain.b32.Rezerv7_DIxor = Rezerv7_DI ^ DBParameters.b32.P11_13;

}

void Interrupt_PE012_Init(void){
	// 1. Включаем DWT для измерения микросекунд (циклов процессора)
	  CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
	  DWT->CYCCNT = 0;
	  DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;

	  // 2. Разрешаем прерывания EXTI в NVIC
	  NVIC_SetPriority(EXTI0_IRQn, 2); // Приоритет выше чем у системных задач
	  NVIC_EnableIRQ(EXTI0_IRQn);
	  NVIC_SetPriority(EXTI1_IRQn, 2);
	  NVIC_EnableIRQ(EXTI1_IRQn);
	  NVIC_SetPriority(EXTI2_IRQn, 2);
	  NVIC_EnableIRQ(EXTI2_IRQn);

	  // Вместо EXTI0_IRQn включаем общий канал для пинов 10-15
	  NVIC_SetPriority(EXTI15_10_IRQn, 2);
	  NVIC_EnableIRQ(EXTI15_10_IRQn);
}

void Process_Frequency_Input(uint8_t channel)
{
    uint32_t current_tick = DWT->CYCCNT;
    uint32_t delta;

    // Расчет дельты с учетом переполнения CYCCNT (раз в 40 секунд при 108МГц)
    delta = current_tick - FreqChannels[channel].LastTick;
//    if (current_tick >= FreqChannels[channel].LastTick) {
//        delta = current_tick - FreqChannels[channel].LastTick;
//    } else {
//        delta = (0xFFFFFFFF - FreqChannels[channel].LastTick) + current_tick;
//    }

    // Программный антидребезг (Lockout)
    if (delta > DEBOUNCE_THRESHOLD_CYCLES)
    {
        FreqChannels[channel].DeltaTicks = delta;
        FreqChannels[channel].LastTick = current_tick;
        FreqChannels[channel].Counter++;

        // Расчет частоты: f = F_cpu / delta
        // Можно делать это здесь или в основном цикле для экономии времени прерывания
        //FreqChannels[channel].Frequency = (float)SystemCoreClock / delta;
    }
}

// Функция безопасного чтения и расчета частоты в основном потоке
void Update_Calculated_Frequency(void)
{
    for(uint8_t i = 0; i < 3; i++)
    {
        // Входим в критическую секцию, чтобы прерывание не исказило данные при чтении
        __disable_irq();
        uint32_t local_delta = FreqChannels[i].DeltaTicks;
        __enable_irq(); // Сразу открываем прерывания обратно

        if (local_delta != 0)
        {
            // Тяжелую математику с float делает основной поток, не мешая прерываниям
            FreqChannels[i].Frequency = (float)SystemCoreClock / local_delta;
        }
        else
        {
            FreqChannels[i].Frequency = 0.0f;
        }
    }

    if(FreqChannels[0].Counter>1) DBMain.f50.FreqGenABHz = FreqChannels[0].Frequency;
    if(FreqChannels[1].Counter>1) DBMain.f50.FreqGenBCHz = FreqChannels[1].Frequency;
    if(FreqChannels[2].Counter>1) DBMain.f50.FreqGenCAHz = FreqChannels[2].Frequency;
    DBMain.f50.SpeedCar = (DBMain.f50.FreqGenABHz + DBMain.f50.FreqGenBCHz + DBMain.f50.FreqGenCAHz)/(3.0f * DBParameters.f50.P6_6);
    FreqChannels[0].Counter = 0;
    FreqChannels[1].Counter = 0;
    FreqChannels[2].Counter = 0;
}

void Init_FreqChannels(void)
{
    for (int i = 0; i < 3; i++)
    {
        FreqChannels[i].DeltaTicks = 0;
        FreqChannels[i].LastTick = 0;
        FreqChannels[i].Counter = 0;
        FreqChannels[i].Frequency = 0.0f;
    }
}
