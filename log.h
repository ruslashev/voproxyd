#pragma once

#include "errors.h"

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

#ifdef DAEMONIZE
#include <syslog.h>
#define log(...) syslog(LOG_NOTICE, __VA_ARGS__)
#else
#include <stdio.h>
#define log(...) do { printf(__VA_ARGS__); puts(""); } while (0)
#endif

#define die_detail(X, ...) do { log(__VA_ARGS__); exit(X); } while (0)

#define die(X, ...) die_detail(X, __VA_ARGS__)

static inline int print_byte(char *writebuf, uint8_t x, int base)
{
    int len = 8, i = 0;

    if (base == 16) {
        sprintf(writebuf, "%02x", x);
        return 2;
    }

    while (--len >= 0) {
        writebuf[i++] = (x & ((uint8_t)1 << len)) ? '1' : '0';
    }

    return 8;
}

static inline void print_buffer(const void *buf, size_t len, int base)
{
    size_t i;
    int bi = 0;
    char writebuf[4096] = { 0 };

    if (base != 2 && base != 16)
        die(ERR_UNSPECIFIED, "print_buffer: base can be 2 or 16, not %d", base);

    for (i = 0; i < len; ++i) {
        bi += print_byte(writebuf + bi, ((unsigned char*)buf)[i], base);
        writebuf[bi++] = ' ';
    }

    writebuf[bi] = 0;

    log("%s", writebuf);
}

static inline void print_buffer_two_bases(const void *buf, size_t len)
{
    print_buffer(buf, len, 16);
    print_buffer(buf, len, 2);
}

