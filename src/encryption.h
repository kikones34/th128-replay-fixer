#pragma once

#include "stdint.h"

void decrypt(uint8_t *data, int length, int block_size, uint8_t base_mask, uint8_t mask_inc);
void encrypt(uint8_t *data, int length, int block_size, uint8_t base_mask, uint8_t mask_inc);
