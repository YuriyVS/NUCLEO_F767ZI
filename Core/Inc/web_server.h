#ifndef WEB_SERVER_H
#define WEB_SERVER_H

extern char web_json_buffer[4096];
int fs_open_custom(struct fs_file *file, const char *name);
void fs_close_custom(struct fs_file *file);

#define ALARM_LOG_SIZE 50 // Сколько последних событий храним

typedef struct {
    uint32_t uptime;  // Секунды с момента старта
    uint8_t alarm_id; // Номер бита (0-63)
    uint8_t is_active; // 1 - авария появилась, 0 - пропала
} AlarmEvent_t;

extern AlarmEvent_t alarm_history[ALARM_LOG_SIZE];
extern uint16_t alarm_head; // Указатель на текущую позицию
extern uint32_t last_fault_mask; // Состояние аварий в прошлом цикле
extern void Process_Alarm_Log(void);

#endif
