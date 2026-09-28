cmake_minimum_required(VERSION 3.10)
project(industrial-controller)
set(CMAKE_C_STANDARD 11)

include_directories(
    FreeRTOS-Kernel/include
    FreeRTOS-Kernel/portable/ThirdParty/GCC/Posix
    FreeRTOS-Kernel/portable/ThirdParty/GCC/Posix/utils
    firmware
    firmware/config     
    firmware/tasks
    firmware/drivers
)

set(FREERTOS_SOURCES
    FreeRTOS-Kernel/tasks.c
    FreeRTOS-Kernel/list.c
    FreeRTOS-Kernel/queue.c
    FreeRTOS-Kernel/timers.c
    FreeRTOS-Kernel/event_groups.c
    FreeRTOS-Kernel/portable/MemMang/heap_4.c
    FreeRTOS-Kernel/portable/ThirdParty/GCC/Posix/port.c
    FreeRTOS-Kernel/portable/ThirdParty/GCC/Posix/utils/wait_for_event.c
)

set(APP_SOURCES
    firmware/main.c
    firmware/tasks/sensor_task.c
    firmware/tasks/control_task.c
    firmware/tasks/can_task.c
    firmware/tasks/logging_task.c
    firmware/tasks/stats_task.c
    firmware/drivers/can_drivers.c
    firmware/drivers/csv_logger.c
)

add_executable(controller ${APP_SOURCES} ${FREERTOS_SOURCES})
target_link_libraries(controller pthread)

add_executable(controller_fault ${APP_SOURCES} ${FREERTOS_SOURCES})
target_link_libraries(controller_fault pthread)
target_compile_definitions(controller_fault PRIVATE FAULT_INJECT)

target_compile_options(controller       PRIVATE -Wall -Wextra)
target_compile_options(controller_fault PRIVATE -Wall -Wextra)