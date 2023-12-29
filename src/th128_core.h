#pragma once

#include <stdint.h>


uint8_t *th128_decode_replay_data(uint8_t *encoded_data, uint32_t compressed_size, uint32_t uncompressed_size);
uint8_t *th128_encode_replay_data(uint8_t *data, uint32_t size, uint32_t &compressed_size);
