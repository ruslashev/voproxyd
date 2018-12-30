#pragma once

#include <stdint.h>

void bridge_directionals(int vert, int horiz, uint8_t pan_speed, uint8_t tilt_speed);
void bridge_relative_move(uint8_t speed, uint8_t p[5], uint8_t t[4]);
void bridge_absolute_move(uint8_t speed, uint8_t p[5], uint8_t t[4]);
void bridge_home();
void bridge_reset();
void bridge_pan_tilt_limit_set(uint8_t position, uint8_t p[5], uint8_t t[4]);
void bridge_pan_tilt_limit_clear(uint8_t position);
void bridge_ramp_curve(int p);
void bridge_slow_mode(int p);

