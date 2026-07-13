#include "shell.h"
#include "kernel.h"
#include "scheduler.h"

static char rx_buffer[MAX_CMD_LEN];
static uint8_t rx_index = 0;

static void cmd_help(int argc, char** argv);
static void cmd_version(int argc, char** argv);
static void cmd_clear(int argc, char** argv);
static void cmd_tasks(int argc, char** argv);
static void cmd_uptime(int argc, char** argv);
static void cmd_mem(int argc, char** argv);
static void cmd_adc(int argc, char** argv);
static void cmd_led(int argc, char** argv);
static void cmd_reset(int argc, char** argv);

static const ShellCommand commands[] = {
    {"help",    cmd_help,    "List available commands"},
    {"version", cmd_version, "Show kernel version details"},
    {"clear",   cmd_clear,   "Clear the terminal screen"},
    {"tasks",   cmd_tasks,   "Show Task Control Blocks with stats"},
    {"uptime",  cmd_uptime,  "Show system uptime"},
    {"mem",     cmd_mem,     "Show SRAM memory usage"},
    {"adc",     cmd_adc,     "Read analog pin (e.g: adc 0)"},
    {"led",     cmd_led,     "Control LED (on|off|toggle)"},
    {"reset",   cmd_reset,   "Soft reset the system"}
};
static const uint8_t num_commands = sizeof(commands) / sizeof(ShellCommand);

void shell_init() {
    memset(rx_buffer, 0, MAX_CMD_LEN);
    rx_index = 0;
    Serial.print(F("nano> "));
}

static void execute_command(char* line) {
    char* argv[MAX_ARGS];
    int argc = 0;

    char* token = strtok(line, " ");
    while (token != NULL && argc < MAX_ARGS) {
        argv[argc++] = token;
        token = strtok(NULL, " ");
    }

    if (argc == 0) {
        Serial.print(F("nano> "));
        return;
    }

    for (uint8_t i = 0; i < num_commands; i++) {
        if (strcmp(argv[0], commands[i].name) == 0) {
            commands[i].handler(argc, argv);
            Serial.print(F("nano> "));
            return;
        }
    }

    Serial.print(F("Error: Command '"));
    Serial.print(argv[0]);
    Serial.println(F("' not found. Type 'help'."));
    Serial.print(F("nano> "));
}

void shell_update() {
    while (Serial.available() > 0) {
        char c = (char)Serial.read();

        if (c == '\r' || c == '\n') {
            if (rx_index > 0) {
                rx_buffer[rx_index] = '\0';
                Serial.println();
                execute_command(rx_buffer);
                rx_index = 0;
                memset(rx_buffer, 0, MAX_CMD_LEN);
            } else {
                Serial.println();
                Serial.print(F("nano> "));
            }
        }
        else if (c == '\b' || c == 127) {
            if (rx_index > 0) {
                rx_index--;
                rx_buffer[rx_index] = '\0';
                Serial.print(F("\b \b"));
            }
        }
        else {
            if (rx_index < (MAX_CMD_LEN - 1)) {
                rx_buffer[rx_index++] = c;
                Serial.print(c);
            }
        }
    }
}

static void cmd_help(int argc, char** argv) {
    Serial.println(F("Available commands:"));
    for (uint8_t i = 0; i < num_commands; i++) {
        Serial.print(F(" - "));
        Serial.print(commands[i].name);
        Serial.print(F(": "));
        Serial.println(commands[i].help_text);
    }
}

static void cmd_version(int argc, char** argv) {
    Serial.print(F("Kernel: "));
    Serial.println(F(KERNEL_NAME));
    Serial.print(F("Version: "));
    Serial.println(F(KERNEL_VERSION));
}

static void cmd_clear(int argc, char** argv) {
    // ANSI clear screen
    Serial.write(27); // ESC
    Serial.print(F("[2J[H"));
}

extern void task_shell();
extern void task_blink();

static const char* get_task_name(const TaskControlBlock* tcb) {
    if (tcb->callback == NULL) return "none";
    if (tcb->callback == task_shell) return "shell";
    if (tcb->callback == task_blink) return "blink";
    return "custom";
}

static const char* get_priority_name(task_priority_t pri) {
    switch (pri) {
        case PRIORITY_LOW:      return "LOW";
        case PRIORITY_NORMAL:   return "NORMAL";
        case PRIORITY_HIGH:     return "HIGH";
        case PRIORITY_CRITICAL: return "CRITICAL";
        default:                return "???";
    }
}

static void cmd_tasks(int argc, char** argv) {
    const TaskControlBlock* tasks = scheduler_get_task_table();
    uint8_t max_tasks = scheduler_get_max_tasks();
    
    Serial.println(F("=== Task Control Blocks ==="));
    Serial.println(F("ID | Name    | Pri     | State    | Execs | Last(µs)"));
    Serial.println(F("---|---------|---------|----------|-------|--------"));
    
    for (uint8_t i = 0; i < max_tasks; i++) {
        if (tasks[i].state == TASK_STATE_INACTIVE) continue;
        
        Serial.print(i);
        Serial.print(F("  | "));
        
        const char* name = get_task_name((TaskControlBlock*)&tasks[i]);
        Serial.print(name);
        for (uint8_t j = strlen(name); j < 7; j++) Serial.print(F(" "));
        Serial.print(F(" | "));
        
        const char* pri_str = get_priority_name(tasks[i].priority);
        Serial.print(pri_str);
        for (uint8_t j = strlen(pri_str); j < 7; j++) Serial.print(F(" "));
        Serial.print(F(" | "));
        
        const char* state_str = "UNKNOWN";
        switch (tasks[i].state) {
            case TASK_STATE_READY:    state_str = "READY   "; break;
            case TASK_STATE_RUNNING:  state_str = "RUNNING "; break;
            case TASK_STATE_BLOCKED:  state_str = "BLOCKED "; break;
            case TASK_STATE_INACTIVE: state_str = "INACTIVE"; break;
        }
        Serial.print(state_str);
        Serial.print(F(" | "));
        Serial.print(tasks[i].exec_count);
        Serial.print(F("    | "));
        Serial.println(tasks[i].last_runtime_us);
    }
    Serial.println(F("=== End TCB ==="));
}

static void cmd_uptime(int argc, char** argv) {
    uint32_t uptime = scheduler_get_uptime_ms();
    uint32_t hours = uptime / 3600000UL;
    uint32_t mins = (uptime % 3600000UL) / 60000UL;
    uint32_t secs = (uptime % 60000UL) / 1000UL;
    uint32_t ms = uptime % 1000UL;
    
    Serial.print(F("System uptime: "));
    Serial.print(hours);
    Serial.print(F("h "));
    Serial.print(mins);
    Serial.print(F("m "));
    Serial.print(secs);
    Serial.print(F("s "));
    Serial.print(ms);
    Serial.println(F("ms"));
}

static void cmd_mem(int argc, char** argv) {
    extern int __heap_start, *__brkval;
    int v;
    int free_memory = (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval);
    int used = 2048 - free_memory;
    
    Serial.print(F("SRAM:  "));
    Serial.print(used);
    Serial.print(F(" / 2048 bytes ("));
    Serial.print((used * 100) / 2048);
    Serial.println(F("%)"));
    Serial.print(F("Free:  "));
    Serial.print(free_memory);
    Serial.println(F(" bytes"));
}

static void cmd_adc(int argc, char** argv) {
    if (argc < 2) {
        Serial.println(F("Usage: adc <pin> (0-5)"));
        return;
    }
    int pin = atoi(argv[1]);
    if (pin < 0 || pin > 5) {
        Serial.println(F("Pin out of range (0-5)"));
        return;
    }
    int value = analogRead(pin);
    Serial.print(F("ADC "));
    Serial.print(pin);
    Serial.print(F(" = "));
    Serial.println(value);
}

static void cmd_led(int argc, char** argv) {
    if (argc < 2) {
        Serial.println(F("Usage: led <on|off|toggle>"));
        return;
    }
    
    static bool led_state = false;
    pinMode(LED_BUILTIN, OUTPUT);
    
    if (strcmp(argv[1], "on") == 0) {
        digitalWrite(LED_BUILTIN, HIGH);
        led_state = true;
        Serial.println(F("LED ON"));
    } else if (strcmp(argv[1], "off") == 0) {
        digitalWrite(LED_BUILTIN, LOW);
        led_state = false;
        Serial.println(F("LED OFF"));
    } else if (strcmp(argv[1], "toggle") == 0) {
        led_state = !led_state;
        digitalWrite(LED_BUILTIN, led_state ? HIGH : LOW);
        Serial.print(F("LED "));
        Serial.println(led_state ? F("ON") : F("OFF"));
    }
}

static void cmd_reset(int argc, char** argv) {
    Serial.println(F("System reset..."));
    delay(100);
    asm volatile ("jmp 0");  // Jump to bootloader (soft reset)
}
