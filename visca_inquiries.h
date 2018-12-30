#pragma once

#include "buffer.h"

#include <stdint.h>

void visca_inquiries_dispatch(const buffer_t *payload, uint32_t seq_number,
    buffer_t *response);

