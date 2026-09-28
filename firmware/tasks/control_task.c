#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "system_data.h"
#include "csv_logger.h"

void control_task(void *arg)
{
    SensorData    data;
    ControlStatus status;

    while (1) {
       
        if (xQueueReceive(sensor_queue, &data, portMAX_DELAY) == pdTRUE) {
            if (xSemaphoreTake(data_mutex, pdMS_TO_TICKS(50)) != pdTRUE) {
                LOG("[CONTROL] Could not acquire mutex — skipping cycle");
                continue;
            }

            status.temperature = data.temperature;

            if (data.temperature == 999 || data.pressure < 0) {
                status.alarm = 2; 
                LOG("[CONTROL] SENSOR FAULT detected — alarm=2");
            } else if (data.temperature > 60) {
                status.alarm = 1;
                LOG("[CONTROL] High temp (%d°C) — cooling activated  alarm=1",
                    data.temperature);
            } else {
                status.alarm = 0;
                LOG("[CONTROL] Temp=%d°C  pressure=%d  alarm=0",
                    data.temperature, data.pressure);
            }

            xSemaphoreGive(data_mutex);
            csv_logger_write(&data, &status);

            xQueueSend(can_queue, &status, portMAX_DELAY);
        }
    }
}