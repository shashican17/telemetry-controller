#include "csv_logger.h"
#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"
#include <stdio.h>
#include <string.h>

static FILE            *s_file  = NULL;
static SemaphoreHandle_t s_mutex = NULL;

int csv_logger_init(const char *filepath)
{
    s_mutex = xSemaphoreCreateMutex();
    if (!s_mutex) {
        fprintf(stderr, "[CSV] ERROR: could not create mutex\n");
        return -1;
    }

    s_file = fopen(filepath, "a");
    if (!s_file) {
        fprintf(stderr, "[CSV] ERROR: could not open '%s'\n", filepath);
        return -1;
    }

    fseek(s_file, 0, SEEK_END);
    if (ftell(s_file) == 0) {
        fprintf(s_file, "tick_ms,temperature,pressure,alarm\n");
        fflush(s_file);
    }

    printf("[CSV] Logging to '%s'\n", filepath);
    return 0;
}

void csv_logger_write(const SensorData *data, const ControlStatus *status)
{
    if (!s_file || !s_mutex) return;

    if (xSemaphoreTake(s_mutex, pdMS_TO_TICKS(50)) == pdTRUE) {
        fprintf(s_file, "%lu,%d,%d,%d\n",
                (unsigned long)xTaskGetTickCount(),
                data->temperature,
                data->pressure,
                status->alarm);
        fflush(s_file);
        xSemaphoreGive(s_mutex);
    }
}

void csv_logger_close(void)
{
    if (s_file) {
        fclose(s_file);
        s_file = NULL;
    }
}