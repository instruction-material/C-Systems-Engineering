#include "dispatch.h"
#include <limits.h>
#include <stdio.h>

static int passed;
static int failed;
static int callback_calls;
static int captured_argument;
static int trap_calls;

static void check_condition(const char* name, int condition) {
    if (condition) {
        ++passed;
    } else {
        ++failed;
        fprintf(stderr, "failed: %s\n", name);
    }
}

static void check(const char* name, CommandStatus actual, CommandStatus expected,
                  int level, int expected_level) {
    check_condition(name, actual == expected && level == expected_level);
    if (actual != expected || level != expected_level) {
        fprintf(stderr, "  status=%s expected=%s level=%d expected=%d\n",
                command_status_name(actual), command_status_name(expected),
                level, expected_level);
    }
}

static CommandStatus observe(DeviceState* state, int argument) {
    ++callback_calls;
    captured_argument = argument;
    state->level = argument;
    return COMMAND_OK;
}

static CommandStatus trap(DeviceState* state, int argument) {
    (void)state;
    (void)argument;
    ++trap_calls;
    return COMMAND_EMPTY;
}

int main(void) {
    const struct {
        const char* command;
        int argument;
        CommandStatus status;
        int level;
    } fixtures[] = {
        {"store", 4, COMMAND_OK, 4},
        {"take", 2, COMMAND_OK, 2},
        {"store", 99, COMMAND_FULL, 2},
        {"clear", 0, COMMAND_OK, 0},
        {"store", 100, COMMAND_OK, 100},
        {"store", 1, COMMAND_FULL, 100},
        {"take", 100, COMMAND_OK, 0},
        {"take", 1, COMMAND_EMPTY, 0},
        {"store", 0, COMMAND_BAD_ARGUMENT, 0},
        {"store", -1, COMMAND_BAD_ARGUMENT, 0},
        {"store", 101, COMMAND_BAD_ARGUMENT, 0},
        {"store", INT_MAX, COMMAND_BAD_ARGUMENT, 0},
        {"take", 0, COMMAND_BAD_ARGUMENT, 0},
        {"take", INT_MIN, COMMAND_BAD_ARGUMENT, 0},
        {"clear", 1, COMMAND_BAD_ARGUMENT, 0},
        {"storex", 0, COMMAND_UNKNOWN, 0},
        {"STORE", 4, COMMAND_UNKNOWN, 0},
        {"take", 101, COMMAND_BAD_ARGUMENT, 0}
    };
    DeviceState state = {0};
    for (size_t i = 0; i < sizeof(fixtures) / sizeof(fixtures[0]); ++i) {
        CommandStatus result = dispatch_command(&state, builtin_commands,
            builtin_command_count, fixtures[i].command, fixtures[i].argument);
        check(fixtures[i].command, result, fixtures[i].status,
              state.level, fixtures[i].level);
    }

    const CommandEntry custom[] = {{"observe", observe}};
    CommandStatus result = dispatch_command(&state, custom, 1, "observe", 17);
    check("caller-supplied table", result, COMMAND_OK, state.level, 17);
    check_condition("callback receives unchanged argument",
                    callback_calls == 1 && captured_argument == 17);

    const CommandEntry duplicate[] = {{"observe", observe}, {"observe", trap}};
    result = dispatch_command(&state, duplicate, 2, "observe", 19);
    check("first duplicate name wins", result, COMMAND_OK, state.level, 19);
    check_condition("exactly one selected callback", callback_calls == 2 && trap_calls == 0);

    result = dispatch_command(&state, NULL, 0, "observe", 0);
    check("empty table", result, COMMAND_UNKNOWN, state.level, 19);
    result = dispatch_command(NULL, custom, 1, "observe", 0);
    check("null state", result, COMMAND_BAD_ARGUMENT, state.level, 19);
    result = dispatch_command(&state, custom, 1, NULL, 0);
    check("null command", result, COMMAND_BAD_ARGUMENT, state.level, 19);
    result = dispatch_command(&state, NULL, 1, "observe", 0);
    check("missing nonempty table", result, COMMAND_BAD_ARGUMENT, state.level, 19);
    const CommandEntry missing_name[] = {{NULL, observe}};
    result = dispatch_command(&state, missing_name, 1, "observe", 0);
    check("missing entry name", result, COMMAND_BAD_ARGUMENT, state.level, 19);
    const CommandEntry missing_handler[] = {{"observe", NULL}};
    result = dispatch_command(&state, missing_handler, 1, "observe", 0);
    check("missing entry handler", result, COMMAND_BAD_ARGUMENT, state.level, 19);
    const CommandEntry malformed_later[] = {{"observe", observe}, {"other", NULL}};
    result = dispatch_command(&state, malformed_later, 2, "observe", 0);
    check("validate whole table before calling", result, COMMAND_BAD_ARGUMENT, state.level, 19);
    state.level = -1;
    result = dispatch_command(&state, custom, 1, "observe", 0);
    check("state below range", result, COMMAND_BAD_ARGUMENT, state.level, -1);
    state.level = 101;
    result = dispatch_command(&state, custom, 1, "observe", 0);
    check("state above range", result, COMMAND_BAD_ARGUMENT, state.level, 101);
    if (failed) {
        fprintf(stderr, "%d dispatch checks failed; %d passed\n", failed, passed);
        return 1;
    }
    printf("%d dispatch checks passed\n", passed);
    return 0;
}
