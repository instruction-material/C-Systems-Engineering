#include "dispatch.h"

static int valid_state(const DeviceState* state) {
    return state != NULL && state->level >= 0 && state->level <= DEVICE_CAPACITY;
}

static CommandStatus store_level(DeviceState* state, int argument) {
    if (!valid_state(state) || argument < 1 || argument > DEVICE_CAPACITY) {
        return COMMAND_BAD_ARGUMENT;
    }
    if (argument > DEVICE_CAPACITY - state->level) {
        return COMMAND_FULL;
    }
    state->level += argument;
    return COMMAND_OK;
}

static CommandStatus take_level(DeviceState* state, int argument) {
    if (!valid_state(state) || argument < 1 || argument > DEVICE_CAPACITY) {
        return COMMAND_BAD_ARGUMENT;
    }
    if (argument > state->level) {
        return COMMAND_EMPTY;
    }
    state->level -= argument;
    return COMMAND_OK;
}

static CommandStatus clear_level(DeviceState* state, int argument) {
    if (!valid_state(state) || argument != 0) {
        return COMMAND_BAD_ARGUMENT;
    }
    state->level = 0;
    return COMMAND_OK;
}

const CommandEntry builtin_commands[] = {
    {"store", store_level},
    {"take", take_level},
    {"clear", clear_level}
};
const size_t builtin_command_count = sizeof(builtin_commands) / sizeof(builtin_commands[0]);

const char* command_status_name(CommandStatus status) {
    switch (status) {
        case COMMAND_OK: return "ok";
        case COMMAND_BAD_ARGUMENT: return "bad-argument";
        case COMMAND_UNKNOWN: return "unknown";
        case COMMAND_FULL: return "full";
        case COMMAND_EMPTY: return "empty";
    }
    return "invalid-status";
}
