#include "buffer.h"
#include "errors.h"
#include "log.h"
#include "socket.h"
#include "visca.h"
#include "sony_visca.h"
#include "sony_visca_commands.h"
#include "sony_visca_inquiries.h"
#include <netdb.h>
#include <string.h>

// redefine die() because all errors here are visca protocol errors
#undef die
#define die(...) die_detail(ERR_VISCA_PROTOCOL, __VA_ARGS__)

_Static_assert(sizeof(struct visca_header_t) == VOIP_HEADER_LENGTH, "VISCA header size must be 8 bytes");

static void visca_header_convert_endianness_ntoh(struct visca_header_t *header)
{
    header->payload_type = ntohs(header->payload_type);
    header->payload_length = ntohs(header->payload_length);
    header->seq_number = ntohl(header->seq_number);
}

static void visca_header_convert_endianness_hton(struct visca_header_t *header)
{
    header->payload_type = htons(header->payload_type);
    header->payload_length = htons(header->payload_length);
    header->seq_number = htonl(header->seq_number);
}

buffer_t* compose_control_reply(uint32_t seq_number)
{
    buffer_t *response = cons_buffer(VOIP_HEADER_LENGTH + 1);
    struct visca_header_t header = {
        .payload_type = 0x0201,
        .payload_length = 0x01,
        .seq_number = seq_number
    };

    visca_header_convert_endianness_hton(&header);

    memcpy(response->data, &header, VOIP_HEADER_LENGTH);

    response->data[VOIP_HEADER_LENGTH] = 0x01; /* ACK: reply for RESET */

    return response;
}

static void handle_visca_command(const struct message_t *message, const struct event_t *event)
{
    buffer_t *response = compose_ack();

    log("visca: handle_visca_command");

    visca_send_response(event, response);
    free(response);

    if (message->payload_length < 5) {
        log("handle_visca_command: bad length %zu", message->payload_length);
        return;
    }

    if (message->payload[0] != 0x81 || message->payload[1] != 0x01) {
        log("handle_visca_command: unexpected payload start %02x %02x",
                message->payload[0], message->payload[1]);
        return;
    }

    sony_visca_commands_dispatch(message, event);

    response = compose_empty_completition();
    visca_send_response(event, response);
    free(response);
}

static void handle_visca_inquiry(const struct message_t *message, const struct event_t *event)
{
    buffer_t *inquiry_data, *response;

    log("visca: handle_visca_inquiry");

    if (message->payload_length < 5) {
        log("handle_visca_inquiry: unexpected length %zu", message->payload_length);
        return;
    }

    if (message->payload[0] != 0x81) {
        log("handle_visca_inquiry: unexpected payload start 0x%02x", message->payload[0]);
        return;
    }

    inquiry_data = sony_visca_inquiries_dispatch(message);

    if (inquiry_data != NULL) {
        response = compose_completition(inquiry_data);

        visca_send_response(event, response);

        free(response);
        free(inquiry_data);
    }
}

static void handle_visca_reply(const struct message_t *message, const struct event_t *event)
{
    (void)message;
    (void)event;

    log("visca: handle_visca_reply");
}

static void handle_visca_device_setting_cmd(const struct message_t *message, const struct event_t *event)
{
    (void)message;
    (void)event;

    log("visca: handle_visca_device_setting_cmd");
}

static void handle_control_command(const struct message_t *message, const struct event_t *event)
{
    log("visca: handle_control_command");
    buffer_t *response;

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

    response = compose_control_reply(message->header->seq_number);
    visca_send_response(event, response);
    free(response);
}

static void handle_control_reply(const struct message_t *message, const struct event_t *event)
{
    (void)message;
    (void)event;

    log("visca: handle_control_reply");
}

void sony_visca_handle_message(const buffer_t *message_buf, const struct event_t *event)
{
    struct message_t message = {
        .header = (struct visca_header_t*)message_buf->data,
        .payload = message_buf->data + 8,
        .payload_length = message_buf->length - 8,
    };

    log("visca: visca_handle_message: got msg:");
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

