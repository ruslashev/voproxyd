#pragma once

#include "buffer.h"
#include "epoll.h"

struct visca_header_t
{
    uint16_t payload_type;
    uint16_t payload_length;
    uint32_t seq_number;
} __attribute__ ((packed));

struct message_t
{
    struct visca_header_t *header;
    const uint8_t *payload;
    size_t payload_length;
};

#define VOIP_MAX_PAYLOAD_LENGTH 16
#define VOIP_HEADER_LENGTH 8
#define VOIP_MAX_MESSAGE_LENGTH (VOIP_HEADER_LENGTH + VOIP_MAX_PAYLOAD_LENGTH)

buffer_t* compose_control_reply(uint32_t seq_number);
void sony_visca_handle_message(const buffer_t *message_buf, const struct event_t *event);

