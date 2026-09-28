# CereberusRTOS - FreeRTOS on POSIX

A multi-task embedded controller built with FreeRTOS, simulating an industrial sensor pipeline with CAN bus output. Designed as a portfolio project demonstrating real-time operating system concepts, defensive embedded programming, and production-quality task architecture.

---

## Architecture

```
┌─────────────┐     ┌───────────────┐     ┌──────────────┐
│ sensor_task │────▶│ sensor_queue  │────▶│ control_task │
│  (prio 1)   │     │  (depth: 10)  │     │  (prio 3)    │
└─────────────┘     └───────────────┘     └──────┬───────┘
                                                  │
                                    ┌─────────────▼──────────┐
                                    │      can_queue         │
                                    │      (depth: 10)       │
                                    └─────────────┬──────────┘
                                                  │
┌──────────────┐     ┌───────────────┐     ┌──────▼───────┐
│ logging_task │◀────│   log_queue   │◀────│   can_task  │
│  (prio 2)    │     │  (depth: 20)  │     │  (prio 3)    │
└──────────────┘     └───────────────┘     └──────────────┘
                                                  │
                                         ┌────────▼────────┐
                                         │   sensor_log.csv│
                                         │  (csv_logger.c) │
                                         └─────────────────┘

┌──────────────┐     ┌─────────────────────────────────────┐
│  stats_task  │     │  Heartbeat timer (1 Hz, software)   │
│  (prio 2)    │     │  Logs free heap every second        │
└──────────────┘     └─────────────────────────────────────┘
```

Tasks communicate exclusively through FreeRTOS queues, there are no direct function calls between tasks and no polling loops. Shared data is protected by a mutex.

---

## Features

### Phase 1 - Safety & correctness
- **Queue-based IPC** - sensor → control → CAN pipeline with no shared globals
- **Mutex protection** - `data_mutex` guards all reads and writes to `SensorData`
- **Stack overflow detection** - `configCHECK_FOR_STACK_OVERFLOW 2` catches stack corruption on every context switch and reports the offending task name
- **Timestamped logging** - every log line is prefixed with `xTaskGetTickCount()` in milliseconds for precise timing analysis

### Phase 2 - Observability
- **Thread-safe logging** - all tasks post to `log_queue`; only `logging_task` calls `printf`, eliminating interleaved output
- **Runtime stats** - `stats_task` prints per-task CPU usage every 5 seconds using `vTaskGetRunTimeStats()`
- **Heartbeat timer** - a FreeRTOS software timer fires every 1 s, logging free heap to detect memory leaks over long runs
- **Heap monitoring** - `xPortGetFreeHeapSize()` reported on every stats cycle

### Phase 3 - Resilience & testing
- **Fault injection** - compile with `-DFAULT_INJECT` to simulate a stuck sensor after 10 s; the control task detects the out-of-range sentinel and escalates to `alarm=2`
- **CSV data logger** - every sensor reading and alarm decision is appended to `sensor_log.csv` with a tick timestamp; mutex-protected for safe concurrent access
- **Graceful shutdown** - `SIGINT`/`SIGTERM` handler flushes and closes the CSV before exit

---

## Project structure

```
industrial-controller/
├── firmware/
│   ├── config/
│   │   └── FreeRTOSConfig.h       # Kernel configuration
│   ├── main.c                     # Scheduler init, queues, mutex, heartbeat timer
│   ├── system_data.h              # Shared structs, LOG() macro, extern handles
│   ├── tasks/
│   │   ├── sensor_task.c          # Reads sensors, posts to sensor_queue
│   │   ├── control_task.c         # Alarm logic, posts to can_queue + CSV
│   │   ├── can_task.c             # Consumes can_queue, calls CAN driver
│   │   ├── logging_task.c         # Sole printf caller - drains log_queue
│   │   └── stats_task.c           # CPU % per task every 5 s
│   └── drivers/
│       ├── can_drivers.c          # CAN hardware abstraction (stubbed for POSIX)
│       ├── csv_logger.h
│       └── csv_logger.c           # Mutex-protected CSV append
├── FreeRTOS-Kernel/               # FreeRTOS source (submodule)
└── CMakeLists.txt
```

---

## Build & run

### Prerequisites

```bash
sudo apt install cmake gcc git
```

### Clone with FreeRTOS kernel

```bash
git clone --recurse-submodules https://github.com/shashican17/telemetry-controller.git
cd telemetry-controller
```

### Build

```bash
mkdir build && cd build
cmake ..
make
```

This produces two binaries:

| Binary | Description |
|---|---|
| `controller` | Normal run |
| `controller_fault` | Fault injection - sensor freezes at 10 s |

### Run

```bash
# Normal operation
./controller

# Fault injection - watch alarm escalate to 2 after 10 s
./controller_fault
```

Press **Ctrl+C** to stop. The CSV log is flushed cleanly on exit.

### Expected output

```
Industrial Controller Starting...
[MAIN] Task 'SensorTask' created
[MAIN] Task 'ControlTask' created
...
[  1023 ms] [SENSOR] Temp=83  Pressure=36
[  1023 ms] [CONTROL] High temp (83°C) - cooling activated  alarm=1
[  1023 ms] [CAN] Sent - Temp=83  Alarm=1
[  1024 ms] [HEARTBEAT] System alive | free heap: 58320 bytes
...
[STATS] ── Task runtime stats ──────────────────
  Task             Abs time     % time
  SensorTask       1023         2%
  ControlTask      4812         9%
  CANTask          3201         6%
  LogTask          512          1%
  StatsTask        128          <1%
[STATS] ── Free heap: 58320 bytes ─────────────
```

### CSV output

After running, `sensor_log.csv` in the build directory contains:

```csv
tick_ms,temperature,pressure,alarm
1023,83,36,1
2041,49,21,0
3058,62,27,1
...
```

---

## Key design decisions

**Why queues instead of global variables?**
The original version used plain `int temperature` globals shared between tasks. This is a race condition - the sensor task can write mid-read by the control task. Replacing globals with a queue gives atomic, ordered delivery with no locking required at the point of transfer.

**Why a dedicated logging task?**
`printf` is not thread-safe on all platforms. Even where it is, concurrent calls from multiple tasks produce interleaved output that is impossible to read. Routing all output through a single task and a queue gives clean, ordered, timestamped output with zero risk of interleaving.

**Why `configCHECK_FOR_STACK_OVERFLOW 2`?**
Method 1 only checks the stack pointer on context switch. Method 2 additionally fills the stack with a known pattern at task creation and checks for pattern corruption - it catches overflows that happen mid-task, not just at the switch boundary. The cost is a small overhead on every context switch, which is acceptable here.

**Why two build targets for fault injection?**
A compile-time flag (`-DFAULT_INJECT`) rather than a runtime flag keeps the fault path completely absent from the production binary. There is zero overhead and zero risk of accidentally enabling it in a deployed image.

---

## What's next (hardware deployment)

This project runs on the FreeRTOS POSIX simulator. Porting to real hardware would involve:

- Replacing `portable/ThirdParty/GCC/Posix` with the target port (e.g. `portable/GCC/ARM_CM4F` for STM32F4)
- Implementing `can_drivers.c` against a real MCP2515 or on-chip CAN peripheral
- Replacing `csv_logger.c` with FatFS writes to an SD card
- Adding a hardware watchdog (IWDG on STM32) alongside the software stack overflow hook
- Replacing `rand()` with actual ADC reads from temperature and pressure sensors

---

## Concepts demonstrated

| Concept | Where |
|---|---|
| Multi-task design | All tasks in `firmware/tasks/` |
| Queue-based IPC | `sensor_queue`, `can_queue`, `log_queue` in `main.c` |
| Mutex / critical section | `data_mutex` in `sensor_task.c`, `control_task.c` |
| Software timers | Heartbeat in `main.c` |
| Stack overflow detection | `vApplicationStackOverflowHook` in `main.c` |
| Runtime statistics | `stats_task.c` |
| Fault injection | `#ifdef FAULT_INJECT` in `sensor_task.c` |
| Thread-safe file I/O | `csv_logger.c` |
| Signal handling | `SIGINT` handler in `main.c` |
| CMake multi-target build | `CMakeLists.txt` |

---

## License

MIT
