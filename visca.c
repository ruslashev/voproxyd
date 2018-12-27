#include "errors.h"
#include "log.h"
#include "visca.h"

#include <netdb.h>

#define VISCA_OVER_IP_MAX_PAYLOAD_LENGTH 16
#define VISCA_OVER_IP_HEADER_LENGTH 8
#define VISCA_OVER_IP_MAX_MESSAGE_LENGTH 24

// redefine die() because all errors here are visca protocol errors
#undef die
#define die(...) die_detail(ERR_VISCA_PROTOCOL, __VA_ARGS__)

struct visca_header
{
    uint16_t payload_type;
    uint16_t payload_length;
    uint32_t seq_number;
} __attribute__ ((packed));

_Static_assert(sizeof(struct visca_header) == VISCA_OVER_IP_HEADER_LENGTH,
        "VISCA header size must be 8 bytes");

static uint32_t g_seq_number = 0;

static void compose_ack(uint8_t *buffer, size_t *n)
{
    *(buffer + 0) = 0x90;
    *(buffer + 1) = 0x40;
    *(buffer + 2) = 0xff;

    *n = 3;
}

static void compose_completition(uint8_t *buffer, size_t *n, uint8_t data[], size_t data_len)
{
    *(buffer + 0) = 0x90;
    *(buffer + 1) = 0x50;

    for (size_t i = 0; i < data_len; ++i) {
        *(buffer + 2 + i) = data[i];
    }

    *(buffer + 2 + data_len) = 0xff;

    *n = 3 + data_len;
}

static void compose_empty_completition(uint8_t *buffer, size_t *n)
{
    compose_completition(buffer, n, NULL, 0);
}

static void compose_control_reply(uint8_t *buffer, size_t *n)
{
    struct visca_header header = {
        .payload_type = 0x0201,
        .payload_length = 0x01,
        .seq_number = g_seq_number++ // todo: packet may be missed
    };

    buffer = (uint8_t*)&header;

    *(buffer + VISCA_OVER_IP_HEADER_LENGTH) = 0x01; /* ACK: reply for RESET */

    *n = VISCA_OVER_IP_HEADER_LENGTH + 1;

    log("compose_control_reply: debug response:");
    print_buffer_two_bases(buffer, *n);
}

static void visca_header_convert_endianness(struct visca_header *header)
{
    header->payload_type = ntohs(header->payload_type);
    header->payload_length = ntohs(header->payload_length);
    header->seq_number = ntohl(header->seq_number);
}

static void handle_visca_command(const uint8_t *payload, size_t length, uint32_t seq_number)
{
    (void)payload;
    (void)length;
    (void)seq_number;

    log("handle_visca_command");
}

static void handle_visca_inquiry(const uint8_t *payload, size_t length, uint32_t seq_number)
{
    (void)payload;
    (void)length;
    (void)seq_number;

    log("handle_visca_inquiry");
}

static void handle_visca_reply(const uint8_t *payload, size_t length, uint32_t seq_number)
{
    (void)payload;
    (void)length;
    (void)seq_number;

    log("handle_visca_reply");
}

static void handle_visca_device_setting_cmd(const uint8_t *payload, size_t length, uint32_t seq_number)
{
    (void)payload;
    (void)length;
    (void)seq_number;

    log("handle_visca_device_setting_cmd");
}

static void handle_control_command(const uint8_t *payload, size_t length, uint32_t seq_number)
{
    (void)payload;
    (void)length;
    (void)seq_number;

    log("handle_control_command");

    switch (payload[0]) {
        case 0x01: /* RESET */
            log("control command RESET");
            break;
        case 0x0F: /* ERROR */
            if (length != 2) {
                die("handle_control_command: ERROR: excepted length == 2, got %ld", length);
            }
            switch (payload[1]) {
                case 0x01:
                    log("abnormality in the sequence number");
                    break;
                case 0x02:
                    log("abnormality in the sequence number");
                    break;
            }
        default:
            die("handle_control_command: unexpected control command type 0x%x", payload[0]);
    }
}

static void handle_control_reply(const uint8_t *payload, size_t length, uint32_t seq_number)
{
    (void)payload;
    (void)length;
    (void)seq_number;

    log("handle_control_reply");
}

void visca_handle_message(const uint8_t *message, size_t length)
{
    struct visca_header *header = (struct visca_header*)message;
    const uint8_t *payload = message + 8;
    size_t payload_length = length - 8;

    log("got msg:");
    print_buffer(message, length, 16);
    print_buffer(message, length, 2);
    log(" ");

    /* visca_header_convert_endianness(header); */

    log("header->payload_type=0x%x", header->payload_type);
    log("header->payload_length=%d", header->payload_length);
    log("header->seq_number=%d", header->seq_number);

    if (header->payload_length != length - 8) {
        die("assertion `header->payload_length == length - 8' failed: %d != %ld",
                header->payload_length, length - 8);
    }

    switch (header->payload_type) {
        case 0x0100:
            handle_visca_command(payload, payload_length, header->seq_number);
            break;
        case 0x0110:
            handle_visca_inquiry(payload, payload_length, header->seq_number);
            break;
        case 0x0111:
            handle_visca_reply(payload, payload_length, header->seq_number);
            break;
        case 0x0120:
            handle_visca_device_setting_cmd(payload, payload_length, header->seq_number);
            break;
        case 0x0200:
            handle_control_command(payload, payload_length, header->seq_number);
            break;
        case 0x0201:
            handle_control_reply(payload, payload_length, header->seq_number);
            break;
    }
}

/* Command ->
       <- Ack             OR    <- Ack
       <- Completition          <- Error

   Inquiry ->
       <- Completition (+ inquiry result data)    OR    <- Error
 */

