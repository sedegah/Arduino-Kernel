#include "scheduler.h"

static TaskControlBlock task_table[MAX_TASKS];
static int8_t current_task_id = -1;
static uint32_t system_uptime_ms = 0;
static uint32_t last_millis = 0;

void scheduler_init() {
    for (uint8_t i = 0; i < MAX_TASKS; i++) {
        task_table[i].callback = NULL;
        task_table[i].state = TASK_STATE_INACTIVE;
        task_table[i].priority = PRIORITY_NORMAL;
        task_table[i].sleep_until = 0;
        task_table[i].exec_count = 0;
        task_table[i].last_runtime_us = 0;
    }
    system_uptime_ms = 0;
    last_millis = millis();
}

bool scheduler_add_task(task_func_t func, task_priority_t priority) {
    for (uint8_t i = 0; i < MAX_TASKS; i++) {
        if (task_table[i].state == TASK_STATE_INACTIVE || task_table[i].callback == NULL) {
            task_table[i].callback = func;
            task_table[i].state = TASK_STATE_READY;
            task_table[i].priority = priority;
            task_table[i].sleep_until = 0;
            task_table[i].exec_count = 0;
            return true;
        }
    }
    return false;
}

void scheduler_start() {
    while (1) {
        // Update system uptime
        uint32_t now = millis();
        system_uptime_ms += (now - last_millis);
        last_millis = now;

        // Priority-based scheduling: execute highest priority READY task
        for (int8_t pri = PRIORITY_CRITICAL; pri >= PRIORITY_LOW; pri--) {
            for (uint8_t i = 0; i < MAX_TASKS; i++) {
                if (task_table[i].callback == NULL) continue;

                if (task_table[i].state == TASK_STATE_BLOCKED) {
                    if ((int32_t)(millis() - task_table[i].sleep_until) >= 0) {
                        task_table[i].state = TASK_STATE_READY;
                    }
                }

                if (task_table[i].state == TASK_STATE_READY && task_table[i].priority == pri) {
                    current_task_id = i;
                    task_table[i].state = TASK_STATE_RUNNING;
                    task_table[i].exec_count++;

                    uint32_t start_us = micros();
                    task_table[i].callback();
                    task_table[i].last_runtime_us = micros() - start_us;

                    if (task_table[i].state == TASK_STATE_RUNNING) {
                        task_table[i].state = TASK_STATE_READY;
                    }
                }
            }
        }
    }
}

void kernel_yield() {
    if (current_task_id >= 0 && task_table[current_task_id].state == TASK_STATE_RUNNING) {
        task_table[current_task_id].state = TASK_STATE_READY;
    }
}

void kernel_delay(uint32_t ms) {
    if (current_task_id >= 0 && task_table[current_task_id].state == TASK_STATE_RUNNING) {
        task_table[current_task_id].sleep_until = millis() + ms;
        task_table[current_task_id].state = TASK_STATE_BLOCKED;
    }
}

uint32_t scheduler_get_uptime_ms() {
    return system_uptime_ms;
}

const TaskControlBlock* scheduler_get_task_table() {
    return task_table;
}

uint8_t scheduler_get_max_tasks() {
    return MAX_TASKS;
}
