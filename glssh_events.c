#include "includes.h"
#include "log.h"
#include "xmalloc.h"
#include "glssh_events.h"
#include <regex.h>
#include <stdlib.h>
#include <string.h>

glssh_event_t *glssh_request_events = NULL;
int glssh_request_events_count = 0;

char* glssh_replace_str(const char* str, const char* from, const char* to) {
    if (str == NULL || from == NULL || to == NULL) return NULL;
    if (strlen(from) == 0) return strdup(str);

    // Подсчитываем количество вхождений
    int count = 0;
    const char* tmp = str;
    size_t from_len = strlen(from);

    while ((tmp = strstr(tmp, from)) != NULL) {
        count++;
        tmp += from_len;
    }

    // Если нет вхождений, возвращаем копию
    if (count == 0) {
        return strdup(str);
    }

    // Вычисляем размер результата
    size_t str_len = strlen(str);
    size_t to_len = strlen(to);
    size_t result_len = str_len + count * (to_len - from_len) + 1;

    char* result = malloc(result_len);
    if (result == NULL) return NULL;

    char* dst = result;
    const char* src = str;
    const char* pos;

    while ((pos = strstr(src, from)) != NULL) {
        size_t len = pos - src;
        memcpy(dst, src, len);
        dst += len;

        memcpy(dst, to, to_len);
        dst += to_len;

        src = pos + from_len;
    }

    strcpy(dst, src);
    return result;
}

int glssh_event_add(glssh_event_t **events, int *count, const char *pattern, const char *data, const int process_count){
	
	/*
		Return:
			0 - Error
			1 - OK
 	*/
	
	glssh_event_t *new_events;
	int new_count;

	if (events == NULL || count == NULL || data == NULL || process_count <= 0) {
		debug("GLSSH: invalid parameters for event_add");
		return 0;
	}

	new_count = *count;
	new_events = xreallocarray(*events, new_count+1, sizeof(glssh_event_t));

	if (new_events == NULL) {
		error("GLSSH: failed to allocate memory");
		return 0;
	}

	*events = new_events;

	(*events)[*count].pattern = xstrdup(pattern);
	(*events)[*count].data = xstrdup(data);
	(*events)[*count].process_count = process_count;
	(*events)[*count].processed = 0;

	(*count)++;

	debug("GLSSH: event %d added: pattern='%s'; data='%s'; count='%d'", *count, pattern, data, process_count);
	return 1;
}

int glssh_events_clear(glssh_event_t **events, int *count) {
	if (events == NULL || count == NULL) return 0;
	if (*events != NULL) {
		for (int i=0; i < *count; i++) {
			free((*events)[i].pattern);
			free((*events)[i].data);
		}
		free(*events);
		*events = NULL;
		*count = 0;
		return 1;
	}
	return 0;
}

glssh_event_t *glssh_get_event(glssh_event_t **events, int *events_count, const char *prompt) {
	regex_t regex;
	int ret;

	if (events == NULL || events_count == NULL || prompt == NULL) return NULL;

	debug("GLSSH: Geting prompt='%s'", prompt);
	for (int i=0; i < *events_count; i++) {
		glssh_event_t *event = &(*events)[i];

		if (event->processed >= event->process_count) continue;

		if (event->pattern != NULL && *event->pattern != '\0') {
			ret = regcomp(&regex, event->pattern, REG_EXTENDED | REG_NOSUB | REG_ICASE);
			if (ret == 0) {
				ret = regexec(&regex, prompt, 0, NULL, 0);
				regfree(&regex);

				if (ret == 0) {
					event->processed++;
					return event;
				}
			}
		}
	}
	return NULL;
}
