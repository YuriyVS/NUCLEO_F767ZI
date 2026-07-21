#include "event_log.h"
#include <string.h>

// Объявление журнала (автоматически попадает в SRAM1)
volatile LogBuffer_t EventLogBuffer;

void Log_Init(void) {
    memset((void*)&EventLogBuffer, 0, sizeof(EventLogBuffer));
    // Фиксируем первое событие — старт системы
    Log_Write(LOG_EV_SYS_INIT, LOG_SEV_INFO, 0.0f, 0);
}

// Потокобезопасная запись события в кольцевой буфер
void Log_Write(uint16_t event_id, uint8_t severity, float value, uint32_t param_id) {
    // Защита от прерываний на время записи (критическая секция)
    uint32_t primask = __get_PRIMASK();
    __disable_irq();

    uint32_t head = EventLogBuffer.head;

    // Заполняем запись
    EventLogBuffer.entries[head].timestamp = HAL_GetTick(); // Можно заменить на Unix-time от RTC
    EventLogBuffer.entries[head].event_id  = event_id;
    EventLogBuffer.entries[head].severity  = severity;
    EventLogBuffer.entries[head].flags     = 0;
    EventLogBuffer.entries[head].value     = value;
    EventLogBuffer.entries[head].param_id  = param_id;

    // Сдвигаем индекс head по кольцу
    EventLogBuffer.head = (head + 1) % LOG_MAX_ENTRIES;

    // Увеличиваем счетчики
    if (EventLogBuffer.count < LOG_MAX_ENTRIES) {
        EventLogBuffer.count++;
    }
    EventLogBuffer.total_recorded++;

#if defined(STM32F7) || defined(STM32H7)
    // Выталкиваем обновленный элемент из D-Cache в SRAM1
    SCB_CleanDCache_by_Addr((uint32_t*)&EventLogBuffer.entries[head], sizeof(LogEntry_t));
#endif

    // Восстанавливаем прерывания
    if (!primask) {
        __enable_irq();
    }
}

// Чтение записи со смещением от самого свежего события (0 = самое свежее)
bool Log_GetEntry(uint32_t depth, LogEntry_t *out_entry) {
    if (depth >= EventLogBuffer.count || out_entry == NULL) {
        return false;
    }

    // Вычисляем индекс в ring-буфере:
    // depth = 0 -> самое последнее событие
    int32_t read_idx = (int32_t)EventLogBuffer.head - 1 - (int32_t)depth;
    if (read_idx < 0) {
        read_idx += LOG_MAX_ENTRIES;
    }

    *out_entry = EventLogBuffer.entries[read_idx];
    return true;
}

// Очистка журнала
void Log_Clear(void) {
    __disable_irq();
    EventLogBuffer.head = 0;
    EventLogBuffer.count = 0;
    __enable_irq();
}
