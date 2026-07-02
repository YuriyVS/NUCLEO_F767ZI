#ifndef AI_NORMALISATION_H
#define AI_NORMALISATION_H

#include <stdint.h>
#include <stdbool.h>

typedef struct {
    // 1. Накопители для интеграции (15 кГц)
    volatile uint32_t RawSum;
    volatile uint32_t SamplesCount;

    // 2. Результаты расчетов
    float MeanRaw;      // Среднее значение АЦП (0...4095)
    float PhysicalVal;  // Значение в физ. единицах (V, A)
    float PhysicalPct;  // Значение в процентах (%)

    // 3. RC Фильтр
    float FilteredVal;  // Отфильтрованное значение (%)
    float PreviousVal;  // Состояние фильтра
    float Kfilter;      // Коэффициент фильтрации
} AI_Channel_t;

// Объявляем массив каналов (6 каналов: Useti, Iakb, + 4 резерва)
extern AI_Channel_t AI_Channels[6];
extern uint32_t raw_sum[6];
extern uint32_t samples_count;

#define Useti_AI AI_Channels[0].MeanRaw
#define Iakb_AI AI_Channels[1].MeanRaw
#define Rezerv1_AI AI_Channels[2].MeanRaw
#define Rezerv2_AI AI_Channels[3].MeanRaw
#define Rezerv3_AI AI_Channels[4].MeanRaw
#define Rezerv4_AI AI_Channels[5].MeanRaw

// Прототипы функций
extern void AI_Accumulate(uint16_t *adc_data);
extern void Process_AI(AI_Channel_t *ch, float P_Gain, float P_Offset, float P_FullScale, float P_TimeConstant, float dt);

#endif
