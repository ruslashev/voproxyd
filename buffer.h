#pragma once

#include <stddef.h>
#include <stdint.h>

typedef struct
{
    size_t length;
    uint8_t *data;
} buffer_t;

buffer_t* cons_buffer(size_t length);
void print_bytes(const void *bytes, size_t len, int base);
void print_buffer(const buffer_t *buffer, int base);

