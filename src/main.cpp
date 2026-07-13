#include "kernel.h"
#include "shell.h"
#include "scheduler.h"

void task_shell() {
    shell_update();
    kernel_yield();
}

void task_blink() {
    static bool state = false;
    pinMode(LED_BUILTIN, OUTPUT);
    state = !state;
    digitalWrite(LED_BUILTIN, state ? HIGH : LOW);
    kernel_delay(500);
}

void setup() {
    kernel_init();
    kernel_boot_banner();
    shell_init();

    scheduler_init();
    scheduler_add_task(task_shell, PRIORITY_NORMAL);
    scheduler_add_task(task_blink, PRIORITY_LOW);

    scheduler_start();
}

void loop() {
    // Left empty intentionally as scheduler handles execution
}
