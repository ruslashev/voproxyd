#include "errors.h"
#include "log.h"
#include "visca.h"

#include <netdb.h>

// redefine die() because all errors here are visca protocol errors
#undef die
#define die(...) die_detail(ERR_VISCA_PROTOCOL, __VA_ARGS__)

#define check_length(X) \
    if (length != (X)) { \
        log("%s: bad length %zu, expected %d", __func__, length, X); \
        return; \
    }

struct visca_header
{
    uint16_t payload_type;
    uint16_t payload_length;
    uint32_t seq_number;
} __attribute__ ((packed));

_Static_assert(sizeof(struct visca_header) == VISCA_OVER_IP_HEADER_LENGTH,
        "VISCA header size must be 8 bytes");

static uint8_t g_response[VISCA_OVER_IP_MAX_MESSAGE_LENGTH];
static size_t g_response_len;

static void compose_ack(uint8_t *buffer, size_t *n)
{
    *(buffer + 0) = 0x90;
    *(buffer + 1) = 0x40;
    *(buffer + 2) = 0xff;

    *n = 3;
}

static void compose_completition(uint8_t *buffer, size_t *n, const uint8_t data[], size_t data_len)
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

static void compose_control_reply(uint8_t *buffer, size_t *n, uint32_t seq_number)
{
    struct visca_header header = {
        .payload_type = 0x0201,
        .payload_length = 0x01,
        .seq_number = seq_number
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

static void log_p5t4(uint8_t p[5], uint8_t t[4])
{
    log("%02x%02x%02x%02x%02x %02x%02x%02x%02x", p[0], p[1], p[2], p[3], p[4],
            t[0], t[1], t[2], t[3]);
}

static void directionals(int vert, int horiz, uint8_t pan_speed, uint8_t tilt_speed)
{
    log("directionals: % d, % d, (pan %d, tilt %d)", vert, horiz, pan_speed, tilt_speed);
}

static void relative_move(uint8_t speed, uint8_t p[5], uint8_t t[4])
{
    log("relative_move: %d,", speed);
    log_p5t4(p, t);
}

static void absolute_move(uint8_t speed, uint8_t p[5], uint8_t t[4])
{
    log("absolute_move: %d,", speed);
    log_p5t4(p, t);
}

static void home()
{
    log("home");
}

static void reset()
{
    log("reset");
}

static void pan_tilt_limit_set(uint8_t position, uint8_t p[5], uint8_t t[4])
{
    log("pan_tilt_limit_set: %d,", position);
    log_p5t4(p, t);
}

static void pan_tilt_limit_clear(uint8_t position)
{
    log("pan_tilt_limit_clear: %d", position);
}

static void ramp_curve(int p)
{
    log("ramp_curve: %d", p);
}

static void slow_mode(int p)
{
    log("slow_mode: %d", p);
}

static void ptd_directionals(const uint8_t *payload, size_t length, uint32_t seq_number)
{
    uint8_t pan_speed, tilt_speed;
    int vert, horiz;

    check_length(9);

    pan_speed = payload[4];
    tilt_speed = payload[5];

    switch (payload[6]) {
        case 0x01:
            horiz = -1;
            break;
        case 0x02:
            horiz = 1;
            break;
        case 0x03:
            horiz = 0;
            break;
        default:
            log("ptd_directionals: unexpected horizontal drive 0x%02x", payload[6]);
            return;
    }

    switch (payload[7]) {
        case 0x01:
            vert = 1;
            break;
        case 0x02:
            vert = -1;
            break;
        case 0x03:
            vert = 0;
            break;
        default:
            log("ptd_directionals: unexpected vertical drive 0x%02x", payload[7]);
            return;
    }

    directionals(vert, horiz, pan_speed, tilt_speed);
}

static void ptd_abs_rel(const uint8_t *payload, size_t length, uint32_t seq_number, int rel)
{
    uint8_t speed, p[5], t[4];

    check_length(16);

    speed = payload[4];

    if (payload[5] != 0) {
        log("ptd_abs_rel: expected payload[5] to be 0, not 0x%02x", payload[5]);
        return;
    }

    for (int i = 0; i < 5; ++i) {
        p[i] = payload[6 + i];
    }

    for (int i = 0; i < 4; ++i) {
        t[i] = payload[11 + i];
    }

    if (rel) {
        relative_move(speed, p, t);
    } else {
        absolute_move(speed, p, t);
    }
}

static void ptd_pan_tilt_limit(const uint8_t *payload, size_t length, uint32_t seq_number)
{
    int set, position;
    uint8_t p[5], t[4];

    check_length(16);

    set = payload[4];

    if (set != 0 && set != 1) {
        log("ptd_pan_tilt_limit: unexpected set byte 0x%02x", set);
    }

    position = payload[5];

    if (set == 1) {
        for (int i = 0; i < 5; ++i) {
            p[i] = payload[6 + i];
        }

        for (int i = 0; i < 4; ++i) {
            t[i] = payload[11 + i];
        }

        pan_tilt_limit_set(position, p, t);
    } else {
        pan_tilt_limit_clear(position);
    }
}

static void ptd_ramp_curve(const uint8_t *payload, size_t length, uint32_t seq_number)
{
    int p;

    check_length(6);

    p = payload[4];

    if (p != 1 && p != 2 && p != 3) {
        log("ptd_ramp_curve: unexpected p %d", p);
        return;
    }

    ramp_curve(p);
}

static void ptd_slow_mode(const uint8_t *payload, size_t length, uint32_t seq_number)
{
    int p;

    check_length(6);

    p = payload[4];

    if (p != 2 && p != 3) {
        log("ptd_slow_mode: unexpected p %d", p);
        return;
    }

    slow_mode(p);
}

static void dispatch_pan_tilt_drive(const uint8_t *payload, size_t length, uint32_t seq_number)
{
    switch (payload[3]) {
        case 0x01: /* directionals or stop */
            ptd_directionals(payload, length, seq_number);
            break;
        case 0x02: /* absolute position */
            ptd_abs_rel(payload, length, seq_number, 0);
            break;
        case 0x03: /* relative position */
            ptd_abs_rel(payload, length, seq_number, 1);
            break;
        case 0x04: /* home */
            home();
            break;
        case 0x05: /* reset */
            reset();
            break;
        case 0x07: /* pan tilt limit */
            ptd_pan_tilt_limit(payload, length, seq_number);
            break;
        case 0x31: /* ramp curve */
            ptd_ramp_curve(payload, length, seq_number);
            break;
        case 0x44: /* pan-tilt slow mode */
            ptd_slow_mode(payload, length, seq_number);
            break;
        default:
            log("dispatch_pan_tilt_drive: unexpected type 0x%02x", payload[3]);
    }
}

static void handle_visca_command(const uint8_t *payload, size_t length, uint32_t seq_number)
{
    log("handle_visca_command");

    if (length < 5) {
        log("handle_control_command: bad length %zu", length);
        return;
    }

    if (payload[0] != 0x81 || payload[1] != 0x01) {
        log("handle_control_command: unexpected payload start %02x %02x", payload[0], payload[1]);
        return;
    }

    switch (payload[2]) {
        case 0x06:
            dispatch_pan_tilt_drive(payload, length, seq_number);
            break;
        default:
            log("handle_control_command: unsupported command 0x%02x", payload[2]);
    }
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
    log("handle_control_command");

    switch (payload[0]) {
        case 0x01: /* RESET */
            log("control command RESET");
            compose_control_reply(g_response, &g_response_len, 0);
            break;
        case 0x0F: /* ERROR */
            log("control command ERROR");

            check_length(2);

            switch (payload[1]) {
                case 0x01:
                    log("abnormality in the sequence number");
                    break;
                case 0x02:
                    log("abnormality in the message type");
                    break;
                default:
                    log("handle_control_command: ERROR: unexpected error type 0x%02x", payload[1]);
                    return;
            }

            compose_control_reply(g_response, &g_response_len, seq_number);
            break;
        default:
            log("handle_control_command: unexpected control command type 0x%02x", payload[0]);
    }
}

static void handle_control_reply(const uint8_t *payload, size_t length, uint32_t seq_number)
{
    (void)payload;
    (void)length;
    (void)seq_number;

    log("handle_control_reply");
}

void visca_handle_message(const uint8_t *message, size_t length, uint8_t *response, size_t *response_len)
{
    struct visca_header *header = (struct visca_header*)message;
    const uint8_t *payload = message + 8;
    size_t payload_length = length - 8;

    log("got msg:");
    print_buffer(message, length, 16);
    print_buffer(message, length, 2);
    log(" ");

    /* visca_header_convert_endianness(header); */

    log("header->payload_type=0x%04x", header->payload_type);
    log("header->payload_length=%d", header->payload_length);
    log("header->seq_number=%d", header->seq_number);

    if (header->payload_length != length - 8) {
        log("assertion `header->payload_length == length - 8' failed: %d != %zu",
                header->payload_length, length - 8);
        return;
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
        default:
            log("visca_handle_message: unexpected payload type 0x%04x", header->payload_type);
    }
}

/* Command ->
       <- Ack             OR    <- Ack
       <- Completition          <- Error

   Inquiry ->
       <- Completition (+ inquiry result data)    OR    <- Error
 */

