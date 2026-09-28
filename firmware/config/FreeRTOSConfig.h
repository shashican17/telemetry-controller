#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H
#define configUSE_PREEMPTION                    1
#define configUSE_IDLE_HOOK                     0
#define configUSE_TICK_HOOK                     0
#define configIDLE_SHOULD_YIELD                 1
#define configCPU_CLOCK_HZ                      1000000
#define configTICK_RATE_HZ                      1000
#define configUSE_16_BIT_TICKS                  0
#define configMAX_PRIORITIES                    5
#define configMINIMAL_STACK_SIZE                256
#define configMAX_TASK_NAME_LEN                 16
#define configTOTAL_HEAP_SIZE                   (64 * 1024)
#define configUSE_MUTEXES                       1
#define configUSE_COUNTING_SEMAPHORES           1
#define configUSE_QUEUE_SETS                    0
#define configQUEUE_REGISTRY_SIZE               10
#define configCHECK_FOR_STACK_OVERFLOW          2
#define configGENERATE_RUN_TIME_STATS           1
#define configUSE_STATS_FORMATTING_FUNCTIONS    1
#define configUSE_TRACE_FACILITY                1
#define portCONFIGURE_TIMER_FOR_RUN_TIME_STATS()    /* nothing needed on POSIX */
#define portGET_RUN_TIME_COUNTER_VALUE()            xTaskGetTickCount()
#define configUSE_TIMERS                        1
#define configTIMER_TASK_PRIORITY               2
#define configTIMER_QUEUE_LENGTH                10
#define configTIMER_TASK_STACK_DEPTH            256
#define configUSE_POSIX_ERRNO                   1
#define configUSE_APPLICATION_TASK_TAG          0
#define INCLUDE_vTaskDelay                      1
#define INCLUDE_vTaskDelete                     1
#define INCLUDE_vTaskSuspend                    1
#define INCLUDE_xTaskGetSchedulerState          1
#define INCLUDE_vTaskPrioritySet                1
#define INCLUDE_uxTaskPriorityGet               1
#define INCLUDE_xQueueGetMutexHolder            1
#define INCLUDE_uxTaskGetStackHighWaterMark     1

#endif