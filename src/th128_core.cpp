#include "th128_core.h"

#include <stdio.h>

#include "encryption.h"
#include "compression.h"


/*
	Data is compressed using LZSS, then encrypted twice with a custom ZUN function.
*/ 


uint8_t *th128_decode_replay_data(uint8_t *encoded_data, uint32_t compressed_size, uint32_t uncompressed_size) {
	printf("Decrypting replay data... ");
	decrypt(encoded_data, compressed_size, 0x800, 0x5e, 0xe7);
	decrypt(encoded_data, compressed_size, 0x80, 0x7d, 0x36);
	printf("done.\n");

	printf("Decompressing replay data... ");
	uint8_t *decoded_data = new uint8_t[uncompressed_size];
	uint32_t decompressed_size = decompress(encoded_data, decoded_data, compressed_size);
	if (decompressed_size != uncompressed_size) {
		printf("error: got %d bytes but expected %d.\n", decompressed_size, uncompressed_size);
		return NULL;
	}
	printf("done.\n");
	
	return decoded_data;
}


uint8_t *th128_encode_replay_data(uint8_t *data, uint32_t size, uint32_t &compressed_size) {
	printf("Compressing replay data... ");
	uint8_t *compressed_data = compress(data, size, compressed_size);
	printf("done.\n");
	
	printf("Encrypting replay data... ");
	encrypt(compressed_data, compressed_size, 0x80, 0x7d, 0x36);
	encrypt(compressed_data, compressed_size, 0x800, 0x5e, 0xe7);
	printf("done.\n");

	return compressed_data;
}
