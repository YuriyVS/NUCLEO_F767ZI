#include "main.h"
#include "AO_Normalisation.h"
#include "DB_Parameters.h"
#include "DB_Constants.h"
#include <stdint.h>
#include <stdbool.h>

const float measuring_range_ao = 4095.0f; // 12-битный ЦАП

volatile AO_Channel_t AO_GenFreq = {.target_AO = 0, .PreviousAO = 0,  .filtered_AO = 0, .DAC_Channel = LL_DAC_CHANNEL_1};
volatile AO_Channel_t AO_Iakb = {.target_AO = 0, .PreviousAO = 0,  .filtered_AO = 0, .DAC_Channel = LL_DAC_CHANNEL_2 };

void Process_AO(volatile AO_Channel_t *ch, float RawValue, float P_Gain, float P_Offset, float P_TimeConstant) {
    // 1. Масштабирование (Нормализация)
    // Формула: AO = Gain * Raw * 4095 / 100 + Offset
	ch->target_AO = (P_Gain * RawValue * measuring_range_ao * 0.0001f) + P_Offset;

    // 2. Расчет коэффициента фильтра (если изменилась постоянная времени)
    if (P_TimeConstant > 0.0f) {
        ch->Kfilter = DBConstants.f50.TaktFilterAO / (P_TimeConstant + DBConstants.f50.TaktFilterAO);
    } else {
        ch->Kfilter = 1.0f; // Фильтр выключен
    }

    // 3. Фильтрация
    // Формула: Y = K*X + (1-K)*Y_prev
    ch->filtered_AO = (ch->Kfilter * ch->target_AO) + ((1.0f - ch->Kfilter) * ch->PreviousAO);
    ch->PreviousAO = ch->filtered_AO;

    // 4. Ограничение, чтобы не выйти за пределы 0..4095
    if (ch->filtered_AO > measuring_range_ao) ch->filtered_AO = measuring_range_ao;
    if (ch->filtered_AO < 0.0f) ch->filtered_AO = 0.0f;

    // 5. Запись в ЦАП
    uint32_t dac_val = (uint32_t)ch->filtered_AO;
    LL_DAC_ConvertData12RightAligned(DAC1, ch->DAC_Channel, dac_val);
    LL_DAC_TrigSWConversion(DAC1, ch->DAC_Channel);
}
