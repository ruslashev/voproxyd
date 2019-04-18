#pragma once

#include "buffer.h"
#include "epoll.h"

buffer_t* compose_ack();
buffer_t* compose_completition(buffer_t *data);
buffer_t* compose_empty_completition();
void visca_handle_message(const buffer_t *message_buf, const struct event_t *event);

