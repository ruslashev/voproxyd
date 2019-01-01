#pragma once

#include "buffer.h"

#define VOIP_MAX_PAYLOAD_LENGTH 16
#define VOIP_HEADER_LENGTH 8
#define VOIP_MAX_MESSAGE_LENGTH (VOIP_HEADER_LENGTH + VOIP_MAX_PAYLOAD_LENGTH)

#define check_length_detail(X, R) \
    if (payload->length != (X)) { \
        log("%s: bad length %zu, expected %d", __func__, payload->length, X); \
        return R; \
    }

#define check_length_null(X) check_length_detail(X, NULL)
#define check_length(X) check_length_detail(X, )

#define bad_byte(X) \
    do { \
        log("%s:%d: unexpected byte 0x%02x", __func__, __LINE__, payload->data[X]); \
        return; \
    } while (0)

void compose_ack(buffer_t *response);
void compose_completition(buffer_t *response, const uint8_t data[], size_t data_len);
void compose_empty_completition(buffer_t *response);
void compose_control_reply(buffer_t *response, uint32_t seq_number);
buffer_t* visca_handle_message(const buffer_t *message);

