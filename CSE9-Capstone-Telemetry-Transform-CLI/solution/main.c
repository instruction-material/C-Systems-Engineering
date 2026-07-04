/**************************
*   CODING STANDARD   *
**************************/

// Use named constants, descriptive names, and purpose comments before nontrivial scopes

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

/****************
*   SOLUTION   *
****************/

static bool write_sample_input(const char *path) {
	FILE *file = fopen(path, "w");
	if (file == NULL) {
		return false;
	}

	fprintf(file, "sample_id,temp_milli_c,voltage_mv,flags\n");
	fprintf(file, "7,21500,3300,5\n");
	fprintf(file, "8,19875,3250,1\n");
	fprintf(file, "9,25000,3400,129\n");
	fclose(file);
	return true;
}

static bool validate_row(
	unsigned int sample_id,
	int temp_milli_c,
	unsigned int voltage_mv,
	unsigned int flags
) {
	return sample_id <= 255u
		&& temp_milli_c >= -40000
		&& temp_milli_c <= 125000
		&& voltage_mv <= 5000u
		&& flags <= 255u;
}

int main(int argc, char **argv) {
	const char *input_path = argc > 1 ? argv[1] : "telemetry-input.csv";
	const char *output_path = argc > 2 ? argv[2] : "telemetry-output.csv";
	FILE *input = NULL;
	FILE *output = NULL;
	char line[256];
	unsigned int line_number = 0;
	unsigned int transformed_rows = 0;

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
	fprintf(
		output,
		"sample_id,temp_milli_c,temp_milli_f,voltage_mv,flags_hex,status\n"
	);

	while (fgets(line, sizeof(line), input) != NULL) {
		unsigned int sample_id = 0;
		int temp_milli_c = 0;
		unsigned int voltage_mv = 0;
		unsigned int flags = 0;
		int temp_milli_f = 0;
		const char *status = "normal";

		line_number += 1;
		if (sscanf(line, "%u,%d,%u,%u", &sample_id, &temp_milli_c, &voltage_mv, &flags) != 4) {
			fprintf(stderr, "parse failure on data line %u\n", line_number);
			fclose(input);
			fclose(output);
			return 1;
		}

		if (!validate_row(sample_id, temp_milli_c, voltage_mv, flags)) {
			fprintf(stderr, "range failure on data line %u\n", line_number);
			fclose(input);
			fclose(output);
			return 1;
		}

		temp_milli_f = (temp_milli_c * 9) / 5 + 32000;
		if (temp_milli_c > 60000) {
			status = "hot";
		}
		else if (temp_milli_c < 0) {
			status = "cold";
		}

		fprintf(
			output,
			"%u,%d,%d,%u,0x%02X,%s\n",
			sample_id,
			temp_milli_c,
			temp_milli_f,
			voltage_mv,
			flags,
			status
		);
		transformed_rows += 1;
	}

	printf("capstone telemetry transform cli (solution)\n");
	printf("wrote output file : %s\n", output_path);
	printf("rows transformed  : %u\n", transformed_rows);
	printf("math note         : temperature stays in fixed-point milli-units during the transform.\n");

	fclose(input);
	fclose(output);
	return 0;
}
