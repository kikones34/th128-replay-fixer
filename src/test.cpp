#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "types.h"
#include "encryption.h"
#include "compression.h"
#include "th128_core.h"
#include "th128_parse.h"
#include "th128_fix.h"
#include "utils.h"


// Test that compressing and decompressing yields the same data
uint8_t *test_compress(uint8_t *data, uint32_t size) {
	uint32_t compressed_size;
	uint8_t *compressed_data = compress(data, size, compressed_size);

	uint8_t *decompressed_data = new uint8_t[size];
	uint32_t decompressed_size = decompress(compressed_data, decompressed_data, compressed_size);
	
	if (decompressed_size != size) {
		printf("Unexpected data size: got %d, expected %d.\n", decompressed_size, size);
	}

	if (memcmp(decompressed_data, data, size)) {
		printf("Data mismatch:\n");
		// for (uint32_t i = 0; i < size; i++) {
		// 	printf("0x%02x ", data[i]);
		// }
		// printf("\n");
		// for (uint32_t i = 0; i < decompressed_size; i++) {
		// 	printf("0x%02x ", decompressed_data[i]);
		// }
		write_file("comp", ".lzss", compressed_data, compressed_size);
		write_file("uncomp", ".raw", data, size);
		write_file("decomp", ".raw", decompressed_data, decompressed_size);
		printf("\n");
	}

	delete[] compressed_data;
	delete[] decompressed_data;

	return 0;
}


// Test that re-encoding a replay yields the same decoded data
void th128_encode_decode_test(const char *file) {
	// Read file
	FILE *fp = fopen(file, "rb");
	if (!fp) {
		perror("Error");
		return;
	}

	fseek(fp, 0L, SEEK_END);
	uint32_t file_size = ftell(fp);
	fseek(fp, 0L, SEEK_SET);
	
	uint8_t *file_data = new uint8_t[file_size];
	fread(file_data, file_size, 1, fp);
	fclose(fp);
	
	// Parse file header
	th128_replay_header_t *header = (th128_replay_header_t *)file_data;
	uint8_t *encoded_data = file_data + sizeof(th128_replay_header_t);
	uint32_t compressed_size = header->compressed_data_size;
	uint32_t uncompressed_size = header->uncompressed_data_size;
	uint8_t *user_data = file_data + header->user_data_offset;
	uint32_t user_data_size = file_size - sizeof(th128_replay_header_t) - compressed_size;

	// Decode original data
	uint8_t *decoded_data = th128_decode_replay_data(encoded_data, compressed_size, uncompressed_size);

	// Encode data
	uint32_t new_encoded_data_size;
	uint8_t *new_encoded_data = th128_encode_replay_data(decoded_data, uncompressed_size, new_encoded_data_size);

	// Build re-encoded file
	uint32_t new_file_size = sizeof(th128_replay_header_t) + new_encoded_data_size + user_data_size;
	uint32_t new_user_data_offset = sizeof(th128_replay_header_t) + new_encoded_data_size;
	uint8_t *new_file_data = new uint8_t[new_file_size];
	uint8_t *curr_file_ptr = new_file_data;

	memcpy(curr_file_ptr, file_data, sizeof(th128_replay_header_t));
	curr_file_ptr += sizeof(th128_replay_header_t);
	th128_replay_header_t *new_header = (th128_replay_header_t *)new_file_data;
	new_header->compressed_data_size = new_encoded_data_size;
	new_header->user_data_offset = new_user_data_offset;

	memcpy(curr_file_ptr, new_encoded_data, new_encoded_data_size);
	curr_file_ptr += new_encoded_data_size;

	memcpy(curr_file_ptr, user_data, user_data_size);
	curr_file_ptr += user_data_size;

	// Write re-encoded file
	write_file(file, ".reenc.rpy", new_file_data, new_file_size);

	// Decode encoded data
	uint8_t *new_decoded_data = th128_decode_replay_data(new_encoded_data, new_encoded_data_size, uncompressed_size);
	if (new_decoded_data == NULL) {
		delete[] file_data;
		delete[] decoded_data;
		delete[] new_encoded_data;
		delete[] new_file_data;
		return;
	}

	// Compare
	if (memcmp(decoded_data, new_decoded_data, uncompressed_size)) {
		printf("!! DATA DIFFERS !!\n");
		
		// Write both decoded data to file
		write_file(file, ".raw", decoded_data, uncompressed_size);
		write_file(file, ".reenc.raw", new_decoded_data, uncompressed_size);
	} else {
		printf("Data matches.\n");
		printf("Original compressed data size: %d\n", compressed_size);
		printf("New compressed data size:      %d\n", new_encoded_data_size);
	}

	// Clean up
	delete[] file_data;
	delete[] decoded_data;
	delete[] new_encoded_data;
	delete[] new_decoded_data;
	delete[] new_file_data;
}


bool th128_decrypt_replay_file(const char *file) {
	// Read file
	FILE *fp = fopen(file, "rb");
	if (!fp) {
		perror("Error");
		return false;
	}

	fseek(fp, 0L, SEEK_END);
	uint32_t file_size = ftell(fp);
	fseek(fp, 0L, SEEK_SET);
	
	uint8_t *file_data = new uint8_t[file_size];
	fread(file_data, file_size, 1, fp);
	fclose(fp);
	
	// Parse file header
	th128_replay_header_t *header = (th128_replay_header_t *)file_data;
	uint8_t *encoded_data = file_data + sizeof(th128_replay_header_t);
	uint32_t compressed_size = header->compressed_data_size;

	// Decrypt data
	printf("Decrypting replay data... ");
	decrypt(encoded_data, compressed_size, 0x800, 0x5e, 0xe7);
	decrypt(encoded_data, compressed_size, 0x80, 0x7d, 0x36);
	printf("done.\n");

	// Write new file
	write_file(file, ".dec", file_data, file_size);
	
	// Clean up
	delete[] file_data;

	return true;
}



const char *const files[] = {
	"replays/th128_01.rpy",
	"replays/th128_02.rpy",
	"replays/th128_03.rpy",
	// "replays/th128_03.rpy.reenc.rpy"
	"replays/th128_04.rpy",
	"replays/th128_05.rpy",
	"replays/th128_06.rpy",
	"replays/th128_07.rpy",
	"replays/th128_08.rpy",
	"replays/th128_09.rpy",
	"replays/th128_10.rpy",
	// "replays/th128_10.rpy.fixed.rpy"
	"replays/th128_11.rpy",
	"replays/th128_12.rpy",
	"replays/th128_13.rpy",
	"replays/th128_14.rpy",
	"replays/th128_15.rpy",
	"replays/th128_16.rpy",
	"replays/th128_17.rpy",
	"replays/th128_18.rpy",
	// "replays/th128_18.rpy.reenc.rpy",
	"replays/th128_19.rpy",
	"replays/th128_20.rpy",
	"replays/th128_21.rpy",
	"replays/th128_22.rpy",
	"replays/th128_23.rpy",
	"replays/th128_24.rpy",
	// "replays/th128_24.rpy.reenc.rpy",
	"replays/th128_25.rpy",
	// "replays/th128_25.rpy.reenc.rpy",
	"replays/th128_26.rpy",
	"replays/th128_27.rpy",
	"replays/th128_28.rpy",
	"replays/th128_29.rpy",
	"replays/th128_30.rpy",
	"replays/th128_31.rpy"
	// "replays/th10_01.rpy",
	// "replays/th10_05.rpy"
};


int main() {
	for (size_t i = 0; i < sizeof(files) / sizeof(*files); i++) {
		printf("Processing %s\n", files[i]);
		// th128_fix_replay_file(files[i]);
		// th128_decode_replay_file(files[i]);
		th128_encode_decode_test(files[i]);
		// th128_decrypt_replay_file(files[i]);
		// th128_parse_user_data(files[i]);
		printf("\n");
	}
	
	return 0;
}
