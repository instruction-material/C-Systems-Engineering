#include "configuration.h"
#include "input.h"
#include <limits.h>
#include <stdio.h>

static void print_binary8(uint8_t value) {
    for (int bit = 7; bit >= 0; --bit) {
        putchar((((uint32_t)value >> (unsigned int)bit) & 1u) ? '1' : '0');
        if (bit == 4) putchar('_');
    }
}

int main(int argc, char** argv) {
    uint8_t value = 0x27u;
    if (argc > 2) {
        fprintf(stderr, "Usage: %s [decimal-byte | 0xHEX-byte]\n", argv[0]);
        return 2;
    }
    if (argc == 2 && !parse_config_input(argv[1], &value)) {
        fputs("Input must be complete decimal or 0x/0X hexadecimal text in 0..255.\n", stderr);
        return 2;
    }
    int flags[4];
    for (unsigned int bit = 0; bit < 4u; ++bit) {
        flags[bit] = config_flag_enabled(value, bit);
        if (flags[bit] < 0 || flags[bit] > 1) {
            fputs("Unfinished configuration task: implement the three named functions.\n", stderr);
            return 78;
        }
    }
    const unsigned int mode = config_read_mode(value);
    uint8_t logging_off = value;
    uint8_t logging_on = value;
    if (mode > 3u || !config_with_flag(value, 0u, false, &logging_off)
        || !config_with_flag(value, 0u, true, &logging_on)) {
        fputs("Unfinished configuration task: implement the three named functions.\n", stderr);
        return 78;
    }
    static const char* const modes[] = {"safe", "balanced", "fast", "diagnostic"};
    const unsigned int reserved = (unsigned int)value & 0xC0u;
    puts("bitflag configuration parser");
    printf("config byte : 0x%02X\n", (unsigned int)value);
    fputs("binary      : ", stdout);
    print_binary8(value);
    putchar('\n');
    printf("logging     : %s\n", flags[0] ? "on" : "off");
    printf("checksum    : %s\n", flags[1] ? "enabled" : "disabled");
    printf("ack         : %s\n", flags[2] ? "required" : "optional");
    printf("encryption  : %s (flag only)\n", flags[3] ? "set" : "clear");
    printf("mode        : %s (%u)\n", modes[mode], mode);
    printf("reserved    : 0x%02X\n", reserved);
    printf("logging off : 0x%02X\n", (unsigned int)logging_off);
    printf("logging on  : 0x%02X\n", (unsigned int)logging_on);
    if (reserved != 0u) {
        puts("warning     : reserved bits are set; no behavior is assigned to them.");
    }
    return 0;
}
