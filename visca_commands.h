#pragma once

#include "buffer.h"

#include <stdint.h>

void visca_commands_dispatch(const struct message_t *message, const struct event_t *event);

