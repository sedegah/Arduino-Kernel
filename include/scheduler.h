#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <Arduino.h>
#include <stdint.h>

#define MAX_TASKS 4

typedef enum {
    TASK_STATE_READY,
    TASK_STATE_RUNNING,
    TASK_STATE_BLOCKED,
    TASK_STATE_INACTIVE
} task_state_t;

typedef enum {
    PRIORITY_LOW = 0,
    PRIORITY_NORMAL = 1,
    PRIORITY_HIGH = 2,
    PRIORITY_CRITICAL = 3
} task_priority_t;

typedef void (*task_func_t)();

struct TaskControlBlock {
    task_func_t callback;
    task_state_t state;
    task_priority_t priority;
    uint32_t sleep_until;
    uint32_t exec_count;      // execution counter
    uint16_t last_runtime_us;  // microseconds spent last run
};

void scheduler_init();
bool scheduler_add_task(task_func_t func, task_priority_t priority);
void scheduler_start();
void kernel_yield();
void kernel_delay(uint32_t ms);
const TaskControlBlock* scheduler_get_task_table();
uint8_t scheduler_get_max_tasks();
uint32_t scheduler_get_uptime_ms();

#endif // SCHEDULER_H
