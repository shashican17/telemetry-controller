#include "FreeRTOS.h"
#include "task.h"
#include "system_data.h"
#include <stdio.h>
#include <string.h>

void stats_task(void *arg)
{

    static char stats_buf[40 * 10];

    vTaskDelay(pdMS_TO_TICKS(5000));  
    while (1) {
        memset(stats_buf, 0, sizeof(stats_buf));
        vTaskGetRunTimeStats(stats_buf);

        printf("\n[STATS] -- Task runtime stats ------------------\n");
        printf("  %-16s %-12s %s\n", "Task", "Abs time", "% time");
        printf("%s", stats_buf);
        printf("[STATS] --------- Free heap: %u bytes-----------------\n\n",
               (unsigned)xPortGetFreeHeapSize());
        fflush(stdout);

        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}