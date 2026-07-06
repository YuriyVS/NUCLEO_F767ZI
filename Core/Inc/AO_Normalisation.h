#ifndef AO_NORMALISATION_H
#define AO_NORMALISATION_H

#include <stdint.h>
#include <stdbool.h>

typedef struct {
    float target_AO;
	float PreviousAO;     // Предыдущее отфильтрованное значение (хранит состояние фильтра)
	float filtered_AO;
    float Kfilter;        // Рассчитанный коэффициент фильтрации
    uint32_t DAC_Channel; // Канал ЦАП (LL_DAC_CHANNEL_1 или LL_DAC_CHANNEL_2)
} AO_Channel_t;

extern volatile AO_Channel_t AO_GenFreq;
extern volatile AO_Channel_t AO_Iakb;

#define GenFreq_AO AO_GenFreq.filtered_AO
#define Iakb_AO AO_Iakb.filtered_AO

extern const float measuring_range_ao;

extern void Process_AO(volatile AO_Channel_t *ch, float RawValue, float P_Gain, float P_Offset, float P_TimeConstant);

#endif
