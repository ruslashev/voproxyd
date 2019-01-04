#include "bridge.h"
#include "buffer.h"
#include "errors.h"
#include "log.h"
#include "socket.h"
#include "visca.h"
#include "visca_commands.h"
#include "visca_inquiries.h"
#include <netdb.h>
#include <string.h>

// redefine die() because all errors here are visca protocol errors
#undef die
#define die(...) die_detail(ERR_VISCA_PROTOCOL, __VA_ARGS__)

_Static_assert(sizeof(struct visca_header) == VOIP_HEADER_LENGTH, "VISCA header size must be 8 bytes");

static void visca_header_convert_endianness_ntoh(struct visca_header *header)
{
    header->payload_type = ntohs(header->payload_type);
    header->payload_length = ntohs(header->payload_length);
    header->seq_number = ntohl(header->seq_number);
}

static void visca_header_convert_endianness_hton(struct visca_header *header)
{
    header->payload_type = htons(header->payload_type);
    header->payload_length = htons(header->payload_length);
    header->seq_number = htonl(header->seq_number);
}

void compose_ack(buffer_t *response)
{
    response->length = 3;

    response->data[0] = 0x90;
    response->data[1] = 0x40;
    response->data[2] = 0xff;
}

buffer_t* compose_completition(buffer_t *data)
{
    size_t data_len = (data == NULL) ? 0 : data->length;
    buffer_t *response = cons_buffer(3 + data_len);

    if (3 + data_len > VOIP_MAX_MESSAGE_LENGTH) {
        log("compose_completition: buffer length too big: %zu", data_len);
        return NULL;
    }

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

void compose_control_reply(buffer_t *response, uint32_t seq_number)
{
    struct visca_header header = {
        .payload_type = 0x0201,
        .payload_length = 0x01,
        .seq_number = seq_number
    };

    visca_header_convert_endianness_hton(&header);

    response->length = VOIP_HEADER_LENGTH + 1;

    memcpy(response->data, &header, VOIP_HEADER_LENGTH);

    response->data[VOIP_HEADER_LENGTH] = 0x01; /* ACK: reply for RESET */
}

static void handle_visca_command(const struct message_t *message, const struct event_t *event)
{
    log("handle_visca_command");

    if (message->payload_length < 5) {
        log("handle_visca_command: bad length %zu", message->payload_length);
        return;
    }

    if (message->payload[0] != 0x81 || message->payload[1] != 0x01) {
        log("handle_visca_command: unexpected payload start %02x %02x",
                message->payload[0], message->payload[1]);
        return;
    }

    visca_commands_dispatch(message, event);
}

static void handle_visca_inquiry(const struct message_t *message, const struct event_t *event)
{
    buffer_t *inquiry_data, *response;

    log("handle_visca_inquiry");

    if (message->payload_length < 5) {
        log("handle_visca_inquiry: unexpected length %zu", message->payload_length);
        return;
    }

    if (message->payload[0] != 0x81) {
        log("handle_visca_inquiry: unexpected payload start 0x%02x", message->payload[0]);
        return;
    }

    inquiry_data = visca_inquiries_dispatch(message);

    if (inquiry_data != NULL) {
        response = compose_completition(inquiry_data);
        socket_send_message_udp_event(event, response);
        free(response);
    }
}

static void handle_visca_reply(const struct message_t *message, const struct event_t *event)
{
    (void)message;
    (void)event;

    log("handle_visca_reply");
}

static void handle_visca_device_setting_cmd(const struct message_t *message, const struct event_t *event)
{
    (void)message;
    (void)event;

    log("handle_visca_device_setting_cmd");
}

static void handle_control_command(const struct message_t *message, const struct event_t *event)
{
    log("handle_control_command");
    buffer_t *response = cons_buffer(VOIP_MAX_MESSAGE_LENGTH);

    switch (message->payload[0]) {
        case 0x01:
            log("control command RESET");
            /* seq_number = 0; */
            break;
        case 0x0F:
            log("control command ERROR");

            check_length(2);

            switch (message->payload[1]) {
                case 0x01:
                    log("abnormality in the sequence number");
                    break;
                case 0x02:
                    log("abnormality in the message type");
                    break;
                default:
                    log("handle_control_command: ERROR: unexpected error type 0x%02x",
                            message->payload[1]);
                    return;
            }

            break;
        default:
            log("handle_control_command: unexpected control command type 0x%02x", message->payload[0]);
            return;
    }

    compose_control_reply(response, message->header->seq_number);
    socket_send_message_udp_event(event, response);
}

static void handle_control_reply(const struct message_t *message, const struct event_t *event)
{
    (void)message;
    (void)event;

    log("handle_control_reply");
}

void visca_handle_message(const buffer_t *message_buf, const struct event_t *event)
{
    struct message_t message = {
        .header = (struct visca_header*)message_buf->data,
        .payload = message_buf->data + 8,
        .payload_length = message_buf->length - 8,
    };

    log(" ");
    log("got msg:");
    print_buffer(message_buf, 16);

    visca_header_convert_endianness_ntoh(message.header);

    log("ptype=0x%04x plen=%d seq_number=%d", message.header->payload_type,
            message.header->payload_length, message.header->seq_number);

    if (message.header->payload_length != message.payload_length) {
        log("assertion `header->payload_length == payload.length' failed: %d != %zu",
                message.header->payload_length, message.payload_length);
        return;
    }

    switch (message.header->payload_type) {
        case 0x0100:
            handle_visca_command(&message, event);
            break;
        case 0x0110:
            handle_visca_inquiry(&message, event);
            break;
        case 0x0111:
            handle_visca_reply(&message, event);
            break;
        case 0x0120:
            handle_visca_device_setting_cmd(&message, event);
            break;
        case 0x0200:
            handle_control_command(&message, event);
            break;
        case 0x0201:
            handle_control_reply(&message, event);
            break;
        default:
            log("visca_handle_message: unexpected payload type 0x%04x", message.header->payload_type);
            return;
    }
}

/* Command ->
       <- Ack             OR    <- Ack
       <- Completition          <- Error

   Inquiry ->
       <- Completition (+ inquiry result data)    OR    <- Error
 */

