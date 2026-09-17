#ifndef GLSSH_EVENT_H
#define GLSSH_EVENT_H

typedef struct glssh_event {
	char *pattern; /* Шаблон для поиска в промпте (может быть NULL) */
	char *data; /* Данные: команда для выполнения или текст для подстановки */
	int process_count; /* Колличество обработок события */
	int processed; /* Сколько раз было уже обработано это событие */
} glssh_event_t;

extern glssh_event_t *glssh_request_events;
extern int glssh_request_events_count;

char* glssh_replace_str(const char* str, const char* from, const char* to);
int glssh_event_add(glssh_event_t **events, int *count, const char *pattern, const char *data, const int process_count); // 1 - OK; 0 - Error
int glssh_events_clear(glssh_event_t **events, int *count); // 1 - OK; 0 - Error
glssh_event_t *glssh_get_event(glssh_event_t **events, int *events_count, const char *prompt); // Возвращает ссылку на glssh_event_t

#endif
