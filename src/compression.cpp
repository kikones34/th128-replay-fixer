#include "compression.h"

#include <stdio.h>
#include <string.h>


/*
	ZUN LZSS format:

	uncompressed: 1|<byte (8 bits)>
	compressed:   0|<index (13 bits)>|<length (4 bits)>

	max index = 2^13 - 1 = 8191
	min length = 3
	max length = 2^4 - 1 + 3 = 18

	history index = data index % 2^13

	must finish with control bit 0 and index 0 (can be truncated)
*/


uint32_t get_bits(uint8_t *buffer, uint32_t max_bytes, uint32_t &curr_byte, uint8_t &curr_mask, int num_bits) {
	uint32_t result = 0;
	uint8_t current;
	current = buffer[curr_byte];
	for (int i = 0; i < num_bits; i++) {
		result <<= 1;
		if (current & curr_mask) {
			result |= 1;
		}
		curr_mask >>= 1;
		if (curr_mask == 0) {
			curr_byte++;
			if (curr_byte == max_bytes) {
				return result;
			}
			current = buffer[curr_byte];
			curr_mask = 0x80;
		}
	}
	return result;
}


void write_bits(uint8_t *buffer, uint32_t &curr_byte, uint8_t &curr_mask, uint32_t value, int num_bits) {
    int value_mask = 1 << (num_bits - 1);
	for (int i = 0; i < num_bits; i++) {
        if (value & value_mask) {
            buffer[curr_byte] |= curr_mask;
        }
		value_mask >>= 1;
        curr_mask >>= 1;
        if (curr_mask == 0) {
            curr_mask = 0x80;
            curr_byte++;
        }
    }
}


uint32_t decompress(uint8_t *compressed_data, uint8_t *decompressed_data, uint32_t length) {
	uint8_t history[HISTORY_SIZE];
	uint8_t curr_mask = 0x80;
	uint32_t curr_src_byte = 0;
	uint32_t curr_dst_byte = 0;
	while (curr_src_byte < length) {
		bool control_bit = get_bits(compressed_data, length, curr_src_byte, curr_mask, 1);
		if (control_bit) {
			uint8_t data_byte = get_bits(compressed_data, length, curr_src_byte, curr_mask, 8);
			decompressed_data[curr_dst_byte] = data_byte;
			history[curr_dst_byte % HISTORY_SIZE] = data_byte;
			curr_dst_byte++;
		} else {
			uint32_t history_index = get_bits(compressed_data, length, curr_src_byte, curr_mask, HISTORY_INDEX_BITS);
			// check for data terminator
			if (history_index == 0) {
				return curr_dst_byte;
			} else {
				history_index -= 1;
			}
			uint32_t data_length = get_bits(compressed_data, length, curr_src_byte, curr_mask, MATCH_LENGTH_BITS) + MIN_MATCH_LENGTH;
			for (uint32_t i = 0; i < data_length; i++) {
				history[curr_dst_byte % HISTORY_SIZE] = history[(history_index + i) % HISTORY_SIZE];
				decompressed_data[curr_dst_byte] = history[(history_index + i) % HISTORY_SIZE];
				curr_dst_byte++;
			}
		}
	}
	printf("WARNING: reached end of data but didn't find data terminator! ");
	return curr_dst_byte;
}


inline int min(int a, int b) {
	return a < b ? a : b;
}

uint8_t *compress(uint8_t *data, uint32_t size, uint32_t &compressed_data_size) {
	uint8_t history[HISTORY_SIZE];
	uint8_t curr_mask = 0x80;
	uint32_t curr_src_byte = 0;
	uint32_t curr_dst_byte = 0;

	// allocate memory for worst case scenario (none of the bytes is compressed)
	uint32_t max_compressed_size = (uint32_t)(size * 1.125) + 2;
	uint8_t *compressed_data = new uint8_t[max_compressed_size];
	memset(compressed_data, 0, max_compressed_size);
	
	while (curr_src_byte < size) {
		// find longest match in history
		uint32_t longest_match_index = 0;
		uint32_t longest_match_length = 0;
		const int curr_history_size = min(curr_src_byte, HISTORY_SIZE);
		const int start_index = curr_src_byte - 1;
		for (int curr_index = start_index; curr_index > start_index - curr_history_size; curr_index--) {
			// we cannot store this index since it would be confused with the data terminator
			if (curr_index % HISTORY_SIZE == HISTORY_SIZE - 1) {
				continue;
			}
			// try to match as many bytes as possible from current index
			uint32_t curr_offset = 0;
			while (
				// compare history data with source data
				// if we go past the newest history index, read from source data instead
				// (history will be updated at the end after the best match is found)
				(
					curr_index + curr_offset < curr_src_byte ?
					history[(curr_index + curr_offset) % HISTORY_SIZE] :
					data[curr_index + curr_offset]
				) == data[curr_src_byte + curr_offset]
				// don't go over max match length
				&& curr_offset < MAX_MATCH_LENGTH
				// don't go over data buffer
				&& curr_src_byte + curr_offset < size
			) {
				curr_offset++;
			}
			// update best match
			if (curr_offset > longest_match_length) {
				longest_match_length = curr_offset;
				longest_match_index = curr_index % HISTORY_SIZE;
			}
			// if best match is max length, we're done
			if (longest_match_length == MAX_MATCH_LENGTH) {
				break;
			}
		}

		if (longest_match_length < 3) {
			// do not compress
			write_bits(compressed_data, curr_dst_byte, curr_mask, 1, 1);
			write_bits(compressed_data, curr_dst_byte, curr_mask, data[curr_src_byte], 8);
			history[curr_src_byte % HISTORY_SIZE] = data[curr_src_byte];
			curr_src_byte++;
		} else {
			// compress
			write_bits(compressed_data, curr_dst_byte, curr_mask, 0, 1);
			write_bits(compressed_data, curr_dst_byte, curr_mask, longest_match_index + 1, HISTORY_INDEX_BITS);
			write_bits(compressed_data, curr_dst_byte, curr_mask, longest_match_length - MIN_MATCH_LENGTH, MATCH_LENGTH_BITS);
			for (uint32_t i = 0; i < longest_match_length; i++) {
				history[curr_src_byte % HISTORY_SIZE] = data[curr_src_byte];
				curr_src_byte++;
			}
		}
	}

	// write data terminator (technically not needed, but just to be safe)
	write_bits(compressed_data, curr_dst_byte, curr_mask, 0, 2);
	
	compressed_data_size = curr_dst_byte + 1;

	return compressed_data;
}
