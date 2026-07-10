#ifndef BLOCK_SIFU_H
#define BLOCK_SIFU_H

#include <stdint.h>
#include <stdbool.h>
#include "DB_Parameters.h"
#include "DB_Main.h"
#include "DB_Constants.h"

extern void Generate_Thyristor_Pulse(uint8_t pulse_num, uint32_t current_ccr);
extern void Sifu_EnableAll(void);
extern void Sifu_EnableYI(uint8_t pulse_num);
extern void Sifu_EnableYI1(void);
extern void Sifu_EnableYI2(void);
extern void Sifu_EnableYI3(void);
extern void Sifu_EnableYI4(void);
extern void Sifu_EnableYI5(void);
extern void Sifu_EnableYI6(void);
extern void Sifu_DisableAll(void);
extern void Sifu_DisableYI14(void);
extern void Sifu_DisableYI36(void);
extern void Sifu_DisableYI52(void);
extern void Sifu_DisableYI1(void);
extern void Sifu_DisableYI2(void);
extern void Sifu_DisableYI3(void);
extern void Sifu_DisableYI4(void);
extern void Sifu_DisableYI5(void);
extern void Sifu_DisableYI6(void);
extern void Sifu_DisableYI(uint8_t pulse_num);
extern void CalculateNextImpuls(uint8_t pulse_num);
extern void Sifu_StartPulseWidth(void);

#endif
