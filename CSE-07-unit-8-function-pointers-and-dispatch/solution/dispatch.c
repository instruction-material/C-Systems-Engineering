#include "dispatch.h"
#include <string.h>

CommandStatus dispatch_command(DeviceState* state, const CommandEntry* entries,
                               size_t count, const char* command, int argument) {
    if (state == NULL || command == NULL ||
        state->level < 0 || state->level > DEVICE_CAPACITY ||
        (count > 0 && entries == NULL)) {
        return COMMAND_BAD_ARGUMENT;
    }
    for (size_t i = 0; i < count; ++i) {
        if (entries[i].name == NULL || entries[i].handler == NULL) {
            return COMMAND_BAD_ARGUMENT;
        }
    }
    for (size_t i = 0; i < count; ++i) {
        if (strcmp(entries[i].name, command) == 0) {
            return entries[i].handler(state, argument);
        }
    }
    return COMMAND_UNKNOWN;
}
