#pragma once

#include <stdint.h>
#include <stdio.h>


void write_file(const char *path, const char *suffix, uint8_t *data, size_t data_length);
void append_utf8_char(uint8_t *string, size_t &idx, const uint8_t *utf8_char);
void print_binary_array(FILE *stream, uint8_t *array, size_t length);
