#include <stdio.h>
#include "csv_logger.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "timers.h"
#include "system_data.h"

QueueHandle_t    sensor_queue;
QueueHandle_t    can_queue;
QueueHandle_t    log_queue;       
SemaphoreHandle_t data_mutex;     
void sensor_task(void *arg);
void control_task(void *arg);
void can_task(void *arg);
void logging_task(void *arg);
void stats_task(void *arg);       
void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName)
{
    (void)xTask;
    fprintf(stderr, "\n[FATAL] Stack overflow in task: %s\n", pcTaskName);
    fprintf(stderr, "        Increase the stack size for this task.\n");
    fflush(stderr);
    for(;;);   
}

static void heartbeat_callback(TimerHandle_t xTimer)
{
    (void)xTimer;
    size_t free_heap = xPortGetFreeHeapSize();
    LOG("[HEARTBEAT] System alive | free heap: %u bytes", (unsigned)free_heap);
}

static void create_task(TaskFunction_t fn, const char *name,
                        uint16_t stack, UBaseType_t priority)
{
    if (xTaskCreate(fn, name, stack, NULL, priority, NULL) != pdPASS) {
        fprintf(stderr, "[MAIN] ERROR: could not create task '%s'\n", name);
        for(;;);
    }
    printf("[MAIN] Task '%s' created\n", name);
}

int main(void)
{
    printf("Industrial Controller Starting...\n");

    if (csv_logger_init("sensor_log.csv") != 0) {
        fprintf(stderr, "[MAIN] WARNING: CSV logging disabled\n");
    }

    data_mutex = xSemaphoreCreateMutex();
    if (!data_mutex) {
        fprintf(stderr, "[MAIN] ERROR: mutex creation failed\n");
        return 1;
    }

    log_queue = xQueueCreate(20, sizeof(LogMessage));
    if (!log_queue) {
        fprintf(stderr, "[MAIN] ERROR: log queue creation failed\n");
        return 1;
    }

    sensor_queue = xQueueCreate(10, sizeof(SensorData));
    can_queue    = xQueueCreate(10, sizeof(ControlStatus));
    if (!sensor_queue || !can_queue) {
        fprintf(stderr, "[MAIN] ERROR: data queue creation failed\n");
        return 1;
    }

    create_task(sensor_task,  "SensorTask",  1024, 1);
    create_task(control_task, "ControlTask", 1024, 3);
    create_task(can_task,     "CANTask",     1024, 3);
    create_task(logging_task, "LogTask",     1024, 2);
    create_task(stats_task,   "StatsTask",   2048, 2); 
    TimerHandle_t heartbeat = xTimerCreate(
        "Heartbeat",
        pdMS_TO_TICKS(1000),
        pdTRUE,       
        NULL,
        heartbeat_callback
    );
    if (!heartbeat || xTimerStart(heartbeat, 0) != pdPASS) {
        fprintf(stderr, "[MAIN] ERROR: heartbeat timer failed\n");
        return 1;
    }
    printf("[MAIN] Heartbeat timer started\n");

    vTaskStartScheduler();
    fprintf(stderr, "[MAIN] FATAL: scheduler returned\n");
    return 1;
}