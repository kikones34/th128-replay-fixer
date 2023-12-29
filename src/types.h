#pragma once

#include <stdint.h>


// Prevent struct padding
#pragma pack(push, 1)

// Size: 0x24
struct th128_replay_header_t {
	uint32_t magic; // '128r' for th12.8
	uint32_t rpy_version; // 2 for latest th12.8
	uint32_t unused; // seems to always be 0
	uint32_t user_data_offset; // offset of user data section
	uint32_t game_version; // usually in hex, e.g. 0.02 = 0x002, 1.00 = 0x100
	uint32_t unused2[2]; // seems to always be 0
	uint32_t compressed_data_size;
	uint32_t uncompressed_data_size;
};


// Size: 0x3c
struct th128_game_config_t {
	uint32_t version; // 01 80 12 00 for th128
	uint16_t shot_button;
	uint16_t bomb_button;
	uint16_t rapid_button;
	uint16_t slow_button;
	uint16_t pause_button;
	uint32_t unused[2]; // all bits always set to 1
	uint16_t joypad_sensitivity;
	uint16_t joypad_sensitivity_dup; // duplicate of previous 2 byes
	uint8_t color_mode; // 32-bit, 16-bit
	uint8_t unknown2; // always 0x1
	uint8_t unknown3; // always 0x1
	uint8_t screen_mode; // full, 640x480, 960x720, 1280x960
	uint8_t frameskip; // none, 1/2, 1/3
	uint8_t unknown4; // always 0x2
	uint8_t bgm_vol;
	uint8_t sfx_vol;
	uint8_t unknown5; // always 0x0
	uint8_t input_latency; // stable, normal, auto, fast
	uint32_t unused2; // seems to always be 0
	uint32_t window_x; // X position of the game window
	uint32_t window_y; // Y position of the game window
	uint32_t unused3[2]; // seems to always be 0
	struct custom_flags {
		uint16_t unused : 3;
		uint16_t not_direct_input : 1;
		uint16_t unused2 : 4;
		uint16_t always_ask : 1;
		uint16_t focus_when_firing : 1;
		uint16_t unused3 : 6;
	} flags;
	uint16_t unused4; // seems to always be 0
};


// Size: 0x70
struct th128_replay_data_t {
	uint8_t name[12]; // only first 8 bytes are used
	uint64_t date; // Unix timestamp
	uint32_t score; // missing a trailing 0
	th128_game_config_t game_config; // copy-paste of the data in th128.cfg
	float slowdown;
	uint32_t num_stages;
	uint32_t route;
	uint32_t unused2; // seems to always be 0
	uint32_t rank;
	uint32_t last_stage;
	uint32_t unused3; // seems to always be 0
};


// Size: 0x90
struct th128_stage_header_t {
	uint16_t stage; // NOTE: the value is correctly saved for C2 route replays, the display code has a bug
	uint16_t seed; // seed used for the RNG
	uint32_t num_frames; // amount of frames the stage lasted
	uint32_t size; // stage data size, excluding header
	uint32_t score; // missing a trailing 0
	uint32_t shot_power; // must add 1
	uint32_t max_piv; // must divide by 100, it's always 1000000 but the game DOES read this value
	int32_t pos_x; // subpixel (*128), origin is middle
	int32_t pos_y; // subpixel (*128), origin is top
	uint32_t continues;
	uint32_t unused; // seems to always be 0
	uint32_t graze;
	uint32_t unused2[21]; // looks like random garbage, probably unused, randomizing it seems to change nothing
	uint32_t motivation; // must divide by 100
	uint32_t perfect_freeze; // must divide by 100
	float freeze_area;
	uint32_t unused3; // seems to always be 0
};


// Size: 0x2
union th128_key_data_t {
	uint16_t raw;
	struct key_bits {
		uint16_t shoot : 1; // doubles as freeze
		uint16_t bomb : 1;
		uint16_t unused : 1; // no key seems to set this bit
		uint16_t focus : 1;
		uint16_t up : 1;
		uint16_t down : 1;
		uint16_t left : 1;
		uint16_t right : 1;
		uint16_t unused2 : 1; // no key seems to set this bit
		uint16_t autoshoot : 1; // doubles as skip dialogue
		uint16_t unused3 : 6; // remaining bits (always 0)
	} bits;
};

// Size: 0x6
struct th128_input_data_t {
	th128_key_data_t holding;
	th128_key_data_t pressed;
	th128_key_data_t released;
};


// Size: 0xc
struct th128_user_data_header_t {
	uint32_t magic; // 'USER'
	uint64_t size; // includes header
};

#pragma pack(pop)


const char *const routes[] = {"A1", "A2", "B1", "B2", "C1", "C2", "Extra"};
const char *const ranks[] = {"Easy", "Normal", "Hard", "Lunatic", "Extra"};
const char *const stages[] = {
	NULL, // 00 is not assigned to any stage
	"A1-1", "A1-2", "A1-3", "A2-2", "A2-3",  // 01 ~ 05
	"B1-1", "B1-2", "B1-3", "B2-2", "B2-3",  // 06 ~ 0a
	"C1-1", "C1-2", "C1-3", "C2-2", "C2-3",  // 0b ~ 0f
	"Extra", // 10
	"A1 All", "A2 All", // 11 ~ 12
	"B1 All", "B2 All", // 13 ~ 14
	"C1 All", "C2 All", // 15 ~ 16
	"Ex All", // 17
};
