#pragma once

#include <stddef.h>
#include <stdint.h>

#define VISCA_OVER_IP_MAX_PAYLOAD_LENGTH 16
#define VISCA_OVER_IP_HEADER_LENGTH 8
#define VISCA_OVER_IP_MAX_MESSAGE_LENGTH 24

void visca_handle_message(const uint8_t *message, size_t length, uint8_t *response, size_t *response_len);

