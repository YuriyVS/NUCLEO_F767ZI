#ifndef BLOCK_SYNHRO_H
#define BLOCK_SYNHRO_H

#include <stdint.h>
#include <stdbool.h>
#include "DB_Parameters.h"
#include "DB_Main.h"
#include "DB_Constants.h"

typedef struct {
    uint32_t LastCapture;
    uint32_t LastTick;        // Метка времени (HAL_GetTick())
    uint32_t GlobalCapture;    // Значение из DWT->CYCCNT (для межфазного сдвига)
    float PeriodFiltered;
    uint32_t T_zero;
    uint32_t T_Pulse_width;
    uint32_t WatchdogTimer;
    uint8_t FazaLoss;
} PhaseSync_t;

extern PhaseSync_t PhaseA, PhaseB, PhaseC;
extern uint32_t deltaAB, deltaAC;
extern float angleAB, angleAC, period;
extern uint32_t ccr1_raw, ccr4_raw, current_ccr1, current_ccr4, ccr_raw;
extern uint32_t ccr3_raw, ccr6_raw, current_ccr3, current_ccr6;
extern uint32_t ccr5_raw, ccr2_raw, current_ccr5, current_ccr2;
extern void Sync_Process_Phase(PhaseSync_t *phase, uint32_t capture);
extern void Sync_Watchdog_Check(void);
extern void Sync_CheckSequence(void);

extern void MX_SyncTimers_NVIC_Init(void);
extern void MX_SyncTimers_Start(void);


#endif
