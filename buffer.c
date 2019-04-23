#include "buffer.h"
#include "log.h"
#include <string.h>

buffer_t* cons_buffer_with_value(size_t length, uint8_t value)
{
    buffer_t *out = malloc(sizeof(buffer_t));
    if (out == NULL)
        die(ERR_ALLOC, "failed to allocate memory for buffer_t");

    out->length = length;

    out->data = malloc(length);

    memset(out->data, value, length);

    return out;
}

buffer_t* cons_buffer(size_t length)
{
    return cons_buffer_with_value(length, 0);
}

void free_buffer(buffer_t *buffer)
{
    free(buffer->data);
    free(buffer);
}

static int print_byte(char *writebuf, uint8_t x, int base)
{
    int len = 8, i = 0;

    if (base == 16) {
        sprintf(writebuf, "%02x", x);
        return 2;
    }

    while (--len >= 0) {
        writebuf[i++] = (x & (1u << (unsigned)len)) ? '1' : '0';
    }

    return 8;
}

static void print_bytes(const char *msg, const void *bytes, size_t len, int base)
{
    size_t i;
    int bi = 0;
    char writebuf[4096] = { 0 };

    if (base != 2 && base != 16)
        die(ERR_UNSPECIFIED, "print_bytes: base can be 2 or 16, not %d", base);

    for (i = 0; i < len; ++i) {
        bi += print_byte(writebuf + bi, ((unsigned char*)bytes)[i], base);
        writebuf[bi++] = ' ';
    }

    writebuf[bi] = 0;

    if (msg != NULL)
        log("%s: %s", msg, writebuf);
    else
        log("%s", writebuf);
}

void print_buffer_msg(const char *msg, const buffer_t *buffer, int base)
{
    print_bytes(msg, buffer->data, buffer->length, base);
}

void print_buffer(const buffer_t *buffer, int base)
{
    print_bytes(NULL, buffer->data, buffer->length, base);
}

