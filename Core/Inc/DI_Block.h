#ifndef DI_BLOCK_H
#define DI_BLOCK_H

#include <stdint.h>
#include <stdbool.h>
#include "DB_Parameters.h"
#include "DB_Main.h"

typedef union {
    uint16_t all;
    struct {
        bool di1  : 1;
        bool di2  : 1;
        bool di3  : 1;
        bool di4  : 1;
        bool di5  : 1;
        bool di6  : 1;
        bool di7  : 1;
        bool di8  : 1;
        bool di9  : 1;
        bool di10 : 1;
        bool di11 : 1;
        bool di12 : 1;
        bool di13 : 1;
        bool di14 : 1;
        bool di15 : 1;
        bool di16 : 1;
    };
} DI_Data_t;

typedef struct {
    uint32_t LastTick;      // Время последнего валидного прерывания
    uint32_t DeltaTicks;    // Разница между импульсами
    float Frequency;        // Рассчитанная частота
    uint32_t Counter;       // Общий счетчик импульсов
} FreqInput_t;

extern volatile DI_Data_t DInputsGr1, current_pins;
extern const DI_Data_t DInputsGr1Default;

extern volatile FreqInput_t FreqChannels[3];
// Порог антидребезга: при 700 Гц период ~1.4 мс. DWT
// Установим "мертвое время" 0.5 мс (половина периода 1кГц), чтобы не терять полезный сигнал.
//При значении SystemCoreClock = 108 000 000 Гц (108 МГц), которое установлено в вашей функции SystemClock_Config,
//результат деления будет именно таким:108,000,000 / 2,000 = 54,000 циклов.
//Временной интервал: 54 000 циклов процессора при частоте 108 МГц соответствуют ровно 500 микросекундам (0.5 мс)
//2000 Гц период 0,5 мсек
#define DEBOUNCE_THRESHOLD_CYCLES (SystemCoreClock / 2000)
extern void Read_DI_Input(void);

#define Kn380V_Vtkl_DI DInputsGr1.di1
#define Kn380V_Otkl_DI DInputsGr1.di2
#define KrishkaVidkrita_DI DInputsGr1.di3
#define DistanVidkl380V_DI DInputsGr1.di4
#define QFVvod_DI DInputsGr1.di5
#define QFVvodAvarStan_DI DInputsGr1.di6
#define Rezerv1_DI DInputsGr1.di7
#define Rezerv2_DI DInputsGr1.di8
#define Rezerv3_DI DInputsGr1.di9
#define Rezerv4_DI DInputsGr1.di10
#define Rezerv5_DI DInputsGr1.di11
#define Rezerv6_DI DInputsGr1.di12
#define Rezerv7_DI DInputsGr1.di13

extern void Interrupt_PE012_Init(void);
extern void Process_Frequency_Input(uint8_t channel);
extern void Update_Calculated_Frequency(void);

#define DI_COUNT 13

// Массив порогов фильтрации в миллисекундах (при вызове раз в 1 мс)
// Здесь можно задать разные значения для каждого входа
//uint32_t di_filter_thresholds[DI_COUNT] = {
//		DBParameters.u50.P10_1,  // DI1:
//		DBParameters.u50.P10_2,  // DI2:
//		DBParameters.u50.P10_3,  // DI3:
//		DBParameters.u50.P10_4, // DI4:
//		DBParameters.u50.P10_5,  // DI5
//		DBParameters.u50.P10_6,  // DI6
//		DBParameters.u50.P10_7,  // DI7
//		DBParameters.u50.P10_8,  // DI8
//		DBParameters.u50.P10_9,  // DI9
//		DBParameters.u50.P10_10,  // DI10
//		DBParameters.u50.P10_11,  // DI11
//		DBParameters.u50.P10_12,  // DI12
//		DBParameters.u50.P10_13  // DI13
//};
// Массив настроек инверсии входов
//bool di_xor_settings[DI_COUNT] = {
//		DBParameters.b32.P11_1,  // DI1:
//		DBParameters.b32.P11_2,  // DI2:
//		DBParameters.b32.P11_3,  // DI3:
//		DBParameters.b32.P11_4, // DI4:
//		DBParameters.b32.P11_5,  // DI5
//		DBParameters.b32.P11_6,  // DI6
//		DBParameters.b32.P11_7,  // DI7
//		DBParameters.b32.P11_8,  // DI8
//		DBParameters.b32.P11_9,  // DI9
//		DBParameters.b32.P11_10,  // DI10
//		DBParameters.b32.P11_11,  // DI11
//		DBParameters.b32.P11_12,  // DI12
//		DBParameters.b32.P11_13  // DI13
//};

// Счетчики текущего состояния для каждого входа
extern uint16_t di_counters[DI_COUNT];

extern void Read_DI_Input_Filtered(void);
extern void DI_XOR(void);
extern void Init_FreqChannels(void);

#endif
