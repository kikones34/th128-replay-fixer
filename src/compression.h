#pragma once

#include <stdint.h>

// ZUN's LZSS parameters
const uint32_t HISTORY_INDEX_BITS = 13;
const uint32_t HISTORY_SIZE = 1 << HISTORY_INDEX_BITS;
const uint32_t MATCH_LENGTH_BITS = 4;
const uint32_t MIN_MATCH_LENGTH = 3;
const uint32_t MAX_MATCH_LENGTH = MIN_MATCH_LENGTH + (1 << MATCH_LENGTH_BITS) - 1;


uint32_t decompress(uint8_t *compressed_data, uint8_t *decompressed_data, uint32_t length);
uint8_t *compress(uint8_t *data, uint32_t size, uint32_t &compressed_data_size);
