#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

static bool write_sample_input(const char* path) {
    FILE* file = fopen(path, "w");
    if (file == NULL) {
        return false;
    }

    fprintf(file, "sample_id,temp_milli_c,voltage_mv,flags\n");
    fprintf(file, "7,21500,3300,5\n");
    fprintf(file, "8,19875,3250,1\n");
    fclose(file);
    return true;
}

int main(int argc, char** argv) {
    const char* input_path = argc > 1 ? argv[1] : "telemetry-input.csv";
    const char* output_path = argc > 2 ? argv[2] : "telemetry-output.csv";
    FILE* input = NULL;
    FILE* output = NULL;
    char line[256];

    if (!write_sample_input(input_path)) {
        fprintf(stderr, "failed to create sample input\n");
        return 1;
    }

    input = fopen(input_path, "r");
    output = fopen(output_path, "w");
    if (input == NULL || output == NULL) {
        fprintf(stderr, "failed to open input or output file\n");
        if (input != NULL) {
            fclose(input);
        }
        if (output != NULL) {
            fclose(output);
        }
        return 1;
    }

    fgets(line, sizeof(line), input);
    fprintf(output,
            "sample_id,temp_milli_c,temp_milli_f,voltage_mv,flags_hex\n");

    while (fgets(line, sizeof(line), input) != NULL) {
        unsigned int sample_id = 0;
        int temp_milli_c = 0;
        unsigned int voltage_mv = 0;
        unsigned int flags = 0;
        int temp_milli_f = 0;

        if (sscanf(line, "%u,%d,%u,%u", &sample_id, &temp_milli_c, &voltage_mv,
                   &flags) != 4) {
            continue;
        }

        temp_milli_f = (temp_milli_c * 9) / 5 + 32000;
        fprintf(output, "%u,%d,%d,%u,0x%02X\n", sample_id, temp_milli_c,
                temp_milli_f, voltage_mv, flags & 0xFFu);
    }

    printf("capstone telemetry transform cli (starter)\n");
    printf("wrote output file: %s\n", output_path);

    /* TODO:
	 * 1. Reject rows with out-of-range sample IDs, temperatures, or voltages.
	 * 2. Report the line number when parsing fails.
	 * 3. Add one more derived output field or summary statistic.
	 */

    fclose(input);
    fclose(output);
    return 0;
}
