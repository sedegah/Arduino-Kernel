#include "kernel.h"

void kernel_init() {
    Serial.begin(115200);
    while (!Serial) {
        ;
    }
}

void kernel_boot_banner() {
    Serial.println(F("\n-------------------------------------------"));
    Serial.print(F("Initializing "));
    Serial.println(F(KERNEL_NAME));
    Serial.print(F("Version: "));
    Serial.println(F(KERNEL_VERSION));
    Serial.print(F("Free SRAM: "));

    extern int __heap_start, *__brkval;
    int v;
    int free_memory = (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval);

    Serial.print(free_memory);
    Serial.println(F(" bytes"));
    Serial.println(F("Status: Ready. Core scheduler online."));
    Serial.println(F("-------------------------------------------"));
}
