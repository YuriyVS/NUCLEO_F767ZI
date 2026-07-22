#include "trace_buffer.h"

// Размещение буферов строго в SRAM1 (адрес 0x20020000+)
__attribute__((section(".sram1_section"))) TraceBuffer_t g_traceBuffer;
__attribute__((section(".sram1_section"))) TraceBuffer_t g_traceSnapshot;

// Инициализация буфера при старте МК
void Trace_Init(void) {
    memset((void*)&g_traceBuffer, 0, sizeof(TraceBuffer_t));
    memset((void*)&g_traceSnapshot, 0, sizeof(TraceBuffer_t));

    g_traceBuffer.head = 0;
    g_traceBuffer.is_running = 1; // По умолчанию запись включена
    g_traceBuffer.is_full = 0;
    g_traceBuffer.sample_count = 0;
}

// Запуск непрерывной записи
void Trace_Start(void) {
    g_traceBuffer.is_running = 1;
}

// Остановка записи (Заморозка)
void Trace_Stop(void) {
    g_traceBuffer.is_running = 0;
}

// Обработчик записи сэмпла (Вызывается СТРОГО из прерывания таймера 1 мс)
void Trace_Process_1ms(float u_seti, float i_akb) {
    if (!g_traceBuffer.is_running) return;

    uint16_t idx = g_traceBuffer.head;

    g_traceBuffer.data[idx].useti = u_seti;
    g_traceBuffer.data[idx].iakb  = i_akb;

    idx++;
    if (idx >= TRACE_SAMPLES) {
        idx = 0;
        g_traceBuffer.is_full = 1;
    }

    g_traceBuffer.head = idx;
    g_traceBuffer.sample_count++;
}
