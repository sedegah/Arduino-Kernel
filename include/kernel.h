#ifndef KERNEL_H
#define KERNEL_H

#include <Arduino.h>

#define KERNEL_NAME    "NanoKernel"
#define KERNEL_VERSION "0.3.0-mvp"

void kernel_init();
void kernel_boot_banner();

#endif // KERNEL_H
