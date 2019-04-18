#pragma once

#include "buffer.h"
#include "epoll.h"

#define bad_byte_detail(X, R) \
    do { \
        log("%s:%d: unexpected byte 0x%02x", __func__, __LINE__, message->payload[X]); \
        return R; \
    } while (0)
#define bad_byte_null(X) bad_byte_detail(X, NULL)
#define bad_byte(X) bad_byte_detail(X, )

#define check_length_detail(X, R) \
    if (message->length != (X)) { \
        log("%s: bad length %zu, expected %d", __func__, message->length, X); \
        return R; \
    }
#define check_length_null(X) check_length_detail(X, NULL)
#define check_length(X) check_length_detail(X, )

#define visca_send_response(E, R) \
    do { \
        log("%s: send response", __func__); \
        print_buffer(R, 16); \
        socket_send_message_udp_event(E, R); \
    } while (0)

buffer_t* compose_ack();
buffer_t* compose_completition(buffer_t *data);
buffer_t* compose_empty_completition();
void visca_handle_message(const buffer_t *message, const struct event_t *event);

