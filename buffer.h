#pragma once

#include <stddef.h>
#include <stdint.h>

struct buffer_t
{
    size_t length;
    uint8_t *data;
};

struct buffer_t* cons_buffer(size_t length);
void print_bytes(const void *bytes, size_t len, int base);
void print_buffer(const struct buffer_t *buffer, int base);

