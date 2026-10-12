#ifndef CSE_DISPATCH_H
#define CSE_DISPATCH_H

#include <stddef.h>

enum { DEVICE_CAPACITY = 100 };

typedef struct {
    int level;
} DeviceState;

typedef enum {
    COMMAND_OK,
    COMMAND_BAD_ARGUMENT,
    COMMAND_UNKNOWN,
    COMMAND_FULL,
    COMMAND_EMPTY
} CommandStatus;

typedef CommandStatus (*CommandHandler)(DeviceState* state, int argument);

typedef struct {
    const char* name;
    CommandHandler handler;
} CommandEntry;

extern const CommandEntry builtin_commands[];
extern const size_t builtin_command_count;

CommandStatus dispatch_command(DeviceState* state, const CommandEntry* entries,
                               size_t count, const char* command, int argument);
const char* command_status_name(CommandStatus status);

#endif
