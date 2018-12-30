#pragma once

#include "buffer.h"

#define VOIP_MAX_PAYLOAD_LENGTH 16
#define VOIP_HEADER_LENGTH 8
#define VOIP_MAX_MESSAGE_LENGTH (VOIP_HEADER_LENGTH + VOIP_MAX_PAYLOAD_LENGTH)

buffer_t* visca_handle_message(const buffer_t *message);

