#include "dispatch.h"
#include <stdio.h>

int main(void) {
    const struct {
        const char* command;
        int argument;
        CommandStatus status;
        int level;
    } steps[] = {
        {"store", 4, COMMAND_OK, 4},
        {"take", 2, COMMAND_OK, 2},
        {"store", 99, COMMAND_FULL, 2},
        {"clear", 0, COMMAND_OK, 0},
        {"unknown", 0, COMMAND_UNKNOWN, 0}
    };
    DeviceState state = {0};
    for (size_t i = 0; i < sizeof(steps) / sizeof(steps[0]); ++i) {
        CommandStatus status = dispatch_command(&state, builtin_commands,
            builtin_command_count, steps[i].command, steps[i].argument);
        if (status != steps[i].status || state.level != steps[i].level) {
            fprintf(stderr, "unfinished or incorrect dispatch at step %zu\n", i);
            return 78;
        }
        printf("%s(%d): %s, level=%d\n", steps[i].command, steps[i].argument,
               command_status_name(status), state.level);
    }
    return 0;
}
