#include "buffer.h"
#include "log.h"

struct buffer_t* cons_buffer(size_t length)
{
    struct buffer_t *out = malloc(sizeof(struct buffer_t));
    if (out == NULL) {
        die(ERR_ALLOC, "failed to allocate memory for buffer_t");
    }

    out->length = length;

    out->data = calloc(length, 1);

    return out;
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

void print_bytes(const void *bytes, size_t len, int base)
{
    size_t i;
    int bi = 0;
    char writebuf[4096] = { 0 };

    if (base != 2 && base != 16) {
        die(ERR_UNSPECIFIED, "print_bytes: base can be 2 or 16, not %d", base);
    }

    for (i = 0; i < len; ++i) {
        bi += print_byte(writebuf + bi, ((unsigned char*)bytes)[i], base);
        writebuf[bi++] = ' ';
    }

    writebuf[bi] = 0;

    log("%s", writebuf);
}

void print_buffer(const struct buffer_t *buffer, int base)
{
    print_bytes(buffer->data, buffer->length, base);
}

