#ifndef TRACE_BUFFER_H
#define TRACE_BUFFER_H

#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#define TRACE_SAMPLES 1000

// Структура одного сэмпла (8 байт)
typedef struct __attribute__((packed)) {
    float useti;
    float iakb;
} TracePoint_t;

// Структура кольцевого буфера (8008 байт)
typedef struct __attribute__((packed)) {
    volatile uint16_t head;         // Текущий индекс записи (0..999)
    volatile uint8_t  is_running;   // 1 - запись идет, 0 - остановлено
    volatile uint8_t  is_full;      // 1 - сделан хотя бы 1 полный круг
    volatile uint32_t sample_count; // Общий счетчик сэмплов с момента старта
    TracePoint_t      data[TRACE_SAMPLES];
} TraceBuffer_t;

// Экспорт буферов в SRAM1
extern TraceBuffer_t g_traceBuffer;
extern TraceBuffer_t g_traceSnapshot;

// Прототипы функций
extern void Trace_Init(void);
extern void Trace_Start(void);
extern void Trace_Stop(void);

// Функция записи 1ms (сделайте inline или обычный вызов в ISR)
extern void Trace_Process_1ms(float u_seti, float i_akb);

#endif // TRACE_BUFFER_H
