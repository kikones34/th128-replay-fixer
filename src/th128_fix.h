#pragma once

#include <stdint.h>


// Route strings for user data
const char *const route_strs[] = {"RouteA-1", "RouteA-2", "RouteB-1", "RouteB-2", "RouteC-1", "RouteC-2", "Extra   "};


void th128_fix_replay_data(uint8_t *decoded_data, uint32_t new_route);
void th128_fix_user_data(uint8_t *user_data, uint32_t new_route);
bool th128_fix_replay_file(const char *file);
