#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "system_data.h"
#include <stdlib.h>

#ifdef FAULT_INJECT
  #define FAULT_INJECT_AFTER_TICKS  pdMS_TO_TICKS(10000) 
  static int fault_active = 0;
#endif

void sensor_task(void *arg)
{
    SensorData data;

    while (1) {

#ifdef FAULT_INJECT
        if (!fault_active && xTaskGetTickCount() > FAULT_INJECT_AFTER_TICKS) {
            fault_active = 1;
            LOG("[SENSOR] *** FAULT INJECTED — sensor stuck ***");
        }
        if (fault_active) {
            data.temperature = 999;  
            data.pressure    = -1;
        } else {
#endif
            data.temperature = rand() % 100;
            data.pressure    = rand() % 50;
#ifdef FAULT_INJECT
        }
#endif

        if (xSemaphoreTake(data_mutex, pdMS_TO_TICKS(50)) == pdTRUE) {
            if (xQueueSend(sensor_queue, &data, pdMS_TO_TICKS(100)) == pdTRUE) {
                LOG("[SENSOR] Temp=%d  Pressure=%d", data.temperature, data.pressure);
            } else {
                LOG("[SENSOR] Queue full — dropping reading");
            }
            xSemaphoreGive(data_mutex);
        } else {
            LOG("[SENSOR] Could not acquire mutex");
        }

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}