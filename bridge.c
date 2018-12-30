#include "bridge.h"
#include "log.h"

static void log_p5t4(uint8_t p[5], uint8_t t[4])
{
    log("%02x%02x%02x%02x%02x %02x%02x%02x%02x", p[0], p[1], p[2], p[3], p[4],
            t[0], t[1], t[2], t[3]);
}

void bridge_directionals(int vert, int horiz, uint8_t pan_speed, uint8_t tilt_speed)
{
    log("bridge_directionals: % d, % d, (pan %d, tilt %d)", vert, horiz, pan_speed, tilt_speed);
}

void bridge_relative_move(uint8_t speed, uint8_t p[5], uint8_t t[4])
{
    log("bridge_relative_move: %d,", speed);
    log_p5t4(p, t);
}

void bridge_absolute_move(uint8_t speed, uint8_t p[5], uint8_t t[4])
{
    log("bridge_absolute_move: %d,", speed);
    log_p5t4(p, t);
}

void bridge_home()
{
    log("bridge_home");
}

void bridge_reset()
{
    log("bridge_reset");
}

void bridge_pan_tilt_limit_set(uint8_t position, uint8_t p[5], uint8_t t[4])
{
    log("bridge_pan_tilt_limit_set: %d,", position);
    log_p5t4(p, t);
}

void bridge_pan_tilt_limit_clear(uint8_t position)
{
    log("bridge_pan_tilt_limit_clear: %d", position);
}

void bridge_ramp_curve(int p)
{
    log("bridge_ramp_curve: %d", p);
}

void bridge_slow_mode(int p)
{
    log("bridge_slow_mode: %d", p);
}

