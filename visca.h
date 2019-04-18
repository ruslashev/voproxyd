#pragma once

#include "buffer.h"
#include "epoll.h"
#include "socket.h"

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

