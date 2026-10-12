#include "dispatch.h"
#include <string.h>

CommandStatus dispatch_command(DeviceState* state, const CommandEntry* entries,
                               size_t count, const char* command, int argument) {
    /* TODO: validate the contract, select the first exact name, and call its handler. */
    (void)state;
    (void)entries;
    (void)count;
    (void)command;
    (void)argument;
    return COMMAND_UNKNOWN;
}
