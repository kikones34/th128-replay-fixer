#include "utils.h"

#include <string.h>


void write_file(const char *path, const char *suffix, uint8_t *data, size_t data_length) {
    size_t new_path_len = strlen(path) + strlen(suffix);
    char *new_path = new char[new_path_len + 1];
    sprintf(new_path, "%s%s", path, suffix);

    FILE *fp = fopen(new_path, "wb");
    fwrite(data, 1, data_length, fp);
    fclose(fp);

    delete[] new_path;
}


void append_utf8_char(uint8_t *string, size_t &idx, const uint8_t *utf8_char) {
	uint8_t first_byte = *utf8_char;
	int num_bytes;
	if (first_byte < 0x80) {
		num_bytes = 1;
	} else if (first_byte < 0xc0) {
		num_bytes = -1;
	} else if (first_byte < 0xe0) {
		num_bytes = 2;
	} else if (first_byte < 0xf0) {
		num_bytes = 3;
	} else if (first_byte < 0xf8) {
		num_bytes = 4;
	} else {
		num_bytes = -1;
	}

	if (num_bytes == -1) {
		printf("Error: invalid UTF-8 first byte: 0x%02x\n", first_byte);
		return;
	}

	for (int i = 1; i < num_bytes; i++) {
		if (utf8_char[i] < 0x80 || utf8_char[i] >= 0xc0) {
			printf("Error: invalid UTF-8 continuation byte: 0x%02x\n", utf8_char[i]);
			return;
		}
	}
	
	for (int i = 0; i < num_bytes; i++) {
		string[idx] = utf8_char[i];
		idx++;
	}
}


void print_binary_array(FILE *stream, uint8_t *array, size_t length) {
	for (size_t i = 0; i < length; i++) {
		uint8_t mask = 0x80;
		do {
			fprintf(stream, "%d", (array[i] & mask) != 0);
		} while (mask >>= 1);
		fprintf(stream, " ");
	}
}
