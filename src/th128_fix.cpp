#include "th128_fix.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "types.h"
#include "th128_core.h"
#include "utils.h"


// Transforms a stage to its route. If unrecognized, returns -1.
int stage2route(uint32_t stage) {
	if (stage < 1) {
		return -1;
	} else if (stage <= 0x3) {
		return 0;
	} else if (stage <= 0x5) {
		return 1;
	} else if (stage <= 0x8) {
		return 2;
	} else if (stage <= 0xa) {
		return 3;
	} else if (stage <= 0xd) {
		return 4;
	} else if (stage <= 0xf) {
		return 5;
	} else if (stage <= 0x10) {
		return 6;
	} else if (stage <= 0x17) {
		return stage - 0x11;
	} else {
		return -1;
	}
}

/*
Bug explanation:
Every time you choose the second route, the route value increments by one.
This value is not reset when you reset the run. As such, the route value can keep incrementing infinitely.
A wrong route value will make most replays crash when trying to watch them.
Luckily, the stage values are correct so the proper route can be inferred from them.

Return: correct route, -1 if error, -2 if route is already correct
*/
int find_correct_route(uint8_t *decoded_data) {
	th128_replay_data_t *replay_data = (th128_replay_data_t *)decoded_data;
	
	printf("Replay route: ");
	if (replay_data->route > 6) {
		printf("Extra+%u (bugged)", replay_data->route - 6);
	} else {
		printf(routes[replay_data->route]);
	}
	printf("\n");

	// Find the correct route
	int correct_route;
	th128_stage_header_t *first_stage = (th128_stage_header_t *)(decoded_data + sizeof(th128_replay_data_t));

	if (replay_data->num_stages == 1) {
		// If there's only one stage, grab the route from it
		correct_route = stage2route(first_stage->stage);
	} else {
		// Otherwise, grab the route from the second stage
		th128_stage_header_t *second_stage = (th128_stage_header_t *)(((uint8_t *)first_stage) + sizeof(th128_stage_header_t) + first_stage->size);
		correct_route = stage2route(second_stage->stage);
	}

	if (correct_route == -1) {
		printf("Error finding the correct route.\n");
		return -1;
	} else if (replay_data->route == (uint32_t)correct_route) {
		printf("Replay already has the correct route, no need to fix.\n");
		return -2;
	} else {
		printf("Correct route: %s\n", routes[correct_route]);
		return correct_route;
	}
}


void th128_fix_replay_data(uint8_t *decoded_data, uint32_t new_route) {
	th128_replay_data_t *replay_data = (th128_replay_data_t *)decoded_data;

	// Fix replay route
	replay_data->route = new_route;

	// Fix replay last stage (only necessary if All Clear)
	if (replay_data->last_stage >= 0x11) {
		replay_data->last_stage = new_route + 0x11;
	}
}


void th128_fix_user_data(uint8_t *user_data, uint32_t new_route) {
	char *replay_info = (char *)(user_data + sizeof(th128_user_data_header_t));
	char *route_str = strstr(replay_info, "Route");
	route_str += 6; // skip over "Route "
	memcpy(route_str, route_strs[new_route], 8); // route strings are always 8 chars long
}


bool th128_fix_replay_file(const char *file) {
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
	if (header->magic != 0x72383231) {
		printf("Not a th128 replay.\n");
		delete[] file_data;
		return false;
	}
	uint8_t *encoded_data = file_data + sizeof(th128_replay_header_t);
	uint32_t compressed_size = header->compressed_data_size;
	uint32_t uncompressed_size = header->uncompressed_data_size;
	uint8_t *user_data = file_data + header->user_data_offset;
	uint32_t user_data_size = file_size - sizeof(th128_replay_header_t) - compressed_size;

	// Decode data
	uint8_t *decoded_data = th128_decode_replay_data(encoded_data, compressed_size, uncompressed_size);
	if (decoded_data == NULL) {
		delete[] file_data;
		return false;
	}

	// Check route info, abort if correct
	int correct_route = find_correct_route(decoded_data);
	if (correct_route < 0) {
		delete[] file_data;
		delete[] decoded_data;
		return false;
	}

	// Fix route in replay and user data
	th128_fix_replay_data(decoded_data, correct_route);
	th128_fix_user_data(user_data, correct_route);
	printf("Fixed replay route.\n");

	// Encode data
	uint32_t new_encoded_data_size;
	uint8_t *new_encoded_data = th128_encode_replay_data(decoded_data, uncompressed_size, new_encoded_data_size);

	// Build new file
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

	// Write new file
	write_file(file, ".fixed.rpy", new_file_data, new_file_size);
	
	// Clean up
	delete[] file_data;
	delete[] decoded_data;
	delete[] new_encoded_data;
	delete[] new_file_data;

	return true;
}
