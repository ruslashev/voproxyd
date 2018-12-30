#pragma once

#include <stddef.h>
#include <stdint.h>

#define VOIP_MAX_PAYLOAD_LENGTH 16
#define VOIP_HEADER_LENGTH 8
#define VOIP_MAX_MESSAGE_LENGTH (VOIP_HEADER_LENGTH + VOIP_MAX_PAYLOAD_LENGTH)

void visca_handle_message(const uint8_t *message, size_t length, uint8_t *response, size_t *response_len);

