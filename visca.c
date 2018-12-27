#include "errors.h"
#include "log.h"
#include "visca.h"

#include <netdb.h>

struct visca_header
{
    uint16_t payload_type;
    uint16_t payload_length;
    uint32_t seq_number;
};

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
            log("visca control command RESET");
            break;
        case 0x0F: /* ERROR */
            if (length != 2) {
                die(ERR_PROTOCOL, "handle_control_command: ERROR: excepted length == 2, got %ld",
                        length);
            }
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
        die(ERR_PROTOCOL,
                "assertion `header->payload_length == length - 8' failed: %d != %ld",
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

