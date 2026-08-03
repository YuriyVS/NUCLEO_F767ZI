/**
  * @file    Block_RTC.h
  * @brief   Драйвер часов реального времени DS3231 для STM32F745/767 (I2C1 + DMA + EXTI3)
  * @hw      SCL: PB8, SDA: PB9, INT/SQW: PE3 (EXTI3)
  */

#ifndef BLOCK_RTC_H
#define BLOCK_RTC_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f7xx_hal.h"
#include <stdint.h>
#include <stdbool.h>
#include "DB_Parameters.h"
#include "DB_Main.h"
#include "DB_Constants.h"

/* -------------------------------------------------------------------------- */
/*                            Аппаратный адрес DS3231                         */
/* -------------------------------------------------------------------------- */
#define DS3231_I2C_ADDR             (0x68 << 1) // 0xD0 (Write), 0xD1 (Read)

/* -------------------------------------------------------------------------- */
/*                           Карта регистров DS3231                           */
/* -------------------------------------------------------------------------- */
#define DS3231_REG_SECONDS          0x00
#define DS3231_REG_MINUTES          0x01
#define DS3231_REG_HOURS            0x02
#define DS3231_REG_DAY              0x03
#define DS3231_REG_DATE             0x04
#define DS3231_REG_MONTH            0x05
#define DS3231_REG_YEAR             0x06
#define DS3231_REG_CONTROL          0x0E
#define DS3231_REG_STATUS           0x0F
#define DS3231_REG_AGING            0x10
#define DS3231_REG_TEMP_MSB         0x11
#define DS3231_REG_TEMP_LSB         0x12

/* -------------------------------------------------------------------------- */
/*                             Маски регистра Control                         */
/* -------------------------------------------------------------------------- */
#define DS3231_CTRL_EOSC            (1 << 7)
#define DS3231_CTRL_BBSQW           (1 << 6)
#define DS3231_CTRL_CONV            (1 << 5)
#define DS3231_CTRL_RS2             (1 << 4)
#define DS3231_CTRL_RS1             (1 << 3)
#define DS3231_CTRL_INTCN           (1 << 2)
#define DS3231_CTRL_A2IE            (1 << 1)
#define DS3231_CTRL_A1IE            (1 << 0)

/* -------------------------------------------------------------------------- */
/*                             Маски регистра Status                          */
/* -------------------------------------------------------------------------- */
#define DS3231_STAT_OSF             (1 << 7)
#define DS3231_STAT_EN32KHZ         (1 << 3)
#define DS3231_STAT_BSY             (1 << 2)
#define DS3231_STAT_A2F             (1 << 1)
#define DS3231_STAT_A1F             (1 << 0)

/* -------------------------------------------------------------------------- */
/*                            Структура даты и времени                        */
/* -------------------------------------------------------------------------- */
typedef struct {
    uint8_t seconds;     // 0..59
    uint8_t minutes;     // 0..59
    uint8_t hours;       // 0..23 (24h format)
    uint8_t day_of_week; // 1..7 (1 = Пн)
    uint8_t day;         // 1..31
    uint8_t month;       // 1..12
    uint8_t year;        // 0..99 (например, 26 = 2026)
    float   temperature; // В градусах Цельсия
    uint8_t time_valid;  // 1 = Время достоверно, 0 = Был сброс питания (OSF=1)
} RTC_DateTime_t;

/* -------------------------------------------------------------------------- */
/*                         Внешние глобальные переменные                      */
/* -------------------------------------------------------------------------- */
extern I2C_HandleTypeDef hi2c1;
extern DMA_HandleTypeDef hdma_i2c1_rx;
extern DMA_HandleTypeDef hdma_i2c1_tx;

extern volatile uint8_t g_rtc_1hz_flag;     // Флаг секундного импульса с PE3 (EXTI3)
extern volatile uint8_t g_rtc_rx_cplt_flag; // Флаг завершения чтения по DMA

/* -------------------------------------------------------------------------- */
/*                            Прототипы функций                               */
/* -------------------------------------------------------------------------- */

HAL_StatusTypeDef Block_RTC_Init(void);
void              Block_RTC_BusRecover(void);

HAL_StatusTypeDef Block_RTC_SetDateTime(const RTC_DateTime_t *dt);
HAL_StatusTypeDef Block_RTC_ReadDateTime_Blocking(RTC_DateTime_t *dt);
HAL_StatusTypeDef Block_RTC_ReadDateTime_DMA(void);
void              Block_RTC_ParseDmaBuffer(RTC_DateTime_t *dt);

HAL_StatusTypeDef Block_RTC_ReadTemperature(float *temp);
HAL_StatusTypeDef Block_RTC_Set1HzOutput(uint8_t enable);
HAL_StatusTypeDef Block_RTC_ClearOSF(void);

#ifdef __cplusplus
}
#endif

#endif /* BLOCK_RTC_H */
