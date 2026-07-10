#include "main.h"
#include "Block_Synhro.h"

PhaseSync_t PhaseA, PhaseB, PhaseC;

/**
  * @brief Настройка приоритетов прерываний для блока синхронизации СИФУ (LL)
  * @note Вынесено отдельно для исключения влияния на другие части системы.
  */
void MX_SyncTimers_NVIC_Init(void)
{
  /* Получаем текущую группировку (обычно PriorityGroup_4, установленная в HAL_Init) */
  uint32_t priorityGroup = NVIC_GetPriorityGrouping();

  /* Устанавливаем ПРИОРИТЕТ 0 (высший).
     Для СИФУ критично, чтобы прерывание захвата не ждало завершения других задач.
  */

  // Фаза А (TIM3) - пин PA6
  NVIC_SetPriority(TIM3_IRQn, NVIC_EncodePriority(priorityGroup, 0, 0));
  NVIC_EnableIRQ(TIM3_IRQn);

  // Фаза B (TIM4) - пин PD14
  NVIC_SetPriority(TIM4_IRQn, NVIC_EncodePriority(priorityGroup, 0, 0));
  NVIC_EnableIRQ(TIM4_IRQn);

  // Фаза C (TIM8) - пин PC9. Используем CC_IRQn (Capture/Compare)
  NVIC_SetPriority(TIM8_CC_IRQn, NVIC_EncodePriority(priorityGroup, 0, 0));
  NVIC_EnableIRQ(TIM8_CC_IRQn);
}

void MX_SyncTimers_Start(void)
{
  /* Включаем прерывания по захвату (Input Capture) для конкретных каналов */

  LL_TIM_EnableIT_CC1(TIM3); // Канал 1 для фазы А (PA6)
  LL_TIM_EnableIT_CC3(TIM4); // Канал 3 для фазы B (PD14)
  LL_TIM_EnableIT_CC4(TIM8); // Канал 4 для фазы C (PC9)

  /* Если вы используете Output Compare для генерации импульсов,
     здесь же можно разрешить прерывания для каналов выдачи,
     но обычно расчет CCR делается внутри обработчика захвата.
  */

  // Запуск таймеров
  LL_TIM_EnableCounter(TIM3);
  LL_TIM_EnableCounter(TIM4);
  LL_TIM_EnableCounter(TIM8);

}

/**
  * @brief Общая логика обработки захвата для любой фазы
  * @param phase: указатель на структуру данных фазы
  * @param capture: текущее значение из регистра CCR (захват)
  */
void Sync_Process_Phase(PhaseSync_t *phase, uint32_t capture)
{
    // 1. Расчет сырого периода (uint32_t корректно считает разницу при переполнении)
    uint32_t raw_period = capture - phase->LastCapture;
    phase->LastCapture = capture;

    // 2. EMA фильтрация периода (согласно требованиям Р50.8)
    // Period_filtered = (New * P50.8) + (Old * (1 - P50.8))
    phase->PeriodFiltered = ((float)raw_period * DBParameters.f100.P50_8) + (phase->PeriodFiltered * (1.0f - DBParameters.f100.P50_8));

    // 3. Расчет виртуального нуля (T_zero) с учетом поправки Р50.5
    // T_phasing = Period_filtered * P50.5 / 360
    float ticks_per_degree = phase->PeriodFiltered / 360.0f;
    uint32_t t_phasing = (uint32_t)(ticks_per_degree * DBParameters.f100.P50_5);
    phase->T_zero = capture + t_phasing;
    phase->T_Pulse_width = (uint32_t)(ticks_per_degree * DBParameters.f100.P50_4);

    // 4. Сброс Watchdog и флага потери фазы
    phase->WatchdogTimer = 0;
    phase->LastTick = HAL_GetTick();
    phase->FazaLoss = 0;
}
/**
  * @brief Проверка наличия синхроимпульсов по всем фазам
  * Вызывается из SysTick_Handler или основного цикла
  */
void Sync_Watchdog_Check(void)
{
    uint32_t currentTick = HAL_GetTick();
    PhaseSync_t *phases[3] = {&PhaseA, &PhaseB, &PhaseC};

    for (int i = 0; i < 3; i++)
    {
        // Если с момента последнего захвата прошло больше P50.9 мс
        if ((currentTick - phases[i]->LastTick) > (uint32_t)DBParameters.f100.P50_9)
        {
            if (phases[i]->FazaLoss == 0) // Если только что потеряли
            {
                phases[i]->FazaLoss = 1;
                // Здесь можно вызвать экстренную блокировку импульсов
                // Stop_All_Thyristors();
            }
        }
    }
}

//проверка чередования фаз
void Sync_CheckSequence(void)
{
    // 1. Проверка наличия всех трех фаз (Watchdog)
    if (PhaseA.FazaLoss || PhaseB.FazaLoss || PhaseC.FazaLoss)
    {
    	DBMain.b64.ABC380ok = 0;
        return;
    }
    /* Нам нужно знать, сколько тактов DWT длится один период.
           Так как частота таймеров и CPU жестко связаны, мы можем пересчитать
           PeriodFiltered (в тиках таймера) в тики DWT.
           K = F_cpu / F_timer (например, 216МГц / 100кГц = 2160)
    */
    float F_cpu = (float)HAL_RCC_GetHCLKFreq();
    float F_tim = (float)(HAL_RCC_GetPCLK1Freq() * 2); // Обычно частота таймеров в 2 раза выше PCLK
    float K = F_cpu / F_tim;

    // 2. Проверка чередования фаз
    // Берем текущий период Фазы А как эталон (360 градусов)
    float period = PhaseA.PeriodFiltered * K;
    if (period < 1.0f) return; // Защита от деления на 0

    /* Рассчитываем разницу во времени между захватами.
       Важно: используем беззнаковую арифметику uint32_t для корректности при переполнении.
    */
    uint32_t deltaAB = PhaseB.GlobalCapture - PhaseA.GlobalCapture;
    uint32_t deltaAC = PhaseC.GlobalCapture - PhaseA.GlobalCapture;

    // Переводим разницу в электрические градусы
    float angleAB = (float)deltaAB * 360.0f / period;
    float angleAC = (float)deltaAC * 360.0f / period;

    /* Нормализация углов (чтобы они всегда были в диапазоне 0..360)
       Это нужно, если захват Фазы А произошел чуть позже захвата B или C в текущем цикле.
    */
    while (angleAB < 0)   angleAB += 360.0f;
    while (angleAB >= 360.0f) angleAB -= 360.0f;
    while (angleAC < 0)   angleAC += 360.0f;
    while (angleAC >= 360.0f) angleAC -= 360.0f;

    /* Проверка условий:
       - Для ABC: угол AB должен быть ~120, AC ~240.
       - Допуск: +/- 20-30 градусов (на случай сильных искажений в сети).
    */

    if ((angleAB > (120.0f - DBParameters.f100.P50_10) && angleAB < (120.0f + DBParameters.f100.P50_10)) &&
        (angleAC > (240.0f - DBParameters.f100.P50_10) && angleAC < (240.0f + DBParameters.f100.P50_10)))
    {
    	DBMain.b64.ABC380ok = 1; // Все условия выполнены
    }
    else
    {
    	DBMain.b64.ABC380ok = 0; // Чередование обратное (ACB) или фазы слиплись
    }
}
