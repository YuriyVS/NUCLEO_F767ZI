#include "main.h"
#include "AI_Normalisation.h"
#include "DB_Parameters.h"
#include "DB_Constants.h"
#include "DB_Main.h"
#include <stdint.h>
#include <stdbool.h>

const float measuring_range_ai = 4095.0f; // 12-битный АЦП
const float dt_ai = 3.3f; // Расчет каждые 3.3 мс

AI_Channel_t AI_Channels[6] = {0};
uint32_t __attribute__((section(".dtcm_data"))) raw_sum[6] = {0};
uint32_t __attribute__((section(".dtcm_data"))) samples_count = 0;

/**
  * @brief Накопление данных (вызывать в DMA ISR)
  */
void __attribute__((section(".itcm_text"))) AI_Accumulate(uint16_t *adc_data) {
	__disable_irq();
    for (int i = 0; i < 6; i++) {
    	raw_sum[i] += adc_data[i];
        //AI_Channels[i].RawSum += adc_data[i];
        //AI_Channels[i].SamplesCount++;
    }
    samples_count++;
    //AI_Channels[0].SamplesCount++; // Общий счетчик для всех каналов
    __enable_irq();
}

/**
  * @brief Обработка, масштабирование и RC-фильтрация
  * @param dt Период вызова функции в секундах (например, 0.001f для 1мс)
  */
void Process_AI(AI_Channel_t *ch, float P_Gain, float P_Offset, float P_FullScale, float P_TimeConstant, float dt)
{
    // 1. Расчет среднего значения (Integrate and Dump)
    if (ch->SamplesCount > 0) {
        ch->MeanRaw = (float)ch->RawSum / (float)ch->SamplesCount;
        // Сброс накопителей для следующего цикла
        ch->RawSum = 0;
        //if (ch == &AI_Channels[5])
        ch->SamplesCount = 0; // Сброс  счетчика в конце цикла
    }

    // 2. Масштабирование по формулам из задания
    // Физическое значение: Val = Gain * AI_Mean + Offset
    ch->PhysicalVal = (P_Gain * ch->MeanRaw) / measuring_range_ai + P_Offset;

    // 3. RC Фильтр (аналогично AO)
    // Коэффициент K = dt / (T + dt)
    if (P_TimeConstant > 0.0f) {
        ch->Kfilter = dt / (P_TimeConstant + dt);
    } else {
        ch->Kfilter = 1.0f; // Фильтр выключен
    }

    // Формула: Y = K*X + (1-K)*Y_prev
    ch->FilteredVal = (ch->Kfilter * ch->PhysicalVal) + ((1.0f - ch->Kfilter) * ch->PreviousVal);
    ch->PreviousVal = ch->FilteredVal;

    // Значение в %: Pct = Val * 100 / FullScale
    if (P_FullScale != 0) {
           ch->PhysicalPct = (ch->FilteredVal * 100.0f) / P_FullScale;
    }

}

void Process_AI_All(void){
    __disable_irq();
    for(int i=0; i<6; i++) {
  	  AI_Channels[i].RawSum = raw_sum[i];
  	  AI_Channels[i].SamplesCount = samples_count;
  	  raw_sum[i] = 0; // Сбрасываем для нового цикла накопления
    }

    samples_count = 0;
    __enable_irq();

    // 1. Напруга в мережі (Useti)
    Process_AI(&AI_Channels[0], DBParameters.f50.P20_1, DBParameters.f50.P20_2,
                       DBParameters.f50.P20_3, DBParameters.f50.P21_2, dt_ai);
    DBMain.f50.UsetiV = AI_Channels[0].FilteredVal;
    DBMain.f50.Useti = AI_Channels[0].PhysicalPct;

    // 2. Струм AKB (Iakb)
    Process_AI(&AI_Channels[3], DBParameters.f50.P20_4, DBParameters.f50.P20_5,
                       DBParameters.f50.P20_6, DBParameters.f50.P21_5, dt_ai);
    DBMain.f50.IakbA = AI_Channels[3].FilteredVal;
    DBMain.f50.Iakb = AI_Channels[3].PhysicalPct;

    // 3. Резерв 1 (Rezerv1)
    Process_AI(&AI_Channels[4], DBParameters.f50.P20_7, DBParameters.f50.P20_8,
                       DBParameters.f50.P20_9, DBParameters.f50.P21_8, dt_ai);
    DBMain.f50.Rezerv1U = AI_Channels[4].FilteredVal;
    DBMain.f50.Rezerv1 = AI_Channels[4].PhysicalPct;
}
