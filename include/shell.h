#ifndef SHELL_H
#define SHELL_H

#include <Arduino.h>

#define MAX_CMD_LEN 32
#define MAX_ARGS    4

typedef void (*cmd_handler_t)(int argc, char** argv);

struct ShellCommand {
    const char* name;
    cmd_handler_t handler;
    const char* help_text;
};

void shell_init();
void shell_update();

#endif // SHELL_H
