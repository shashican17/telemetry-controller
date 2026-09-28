#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "system_data.h"
#include <stdio.h>

void logging_task(void *arg)
{
    LogMessage lm;

    while (1) {

        if (xQueueReceive(log_queue, &lm, portMAX_DELAY) == pdTRUE) {
            printf("%s\n", lm.msg);
            fflush(stdout);  
        }
    }
}