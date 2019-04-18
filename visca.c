#include "visca.h"
#include "log.h"

#undef die
#define die(...) die_detail(ERR_VISCA_PROTOCOL, __VA_ARGS__)

buffer_t* compose_ack()
{
    buffer_t *response = cons_buffer(3);

    response->data[0] = 0x90;
    response->data[1] = 0x40;
    response->data[2] = 0xff;

    return response;
}

buffer_t* compose_completition(buffer_t *data)
{
    size_t data_len = (data == NULL) ? 0 : data->length;
    buffer_t *response = cons_buffer(3 + data_len);

    response->data[0] = 0x90;
    response->data[1] = 0x50;

    for (size_t i = 0; i < data_len; ++i) {
        response->data[2 + i] = data->data[i];
    }

    response->data[2 + data_len] = 0xff;

    return response;
}

buffer_t* compose_empty_completition()
{
    return compose_completition(NULL);
}

void visca_handle_message(const buffer_t *message_buf, const struct event_t *event)
{

}

