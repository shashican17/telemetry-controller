#ifndef SYSTEM_DATA_H
#define SYSTEM_DATA_H

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include <stdio.h>


typedef struct {
    int temperature;
    int pressure;
} SensorData;

typedef struct {
    int temperature;
    int alarm;
} ControlStatus;

#define LOG_MSG_MAX_LEN  96

typedef struct {
    char msg[LOG_MSG_MAX_LEN];
} LogMessage;

extern QueueHandle_t sensor_queue;
extern QueueHandle_t can_queue;
extern QueueHandle_t log_queue;       
extern SemaphoreHandle_t data_mutex;  

#define LOG(fmt, ...)                                                   \
    do {                                                                \
        LogMessage _lm;                                                 \
        TickType_t _t = xTaskGetTickCount();                            \
        snprintf(_lm.msg, LOG_MSG_MAX_LEN,                              \
                 "[%6lu ms] " fmt, (unsigned long)_t, ##__VA_ARGS__);   \
        if (log_queue) {                                                \
            xQueueSend(log_queue, &_lm, pdMS_TO_TICKS(10));            \
        } else {                                                        \
            printf("%s\n", _lm.msg);                                    \
        }                                                               \
    } while(0)

#endif 