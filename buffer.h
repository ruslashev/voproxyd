#pragma once

#include <stddef.h>
#include <stdint.h>

typedef struct
{
    size_t length;
    uint8_t *data;
} buffer_t;

buffer_t* cons_buffer(size_t length);
void free_buffer(buffer_t *buffer);
void print_buffer_msg(const char *msg, const buffer_t *buffer, int base);
void print_buffer(const buffer_t *buffer, int base);

