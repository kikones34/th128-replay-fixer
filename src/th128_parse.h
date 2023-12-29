#pragma once

#include <stdint.h>


// utf-8 char codes
namespace char_codes {
    const uint8_t noop[] = {'.'};

    const uint8_t left[] = {0xE2, 0x86, 0x90};
    const uint8_t up[] = {0xE2, 0x86, 0x91};
    const uint8_t right[] = {0xE2, 0x86, 0x92};
    const uint8_t down[] = {0xE2, 0x86, 0x93};
    const uint8_t up_left[] = {0xE2, 0x86, 0x96};
    const uint8_t up_right[] = {0xE2, 0x86, 0x97};
    const uint8_t down_right[] = {0xE2, 0x86, 0x98};
    const uint8_t down_left[] = {0xE2, 0x86, 0x99};

    const uint8_t bomb[] = {'B'};
    const uint8_t focus_press[] = {'F'};
    const uint8_t focus_release[] = {'f'};
    const uint8_t shoot_press[] = {'S'};
    const uint8_t shoot_release[] = {'s'};
    const uint8_t autoshoot_press[] = {'A'};
    const uint8_t autoshoot_release[] = {'a'};
    const uint8_t quit[] = {'Q'};

    // direction inputs to movement mapping
    const uint8_t *const movement[] = {
        noop, up, down, down,
        left, up_left, down_left, up_left,
        right, up_right, down_right, up_right,
        left, up_left, down_left, up_left,
    };
}


// game config constants
const char *const color_modes[] = {"32-bit", "16-bit"};
const char *const screen_modes[] = {"Fullscreen (640x480)", "Windowed (640x480)", "Windowed (960x720)", "Windowed (1280x960)"};
const char *const frameskip_modes[] = {"None", "1/2", "1/3"};
const char *const input_latency_modes[] = {"Stable", "Normal", "Automatic", "Fast"};


void th128_parse_replay_data(uint8_t *decoded_data, const char *out_file);
bool th128_decode_replay_file(const char *file);
bool th128_parse_user_data(const char *file);
