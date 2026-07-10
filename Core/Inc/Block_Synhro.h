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
extern void Sync_Process_Phase(PhaseSync_t *phase, uint32_t capture);
extern void Sync_Watchdog_Check(void);
extern void Sync_CheckSequence(void);

extern void MX_SyncTimers_NVIC_Init(void);
extern void MX_SyncTimers_Start(void);


#endif
