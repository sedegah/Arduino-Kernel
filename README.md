# NanoKernel — Minimalist RTOS for ATmega328P


A compact, production-oriented, bare-metal operating environment for the Arduino UNO (ATmega328P). NanoKernel provides cooperative multitasking, a pointer-based serial shell, command parsing, task diagnostics, and a small system utilities suite — all designed to fit into a 32 KB flash / 2 KB SRAM device.


## Quick Start

- Build and upload with PlatformIO (recommended):
```bash
pio run -e uno
pio run -t upload -e uno --upload-port /dev/ttyACM0
```

- Open serial monitor at 115200:
```bash
pio device monitor -p /dev/ttyACM0 -b 115200
```

If you need to run without PlatformIO installed, see the earlier instructions in this repo for Docker and pipx installation.

## Features

- Priority-based scheduler (LOW, NORMAL, HIGH, CRITICAL)
- Per-task statistics: execution count and last runtime in microseconds
- System uptime in hours/minutes/seconds/ms
- `tasks` command now prints task name, priority, state, execs and runtime
- `uptime` command prints formatted uptime
- `mem` command shows SRAM usage and free bytes
- `adc <n>` command reads analog pins A0..A5
- `led <on|off|toggle>` command controls the built-in LED
- `reset` performs a soft system reset

## Shell 

When the board boots you will see the banner and the `nano>` prompt:

```
-------------------------------------------
Initializing NanoKernel
Version: 0.3.0-mvp
Free SRAM: 1564 bytes
Status: Ready. Core scheduler online.
-------------------------------------------
nano>
```

Available commands :

| Command | Description |
|---|---|
| `help` | List available commands |
| `version` | Kernel name and version |
| `clear` | Clear terminal screen |
| `tasks` | Show Task Control Blocks with stats (name, priority, state, execs, last µs) |
| `uptime` | Show system uptime (h m s ms) |
| `mem` | Show SRAM usage and free bytes |
| `adc <0-5>` | Read analog input A0..A5 |
| `led <on|off|toggle>` | Control builtin LED (pin 13) |
| `reset` | Soft reset the system (jump to address 0) |

Examples:

```
nano> tasks
=== Task Control Blocks ===
ID | Name    | Pri     | State    | Execs | Last(µs)
---|---------|---------|----------|-------|--------
0  | shell   | NORMAL  | RUNNING  | 124   | 85
1  | blink   | LOW     | BLOCKED  | 124   | 12
=== End TCB ===

nano> uptime
System uptime: 0h 2m 14s 128ms

nano> mem
SRAM:  757 / 2048 bytes (36%)
Free:  1291 bytes

nano> adc 0
ADC 0 = 512

nano> led toggle
LED ON

nano> reset
System reset...
```

## Scheduler & Tasks

- Scheduler now supports priorities. The scheduler scans from CRITICAL → LOW and runs READY tasks at the highest priority available.
- Tasks report runtime using `micros()` so `tasks` shows the most recent execution time in microseconds and a simple execution count. This helps identify hot code paths and performance regressions in constrained environments.

## Memory & Performance

- Current build (example): RAM used ~757 bytes (37% of 2048). Flash used ~6.1 KB (19% of 32 KB).
- The cooperative design keeps context simple (no stack-switching); runtime per-task overhead remains small.

## Debugging & Troubleshooting

- If the serial monitor repeatedly reconnects, unplug and replug the UNO, close any other serial program, and retry the monitor.
- If upload fails, ensure no monitor is open and try:
```bash
pkill -9 pio || true
sudo pio run -t upload -e uno --upload-port /dev/ttyACM0
```
- On Linux, add your user to `dialout` to avoid sudo for serial access:
```bash
sudo usermod -aG dialout $USER
newgrp dialout
```

## Extending NanoKernel

The codebase is intentionally small and straightforward. To add features:

- Implement a new task handler in `src/` and register it with `scheduler_add_task(fn, PRIORITY_NORMAL)`.
- Add a shell command by creating a `static void cmd_xxx(int argc, char** argv)` and adding an entry to the `commands[]` table in `src/shell.cpp`.

## Project Layout

```
Arduino Nanokernel/
├── platformio.ini
├── README.md
├── include/
│   ├── kernel.h
│   ├── shell.h
│   └── scheduler.h
└── src/
    ├── kernel.cpp
    ├── shell.cpp
    ├── scheduler.cpp
    └── main.cpp
```

## License

Public domain / Educational use. Modify and redistribute freely.

---

Enjoy the NanoKernel — let me know if you want any of the advanced features converted into optional compile-time flags or extended with IPC primitives.
