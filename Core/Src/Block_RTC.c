/**
  * @file    Block_RTC.c
  * @brief   Реализация драйвера DS3231 для STM32F745
  */
#include "main.h"
#include "Block_RTC.h"
#include "string.h"


/* Handlers периферии */
//I2C_HandleTypeDef hi2c1;
DMA_HandleTypeDef hdma_i2c1_rx;
DMA_HandleTypeDef hdma_i2c1_tx;

/* Флаги прерываний */
volatile uint8_t g_rtc_1hz_flag = 0;
volatile uint8_t g_rtc_rx_cplt_flag = 0;

/*
 * Буферы DMA с ОБЯЗАТЕЛЬНЫМ выравниванием по границе 32 байт
 * для корректной работы D-Cache на ядре Cortex-M7 (STM32F7)!
 */
static uint8_t rtc_rx_dma_buf[32] __attribute__((aligned(32)));
static uint8_t rtc_tx_dma_buf[32] __attribute__((aligned(32)));

/* -------------------------------------------------------------------------- */
/*                        Вспомогательные BCD функции                         */
/* -------------------------------------------------------------------------- */
static inline uint8_t RTC_BcdToDec(uint8_t val) {
    return (uint8_t)(((val >> 4) * 10) + (val & 0x0F));
}

static inline uint8_t RTC_DecToBcd(uint8_t val) {
    return (uint8_t)(((val / 10) << 4) | (val % 10));
}

/* -------------------------------------------------------------------------- */
/*   Процедура аппаратно-программной разблокировки шины I2C1 (Stuck SDA Fix)   */
/* -------------------------------------------------------------------------- */
void Block_RTC_BusRecover(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    __HAL_RCC_GPIOB_CLK_ENABLE();

    /* 1. Настраиваем PB6 (SCL) и PB7 (SDA) как GPIO Output Open-Drain */
    GPIO_InitStruct.Pin = GPIO_PIN_8 | GPIO_PIN_9;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_8 | GPIO_PIN_9, GPIO_PIN_SET);

    /* 2. Если линия SDA зажата в 0 ведомым устройством, тактируем SCL */
    if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_9) == GPIO_PIN_RESET)
    {
        for (uint8_t i = 0; i < 9; i++)
        {
            HAL_GPIO_WritePin(GPIOB, GPIO_PIN_8, GPIO_PIN_RESET);
            for (volatile int d = 0; d < 100; d++);

            HAL_GPIO_WritePin(GPIOB, GPIO_PIN_8, GPIO_PIN_SET);
            for (volatile int d = 0; d < 100; d++);

            if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_9) == GPIO_PIN_SET) {
                break; // Линия SDA освободилась!
            }
        }

        /* 3. Генерируем принудительный STOP */
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_8, GPIO_PIN_SET);
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_9, GPIO_PIN_RESET);
        for (volatile int d = 0; d < 100; d++);
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_9, GPIO_PIN_SET);
        for (volatile int d = 0; d < 100; d++);
    }
}

/* -------------------------------------------------------------------------- */
/*                      Инициализация аппаратных ресурсов                     */
/* -------------------------------------------------------------------------- */
HAL_StatusTypeDef Block_RTC_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    /* ========================================================================= */
    /* 1. Включаем тактование ВСЕХ периферийных блоков (ОБЯЗАТЕЛЬНО ПЕРВЫМ ДЕЛОМ) */
    /* ========================================================================= */
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOE_CLK_ENABLE();
    __HAL_RCC_DMA1_CLK_ENABLE();
    __HAL_RCC_I2C1_CLK_ENABLE();

    /* ========================================================================= */
    /* 2. Аппаратный сброс блока I2C1 + Программная разблокировка шины (PB8/PB9)  */
    /* ========================================================================= */
    __HAL_RCC_I2C1_FORCE_RESET();
    for (volatile int i = 0; i < 100; i++);
    __HAL_RCC_I2C1_RELEASE_RESET();

    Block_RTC_BusRecover();

    /* ========================================================================= */
    /* 3. Конфигурация GPIO для I2C1: PB8 (SCL), PB9 (SDA)                       */
    /* ========================================================================= */
    GPIO_InitStruct.Pin = GPIO_PIN_8 | GPIO_PIN_9;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
    GPIO_InitStruct.Pull = GPIO_NOPULL; // Внешние подтяжки на плате
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF4_I2C1;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    /* ========================================================================= */
    /* 4. Конфигурация GPIO и NVIC для входного сигнала 1 Гц (PE3 / EXTI3)       */
    /* ========================================================================= */
    GPIO_InitStruct.Pin = GPIO_PIN_3;
    GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING; // Спадающий фронт DS3231
    GPIO_InitStruct.Pull = GPIO_NOPULL;          // Внешняя подтяжка R8 10k
    HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

    /* Очищаем ложный флаг прерывания, если он застрял при подаче питания */
    __HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_3);

    HAL_NVIC_SetPriority(EXTI3_IRQn, 6, 0);
    HAL_NVIC_EnableIRQ(EXTI3_IRQn);

    /* ========================================================================= */
    /* 5. Конфигурация DMA1: Stream 0 (I2C1_RX) и Stream 6 (I2C1_TX)             */
    /* ========================================================================= */
    // RX: DMA1 Stream 0 Channel 1
    hdma_i2c1_rx.Instance = DMA1_Stream0;
    hdma_i2c1_rx.Init.Channel = DMA_CHANNEL_1;
    hdma_i2c1_rx.Init.Direction = DMA_PERIPH_TO_MEMORY;
    hdma_i2c1_rx.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_i2c1_rx.Init.MemInc = DMA_MINC_ENABLE;
    hdma_i2c1_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_i2c1_rx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    hdma_i2c1_rx.Init.Mode = DMA_NORMAL;
    hdma_i2c1_rx.Init.Priority = DMA_PRIORITY_LOW;
    hdma_i2c1_rx.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
    HAL_DMA_Init(&hdma_i2c1_rx);
    __HAL_LINKDMA(&hi2c1, hdmarx, hdma_i2c1_rx);

    // TX: DMA1 Stream 6 Channel 1
    hdma_i2c1_tx.Instance = DMA1_Stream6;
    hdma_i2c1_tx.Init.Channel = DMA_CHANNEL_1;
    hdma_i2c1_tx.Init.Direction = DMA_MEMORY_TO_PERIPH;
    hdma_i2c1_tx.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_i2c1_tx.Init.MemInc = DMA_MINC_ENABLE;
    hdma_i2c1_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_i2c1_tx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    hdma_i2c1_tx.Init.Mode = DMA_NORMAL;
    hdma_i2c1_tx.Init.Priority = DMA_PRIORITY_LOW;
    hdma_i2c1_tx.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
    HAL_DMA_Init(&hdma_i2c1_tx);
    __HAL_LINKDMA(&hi2c1, hdmatx, hdma_i2c1_tx);

    /* Настройка NVIC для DMA (RX и TX) */
    HAL_NVIC_SetPriority(DMA1_Stream0_IRQn, 7, 0);
    HAL_NVIC_EnableIRQ(DMA1_Stream0_IRQn);

    HAL_NVIC_SetPriority(DMA1_Stream6_IRQn, 7, 0); // ИСПРАВЛЕНО: Включено прерывание TX
    HAL_NVIC_EnableIRQ(DMA1_Stream6_IRQn);

    /* ========================================================================= */
    /* 6. Настройка NVIC прерываний для событий и ошибок I2C1                    */
    /* ========================================================================= */
    HAL_NVIC_SetPriority(I2C1_EV_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(I2C1_EV_IRQn);

    HAL_NVIC_SetPriority(I2C1_ER_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(I2C1_ER_IRQn);

    /* ========================================================================= */
    /* 7. Аппаратная инициализация периферии I2C1                                */
    /* ========================================================================= */
    hi2c1.Instance = I2C1;
    hi2c1.Init.Timing = 0x20303E5D; // Расчетный тайминг CubeMX для 100kHz
    hi2c1.Init.OwnAddress1 = 0;
    hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
    hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
    hi2c1.Init.OwnAddress2 = 0;
    hi2c1.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
    hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
    hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;

    if (HAL_I2C_Init(&hi2c1) != HAL_OK)
    {
        return HAL_ERROR;
    }

    if (HAL_I2CEx_ConfigAnalogFilter(&hi2c1, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
    {
        return HAL_ERROR;
    }

    if (HAL_I2CEx_ConfigDigitalFilter(&hi2c1, 0) != HAL_OK)
    {
        return HAL_ERROR;
    }

    /* ========================================================================= */
    /* 8. Настройка DS3231: Включаем меандр 1 Гц на выходе INT/SQW                */
    /* ========================================================================= */
    return Block_RTC_Set1HzOutput(1);
}

/* -------------------------------------------------------------------------- */
/*                   Включение / Выключение меандра 1 Гц (PE3)                */
/* -------------------------------------------------------------------------- */
HAL_StatusTypeDef Block_RTC_Set1HzOutput(uint8_t enable)
{
    uint8_t ctrl_reg = 0;
    uint8_t stat_reg = 0;

    /* Читаем Control и Status */
    if (HAL_I2C_Mem_Read(&hi2c1, DS3231_I2C_ADDR, DS3231_REG_CONTROL, 1, &ctrl_reg, 1, 100) != HAL_OK) return HAL_ERROR;
    if (HAL_I2C_Mem_Read(&hi2c1, DS3231_I2C_ADDR, DS3231_REG_STATUS, 1, &stat_reg, 1, 100) != HAL_OK) return HAL_ERROR;

    if (enable) {
        ctrl_reg &= ~DS3231_CTRL_INTCN; // INTCN = 0 -> Режим генератора SQW
        ctrl_reg &= ~(DS3231_CTRL_RS1 | DS3231_CTRL_RS2); // RS2=0, RS1=0 -> Частота 1 Hz
    } else {
        ctrl_reg |= DS3231_CTRL_INTCN;  // INTCN = 1 -> Отключить SQW
    }

    stat_reg &= ~DS3231_STAT_EN32KHZ; // Отключаем неиспользуемый вывод 32kHz для экономии батарейки

    if (HAL_I2C_Mem_Write(&hi2c1, DS3231_I2C_ADDR, DS3231_REG_CONTROL, 1, &ctrl_reg, 1, 100) != HAL_OK) return HAL_ERROR;
    if (HAL_I2C_Mem_Write(&hi2c1, DS3231_I2C_ADDR, DS3231_REG_STATUS, 1, &stat_reg, 1, 100) != HAL_OK) return HAL_ERROR;

    return HAL_OK;
}

/* -------------------------------------------------------------------------- */
/*                       Установка даты и времени в DS3231                    */
/* -------------------------------------------------------------------------- */
HAL_StatusTypeDef Block_RTC_SetDateTime(const RTC_DateTime_t *dt)
{
    if (!dt) return HAL_ERROR;

    /* Формируем пакет записи (7 байт регистров времени) */
    rtc_tx_dma_buf[0] = RTC_DecToBcd(dt->seconds);
    rtc_tx_dma_buf[1] = RTC_DecToBcd(dt->minutes);
    rtc_tx_dma_buf[2] = RTC_DecToBcd(dt->hours) & 0x3F; // 24-часовой формат (bit 6 = 0)
    rtc_tx_dma_buf[3] = RTC_DecToBcd(dt->day_of_week);
    rtc_tx_dma_buf[4] = RTC_DecToBcd(dt->day);
    rtc_tx_dma_buf[5] = RTC_DecToBcd(dt->month) & 0x1F;
    rtc_tx_dma_buf[6] = RTC_DecToBcd(dt->year);

    /* Очистка D-Cache перед отправкой через DMA */
    SCB_CleanDCache_by_Addr((uint32_t *)rtc_tx_dma_buf, 32);

    /* Пакетная запись всех 7 регистров за один подход (Burst Write) */
    if (HAL_I2C_Mem_Write(&hi2c1, DS3231_I2C_ADDR, DS3231_REG_SECONDS,
                          I2C_MEMADD_SIZE_8BIT, rtc_tx_dma_buf, 7, 100) != HAL_OK)
    {
        return HAL_ERROR;
    }

    /* Сбрасываем флаг сброса питания (OSF), так как время заново задано */
    Block_RTC_ClearOSF();

    return HAL_OK;
}

/* -------------------------------------------------------------------------- */
/*                   Блокирующее считывание даты и времени                    */
/* -------------------------------------------------------------------------- */
HAL_StatusTypeDef Block_RTC_ReadDateTime_Blocking(RTC_DateTime_t *dt)
{
    uint8_t raw_buf[7];
    uint8_t stat_reg = 0;

    if (!dt) return HAL_ERROR;

    /* 1. Пакетное считывание 7 байт времени */
    if (HAL_I2C_Mem_Read(&hi2c1, DS3231_I2C_ADDR, DS3231_REG_SECONDS,
                         I2C_MEMADD_SIZE_8BIT, raw_buf, 7, 100) != HAL_OK)
    {
        return HAL_ERROR;
    }

    /* 2. Чтение регистра статуса для проверки флага OSF (Oscillator Stop Flag) */
    HAL_I2C_Mem_Read(&hi2c1, DS3231_I2C_ADDR, DS3231_REG_STATUS, 1, &stat_reg, 1, 100);

    /* 3. Декодирование BCD */
    dt->seconds     = RTC_BcdToDec(raw_buf[0] & 0x7F);
    dt->minutes     = RTC_BcdToDec(raw_buf[1] & 0x7F);
    dt->hours       = RTC_BcdToDec(raw_buf[2] & 0x3F);
    dt->day_of_week = RTC_BcdToDec(raw_buf[3] & 0x07);
    dt->day         = RTC_BcdToDec(raw_buf[4] & 0x3F);
    dt->month       = RTC_BcdToDec(raw_buf[5] & 0x1F);
    dt->year        = RTC_BcdToDec(raw_buf[6]);
    dt->time_valid  = (stat_reg & DS3231_STAT_OSF) ? 0 : 1;

    /* 4. Параллельно забираем температуру */
    Block_RTC_ReadTemperature(&dt->temperature);

    return HAL_OK;
}

/* -------------------------------------------------------------------------- */
/*               Неблокирующее фоновое считывание через DMA                    */
/* -------------------------------------------------------------------------- */
HAL_StatusTypeDef Block_RTC_ReadDateTime_DMA(void)
{
    g_rtc_rx_cplt_flag = 0;

    /* Инвалидация D-Cache перед приемом данных контроллером DMA */
    SCB_InvalidateDCache_by_Addr((uint32_t *)rtc_rx_dma_buf, 32);

    /* Запуск DMA чтения 7 байт с адреса 0x00 */
    return HAL_I2C_Mem_Read_DMA(&hi2c1, DS3231_I2C_ADDR, DS3231_REG_SECONDS,
                                I2C_MEMADD_SIZE_8BIT, rtc_rx_dma_buf, 7);
}

/* Парсинг буфера DMA после срабатывания коллбека окончания приема */
void Block_RTC_ParseDmaBuffer(RTC_DateTime_t *dt)
{
    if (!dt) return;

    dt->seconds     = RTC_BcdToDec(rtc_rx_dma_buf[0] & 0x7F);
    dt->minutes     = RTC_BcdToDec(rtc_rx_dma_buf[1] & 0x7F);
    dt->hours       = RTC_BcdToDec(rtc_rx_dma_buf[2] & 0x3F);
    dt->day_of_week = RTC_BcdToDec(rtc_rx_dma_buf[3] & 0x07);
    dt->day         = RTC_BcdToDec(rtc_rx_dma_buf[4] & 0x3F);
    dt->month       = RTC_BcdToDec(rtc_rx_dma_buf[5] & 0x1F);
    dt->year        = RTC_BcdToDec(rtc_rx_dma_buf[6]);
}

/* -------------------------------------------------------------------------- */
/*                         Считывание температуры (°C)                         */
/* -------------------------------------------------------------------------- */
HAL_StatusTypeDef Block_RTC_ReadTemperature(float *temp)
{
    uint8_t raw_temp_buf[2];

    if (!temp) return HAL_ERROR;

    if (HAL_I2C_Mem_Read(&hi2c1, DS3231_I2C_ADDR, DS3231_REG_TEMP_MSB,
                         I2C_MEMADD_SIZE_8BIT, raw_temp_buf, 2, 100) == HAL_OK)
    {
        int16_t raw_val = ((int16_t)raw_temp_buf[0] << 8) | raw_temp_buf[1];
        raw_val >>= 6; // Знаковый 10-битный код
        *temp = (float)raw_val * 0.25f;
        return HAL_OK;
    }

    return HAL_ERROR;
}

/* -------------------------------------------------------------------------- */
/*                   Сброс флага сброса питания (OSF)                          */
/* -------------------------------------------------------------------------- */
HAL_StatusTypeDef Block_RTC_ClearOSF(void)
{
    uint8_t stat_reg = 0;

    if (HAL_I2C_Mem_Read(&hi2c1, DS3231_I2C_ADDR, DS3231_REG_STATUS, 1, &stat_reg, 1, 100) == HAL_OK)
    {
        stat_reg &= ~DS3231_STAT_OSF;
        return HAL_I2C_Mem_Write(&hi2c1, DS3231_I2C_ADDR, DS3231_REG_STATUS, 1, &stat_reg, 1, 100);
    }

    return HAL_ERROR;
}

/* -------------------------------------------------------------------------- */
/*                            Обработчики прерываний                          */
/* -------------------------------------------------------------------------- */

/* EXTI3 Handler для секундного импульса (PE3) */
void EXTI3_IRQHandler(void)
{
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_3);
}

/* Коллбек события EXTI3 */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    if (GPIO_Pin == GPIO_PIN_3) {
        g_rtc_1hz_flag = 1; // Устанавливаем флаг: прошла 1 секунда
    }
}

/* DMA1 Stream 0 Interrupt Handler */
void DMA1_Stream0_IRQHandler(void)
{
    HAL_DMA_IRQHandler(&hdma_i2c1_rx);
}

/* Коллбек окончания чтения I2C по DMA */
void HAL_I2C_MemRxCpltCallback(I2C_HandleTypeDef *hi2c)
{
    if (hi2c->Instance == I2C1) {
        g_rtc_rx_cplt_flag = 1; // Сигнал готовности данных в rtc_rx_dma_buf
    }
}

void I2C1_EV_IRQHandler(void)
{
    HAL_I2C_EV_IRQHandler(&hi2c1);
}

void I2C1_ER_IRQHandler(void)
{
    HAL_I2C_ER_IRQHandler(&hi2c1);
}
