#include "bridge.h"
#include "buffer.h"
#include "errors.h"
#include "log.h"
#include "visca.h"
#include "visca_commands.h"
#include "visca_inquiries.h"

#include <netdb.h>
#include <string.h>

// redefine die() because all errors here are visca protocol errors
#undef die
#define die(...) die_detail(ERR_VISCA_PROTOCOL, __VA_ARGS__)

#define stub() do { log("%s:%d: unimplemented", __func__, __LINE__); return; } while (0)

struct visca_header
{
    uint16_t payload_type;
    uint16_t payload_length;
    uint32_t seq_number;
} __attribute__ ((packed));

_Static_assert(sizeof(struct visca_header) == VOIP_HEADER_LENGTH,
        "VISCA header size must be 8 bytes");

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

void compose_completition(buffer_t *response, const uint8_t data[], size_t data_len)
{
    if (3 + data_len > VOIP_MAX_MESSAGE_LENGTH) {
        log("compose_completition: buffer length too big: %zu", data_len);
        return;
    }

    response->length = 3 + data_len;

    response->data[0] = 0x90;
    response->data[1] = 0x50;

    for (size_t i = 0; i < data_len; ++i) {
        response->data[2 + i] = data[i];
    }

    response->data[2 + data_len] = 0xff;
}

void compose_empty_completition(buffer_t *response)
{
    compose_completition(response, NULL, 0);
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

static void handle_visca_command(const buffer_t *payload, uint32_t seq_number, buffer_t *response)
{
    log("handle_visca_command");

    if (payload->length < 5) {
        log("handle_visca_command: bad length %zu", payload->length);
        return;
    }

    if (payload->data[0] != 0x81 || payload->data[1] != 0x01) {
        log("handle_visca_command: unexpected payload start %02x %02x",
                payload->data[0], payload->data[1]);
        return;
    }

    visca_commands_dispatch(payload, seq_number, response);
}

static void handle_visca_inquiry(const buffer_t *payload, uint32_t seq_number, buffer_t *response)
{
    log("handle_visca_inquiry");

    if (payload->length < 5) {
        log("handle_visca_inquiry: unexpected length %zu", payload->length);
        return;
    }

    if (payload->data[0] != 0x81) {
        log("handle_visca_inquiry: unexpected payload start 0x%02x", payload->data[0]);
        return;
    }

    if (payload->data[1] == 0x01) {
        switch (payload->data[2]) {
            case 0x7e:
                if (payload->data[3] == 0x01) {
                    bridge_inq_exposure_nd_filter();
                } else if (payload->data[3] == 0x04) {
                    bridge_inq_preset_mode();
                } else {
                    log("handle_visca_inquiry: unexpected byte 0x%02x", payload->data[3]);
                    return;
                }
                break;
            case 0x06:
                bridge_inq_pan_tilt_limit();
                break;
        }
    } else if (payload->data[1] == 0x09) {
        visca_inquiries_dispatch(payload, seq_number, response);
    } else {
        log("handle_visca_inquiry: invalid packet continuation 0x%02x", payload->data[1]);
        return;
    }
}

static void handle_visca_reply(const buffer_t *payload, uint32_t seq_number, buffer_t *response)
{
    (void)payload;
    (void)response;
    (void)seq_number;

    log("handle_visca_reply");
}

static void handle_visca_device_setting_cmd(const buffer_t *payload, uint32_t seq_number,
        buffer_t *response)
{
    (void)payload;
    (void)response;
    (void)seq_number;

    log("handle_visca_device_setting_cmd");
}

static void handle_control_command(const buffer_t *payload, uint32_t seq_number, buffer_t *response)
{
    log("handle_control_command");

    switch (payload->data[0]) {
        case 0x01: /* RESET */
            log("control command RESET");
            seq_number = 0;
            break;
        case 0x0F: /* ERROR */
            log("control command ERROR");

            check_length(2);

            switch (payload->data[1]) {
                case 0x01:
                    log("abnormality in the sequence number");
                    break;
                case 0x02:
                    log("abnormality in the message type");
                    break;
                default:
                    log("handle_control_command: ERROR: unexpected error type 0x%02x",
                            payload->data[1]);
                    return;
            }

            break;
        default:
            log("handle_control_command: unexpected control command type 0x%02x", payload->data[0]);
            return;
    }

    compose_control_reply(response, seq_number);
}

static void handle_control_reply(const buffer_t *payload, uint32_t seq_number, buffer_t *response)
{
    (void)payload;
    (void)response;
    (void)seq_number;

    log("handle_control_reply");
}

buffer_t* visca_handle_message(const buffer_t *message)
{
    struct visca_header *header = (struct visca_header*)message->data;
    buffer_t payload = {
        .length = message->length - 8,
        .data = message->data + 8
    }, *response = cons_buffer(VOIP_MAX_MESSAGE_LENGTH);

    response->length = 0;

    log(" ");
    log("got msg:");
    print_buffer(message, 16);

    visca_header_convert_endianness_ntoh(header);

    /* log("header->payload_type=0x%04x", header->payload_type); */
    /* log("header->payload_length=%d", header->payload_length); */
    /* log("header->seq_number=%d", header->seq_number); */

    if (header->payload_length != payload.length) {
        log("assertion `header->payload_length == payload.length' failed: %d != %zu",
                header->payload_length, payload.length);
        return NULL;
    }

    switch (header->payload_type) {
        case 0x0100:
            handle_visca_command(&payload, header->seq_number, response);
            break;
        case 0x0110:
            handle_visca_inquiry(&payload, header->seq_number, response);
            break;
        case 0x0111:
            handle_visca_reply(&payload, header->seq_number, response);
            break;
        case 0x0120:
            handle_visca_device_setting_cmd(&payload, header->seq_number, response);
            break;
        case 0x0200:
            handle_control_command(&payload, header->seq_number, response);
            break;
        case 0x0201:
            handle_control_reply(&payload, header->seq_number, response);
            break;
        default:
            log("visca_handle_message: unexpected payload type 0x%04x", header->payload_type);
            return NULL;
    }

    return response;
}

/* Command ->
       <- Ack             OR    <- Ack
       <- Completition          <- Error

   Inquiry ->
       <- Completition (+ inquiry result data)    OR    <- Error
 */

