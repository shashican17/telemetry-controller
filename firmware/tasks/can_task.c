#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "system_data.h"

extern void can_init(void);
extern void can_send(int temp, int alarm);

void can_task(void *arg)
{
    ControlStatus status;
    can_init();

    while (1) {
        if (xQueueReceive(can_queue, &status, portMAX_DELAY) == pdTRUE) {
            can_send(status.temperature, status.alarm);
            LOG("[CAN] Sent — Temp=%d  Alarm=%d", status.temperature, status.alarm);
        }
    }
}