#ifndef DO_BLOCK_H
#define DO_BLOCK_H

#include <stdint.h>
#include <stdbool.h>
#include "DB_Parameters.h"
#include "DB_Main.h"

// Структура для 16 выходов (задействованы PG0-PG13)
typedef union {
    uint16_t all;
    struct {
        bool do1  : 1; bool do2  : 1; bool do3  : 1; bool do4  : 1;
        bool do5  : 1; bool do6  : 1; bool do7  : 1; bool do8  : 1;
        bool do9  : 1; bool do10 : 1; bool do11 : 1; bool do12 : 1;
        bool do13 : 1; bool do14 : 1; bool do15 : 1; bool do16 : 1;
    };
} DO_Data_t;

extern volatile DO_Data_t DOutputsGr1; // Логические состояния (из алгоритма)
extern const DO_Data_t DOutputsGr1Default; // Логические состояния (из алгоритма)

#define i380V_Podano_DO DOutputsGr1.do1
#define i380V_Znyato_DO DOutputsGr1.do2
#define Merezha380vFault_DO DOutputsGr1.do3
#define GeneratorFault_DO DOutputsGr1.do4
#define i380V_Vvod_DO DOutputsGr1.do5
#define BlockDoor_DO DOutputsGr1.do6
#define Rezerv1_DO DOutputsGr1.do7
#define Rezerv2_DO DOutputsGr1.do8
#define Rezerv3_DO DOutputsGr1.do9
#define Rezerv4_DO DOutputsGr1.do10
#define Rezerv5_DO DOutputsGr1.do11
#define Rezerv6_DO DOutputsGr1.do12
#define Rezerv7_DO DOutputsGr1.do13
#define Rezerv8_DO DOutputsGr1.do14


extern void DO_XOR(void);
extern void Write_DO_Output(void);


#endif
