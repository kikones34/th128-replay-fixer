#include "th128_parse.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#include "types.h"
#include "th128_core.h"
#include "encryption.h"
#include "utils.h"


const uint8_t *get_action_char(th128_input_data_t input_data) {
	// end of input, used when replay ends abruptly (save and quit)
	if (input_data.holding.raw == 0xffff) {
		return char_codes::quit;
	}
	
	// print non-movement actions following some reasonable priority
	if (input_data.pressed.bits.bomb) {
		return char_codes::bomb;
	}

	if (input_data.pressed.bits.autoshoot) {
		return char_codes::autoshoot_press;
	}
	if (input_data.released.bits.autoshoot) {
		return char_codes::autoshoot_release;
	}

	if (input_data.pressed.bits.focus) {
		return char_codes::focus_press;
	}
	if (input_data.released.bits.focus) {
		return char_codes::focus_release;
	}

	if (input_data.pressed.bits.shoot) {
		return char_codes::shoot_press;
	}
	if (input_data.released.bits.shoot) {
		return char_codes::shoot_release;
	}

	// if only moving, print corresponding movement character
	return char_codes::movement[(input_data.holding.raw >> 4) & 0xf];
}


/*
	Decoded data structure:
	th128_replay_data_t   (0x70 bytes)
	th128_stage_header_t  (0x90 bytes)
	<stage data>          (variable bytes)
	th128_stage_header_t  (0x90 bytes)
	<stage data>          (variable bytes)
	...

	number of stages found at offset 0x20 of th128_replay_data_t
	size of stage data found at offset 0x08 of th128_stage_header_t

	Stage data structure:
	<actions data> (6 bytes per frame, total size = 6 * num_frames)
	<fps data>     (remaining bytes, contains fps info, length appears to be ceil(num_frames / 2))
*/
void th128_parse_replay_data(uint8_t *decoded_data, const char *out_file) {
	printf("Parsing replay data... ");

	// Parse replay data
	th128_replay_data_t *replay_data = (th128_replay_data_t*)decoded_data;
	uint32_t num_stages = replay_data->num_stages;

	th128_stage_header_t *stage_data[3];
	uint8_t *curr_offset = decoded_data + sizeof(th128_replay_data_t);
	for (uint32_t i = 0; i < num_stages; i++) {
		stage_data[i] = (th128_stage_header_t *)curr_offset;
		curr_offset += sizeof(th128_stage_header_t) + stage_data[i]->size;
	}

	// Write data to file
	FILE *fp = fopen(out_file, "wb");
	
	/* Replay metadata */
	fprintf(fp, "Replay data\n");
	fprintf(fp, "-----------\n");
	fprintf(fp, "Name: %.*s\n", 8, replay_data->name);
	
	// Format timestamp
	time_t time = replay_data->date;
	tm *utc_time = gmtime(&time);
	char date_str[256];
	strftime(date_str, sizeof(date_str), "%Y-%m-%d %H:%M:%S (UTC)", utc_time);
	fprintf(fp, "Date: %s\n", date_str);
	
	fprintf(fp, "Score: %u\n", replay_data->score * 10);
	fprintf(fp, "Slowdown rate: %.2f%%\n", replay_data->slowdown);
	fprintf(fp, "Number of stages: %u\n", replay_data->num_stages);

	// Handle route bug
	fprintf(fp, "Route: ");
	uint32_t route = replay_data->route;
	if (route > 6) {
		fprintf(fp, "Extra+%u (bugged)", route - 6);
	} else {
		fprintf(fp, routes[route]);
	}
	fprintf(fp, "\n");

	fprintf(fp, "Rank: %s\n", ranks[replay_data->rank]);
	// No need to handle the route bug here, as attempting to save a replay with a last_stage > 0x17 crashes the game
	fprintf(fp, "Last stage: %s\n", stages[replay_data->last_stage]);
	fprintf(fp, "\n\n");

	// Game config
	fprintf(fp, "Game settings\n");
	fprintf(fp, "-------------\n");
	fprintf(fp, "Shot button: %02hu\n", replay_data->game_config.shot_button);
	fprintf(fp, "Bomb button: %02hu\n", replay_data->game_config.bomb_button);
	fprintf(fp, "Rapid button: %02hu\n", replay_data->game_config.rapid_button);
	fprintf(fp, "Slow button: %02hu\n", replay_data->game_config.slow_button);
	fprintf(fp, "Pause button: %02hu\n", replay_data->game_config.pause_button);
	fprintf(fp, "Joypad sensitivity: %hu\n", replay_data->game_config.joypad_sensitivity);
	fprintf(fp, "Color mode: %s\n", color_modes[replay_data->game_config.color_mode]);
	fprintf(fp, "Screen mode: %s\n", screen_modes[replay_data->game_config.screen_mode]);
	fprintf(fp, "Frameskip: %s\n", frameskip_modes[replay_data->game_config.frameskip]);
	fprintf(fp, "BGM volume: %hu%%\n", replay_data->game_config.bgm_vol);
	fprintf(fp, "SFX volume: %hu%%\n", replay_data->game_config.sfx_vol);
	fprintf(fp, "Input latency: %s\n", input_latency_modes[replay_data->game_config.input_latency]);	
	fprintf(fp, "Initial window X position: %u\n", replay_data->game_config.window_x);
	fprintf(fp, "Initial window Y position: %u\n", replay_data->game_config.window_y);
	fprintf(fp, "Always ask screen mode: %s\n", replay_data->game_config.flags.always_ask ? "enabled" : "disabled");
	fprintf(fp, "Focus when firing: %s\n", replay_data->game_config.flags.focus_when_firing ? "enabled" : "disabled");
	fprintf(fp, "Don't use DirectInput: %s\n", replay_data->game_config.flags.not_direct_input ? "enabled" : "disabled");
	fprintf(fp, "\n\n");

	/* Stage data */
	for (uint32_t i = 0; i < num_stages; i++) {
		fprintf(fp, "Stage %u data\n", i + 1);
		fprintf(fp, "------------\n");

		fprintf(fp, "Stage: %s\n", stages[stage_data[i]->stage]);
		fprintf(fp, "RNG seed: %hu\n", stage_data[i]->seed);
		fprintf(fp, "Number of frames: %u\n", stage_data[i]->num_frames);
		fprintf(fp, "Initial score: %u\n", stage_data[i]->score * 10);
		fprintf(fp, "Level: %u\n", stage_data[i]->shot_power + 1);
		fprintf(fp, "X position: %.2f\n", stage_data[i]->pos_x / 128.0f);
		fprintf(fp, "Y position: %.2f\n", stage_data[i]->pos_y / 128.0f);
		fprintf(fp, "Continues used: %d\n", stage_data[i]->continues);
		fprintf(fp, "Graze: %d\n", stage_data[i]->graze);
		fprintf(fp, "Motivation: %.2f%%\n", stage_data[i]->motivation / 100.0f);
		fprintf(fp, "Perfect freeze: %.2f%%\n", stage_data[i]->perfect_freeze / 100.0f);
		fprintf(fp, "Freeze area: %.2f%%\n", stage_data[i]->freeze_area);

		fprintf(fp, "\n= Time =   ========================= Actions ==========================\n");
		th128_input_data_t *input_data = (th128_input_data_t *)(((uint8_t *)stage_data[i]) + sizeof(th128_stage_header_t));
		uint8_t actions_str[241] = {0};
		size_t actions_str_idx = 0;
		for (uint32_t j = 0; j < stage_data[i]->num_frames; j++) {
			if (j % 60 == 0) {
				fprintf(fp, "[%06d]   ", j / 60);
			}
			const uint8_t *action_char = get_action_char(input_data[j]);
			append_utf8_char(actions_str, actions_str_idx, action_char);
			if (j % 60 == 59) {
				fprintf(fp, "%s\n", actions_str);
				memset(actions_str, 0, sizeof(actions_str));
				actions_str_idx = 0;
			}
		}
		if (stage_data[i]->num_frames % 60) {
			fprintf(fp, "%s\n", (char *)actions_str);
		}

		fprintf(fp, "\nFPS data:\n");
		uint8_t *fps_data = ((uint8_t *)input_data) + stage_data[i]->num_frames * 6;
		uint32_t fps_data_size = stage_data[i]->size - stage_data[i]->num_frames * 6;
		for (uint32_t j = 0; j < fps_data_size; j++) {
			fprintf(fp, "%02d", fps_data[j]);
			if (j % 24 == 23) {
				fprintf(fp, "\n");
			} else {
				fprintf(fp, " ");
			}
		}
		fprintf(fp, "\n\n\n");
	}
	fclose(fp);

	printf("done.\n");
}


/*
	File structure:
	th128_replay_header_t (0x24 bytes)
	<encoded data>        (variable bytes)
	<user data section>   (variable bytes)

	size of encoded data found at offset 0x1c of th128_replay_header_t
*/
bool th128_decode_replay_file(const char *file) {
	// Read file
	FILE *fp = fopen(file, "rb");
	if (!fp) {
		perror("Error");
		return false;
	}

	fseek(fp, 0L, SEEK_END);
	uint32_t file_size = ftell(fp);
	fseek(fp, 0L, SEEK_SET);
	
	uint8_t *file_data = new unsigned char[file_size];
	fread(file_data, file_size, 1, fp);
	fclose(fp);
	
	// Parse file header
	th128_replay_header_t *header = (th128_replay_header_t *)file_data;
	uint8_t *encoded_data = file_data + sizeof(th128_replay_header_t);
	uint32_t compressed_size = header->compressed_data_size;
	uint32_t uncompressed_size = header->uncompressed_data_size;

	// Decode data and save it to RAW file
	uint8_t *decoded_data = th128_decode_replay_data(encoded_data, compressed_size, uncompressed_size);
	if (decoded_data == NULL) {
		delete[] file_data;
		return false;
	}
	write_file(file, ".raw", decoded_data, uncompressed_size);

	// Parse decoded data and write to TXT file (UTF-8)
	char out_file[256];
	sprintf(out_file, "%s.txt", file);
	th128_parse_replay_data(decoded_data, out_file);

	// Clean up
	delete[] file_data;
	delete[] decoded_data;

	return true;
}


/*
	User data structure:

	USER	magic string
	uint64	size of replay info section (incl. header)
	<replay info section> (Shift JIS string)
	USER	magic string
	uint64	size of replay comment section (incl. header)
	<replay comment section> (Shift JIS string)

*/
bool th128_parse_user_data(const char *file) {
	// Open file
	FILE *fp = fopen(file, "rb");
	if (!fp) {
		perror("Error");
		return false;
	}
	printf("Parsing user data... ");

	// Seek to user data section
	fseek(fp, offsetof(th128_replay_header_t, user_data_offset), SEEK_SET);
	uint32_t user_data_offset = 34;
	fread(&user_data_offset, sizeof(uint32_t), 1, fp);
	fseek(fp, user_data_offset, SEEK_SET);

	// Check magic string
	char user_str[5];
	fscanf(fp, "%4s", user_str);
	if (strcmp(user_str, "USER")) {
		printf("ERROR: unexpected magic string at user data (offset = %u)\n", user_data_offset);
		return false;
	}
	
	// Read replay info
	uint64_t replay_info_size;
	fread(&replay_info_size, sizeof(uint64_t), 1, fp);
	replay_info_size -= 0xc;  // remove magic and size
	char *replay_info = new char[replay_info_size];
	fread(replay_info, 1, replay_info_size, fp);

	// Check magic string again
	fscanf(fp, "%4s", user_str);
	if (strcmp(user_str, "USER")) {
		printf("ERROR: unexpected magic string at user comment (offset = %u)\n", user_data_offset);
		delete[] replay_info;
		return false;
	}

	// Read user comment
	uint64_t comment_size;
	fread(&comment_size, sizeof(uint64_t), 1, fp);
	comment_size -= 0xc;  // remove magic and size
	char *user_comment = new char[comment_size];
	fread(user_comment, 1, comment_size, fp);
	
	// Write file (Shift JIS encoded)
	char file_user[256];
	sprintf(file_user, "%s.user.txt", file);
	FILE *fp_user = fopen(file_user, "wb");
	fputs(replay_info, fp_user);
	fputs(user_comment, fp_user);
	fclose(fp_user);

	// Clean up
	delete[] replay_info;
	delete[] user_comment;

	printf("done.\n");
	return true;
}
