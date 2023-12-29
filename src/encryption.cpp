#include "encryption.h"

#include <string.h>


/*
	Within a block, bytes are decoded in order, but they're placed in reverse order and interleaved in two halves.
	
	For the original bytes:
	0 1 2 3 4 5 6 7

	They're placed in the following order:
	7 3 6 2 5 1 4 0

	Each time a byte is decoded, the mask is incremented, so decoding order is important.
*/
void decrypt(uint8_t *data, int length, int block_size, uint8_t base_mask, uint8_t mask_inc) {
	uint8_t *encoded_data = new uint8_t[length];
	memcpy(encoded_data, data, length);
	
	int bytes_left = length;
	
	// if the last block is less than 1/4 of the block size, ignore it
	if ((bytes_left % block_size) < (block_size / 4)) {
		bytes_left -= bytes_left % block_size;
	}
	
	// if number of bytes is odd, ignore last byte
	bytes_left -= length & 1;
	
	int curr_byte = 0;
	uint8_t curr_mask = base_mask;
	while (bytes_left) {
		if (bytes_left < block_size) {
			block_size = bytes_left;
		}
		
		int tp1 = curr_byte + block_size - 1;
		int tp2 = curr_byte + block_size - 2;
		
		// same as /2 and round up
		int hf = (block_size + (block_size & 0x1)) / 2;
		for (int i = 0; i < hf; i++) {
			data[tp1] = encoded_data[curr_byte] ^ curr_mask;
			curr_mask += mask_inc;
			tp1 -= 2;
			curr_byte++;
		}
		
		// same as /2 and round down
		hf = block_size / 2;
		for (int i = 0; i < hf; i++) {
			data[tp2] = encoded_data[curr_byte] ^ curr_mask;
			curr_mask += mask_inc;
			tp2 -= 2;
			curr_byte++;
		}
		
		bytes_left -= block_size;
	}
	delete[] encoded_data;
}


// Same as decrypt, but buffer indices are reversed
void encrypt(uint8_t *data, int length, int block_size, uint8_t base_mask, uint8_t mask_inc) {
	uint8_t *decoded_data = new uint8_t[length];
	memcpy(decoded_data, data, length);
	
	int bytes_left = length;
	
	// if the last block is less than 1/4 of the block size, ignore it
	if ((bytes_left % block_size) < (block_size / 4)) {
		bytes_left -= bytes_left % block_size;
	}
	
	// if number of bytes is odd, ignore last byte
	bytes_left -= length & 1;
	
	int curr_byte = 0;
	uint8_t curr_mask = base_mask;
	while (bytes_left) {
		if (bytes_left < block_size) {
			block_size = bytes_left;
		}

		int tp1 = curr_byte + block_size - 1;
		int tp2 = curr_byte + block_size - 2;
		
		// same as /2 and round up
		int hf = (block_size + (block_size & 0x1)) / 2;
		for (int i = 0; i < hf; i++) {
			data[curr_byte] = decoded_data[tp1] ^ curr_mask;
			curr_mask += mask_inc;
			tp1 -= 2;
			curr_byte++;
		}
		
		// same as /2 and round down
		hf = block_size / 2;
		for (int i = 0; i < hf; i++) {
			data[curr_byte] = decoded_data[tp2] ^ curr_mask;
			curr_mask += mask_inc;
			tp2 -= 2;
			curr_byte++;
		}
		
		bytes_left -= block_size;
	}
	delete[] decoded_data;
}
