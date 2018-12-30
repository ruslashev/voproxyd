#include "bridge.h"
#include "buffer.h"
#include "errors.h"
#include "log.h"
#include "visca.h"

#include <netdb.h>
#include <string.h>

// redefine die() because all errors here are visca protocol errors
#undef die
#define die(...) die_detail(ERR_VISCA_PROTOCOL, __VA_ARGS__)

#define check_length_detail(X, R) \
    if (payload->length != (X)) { \
        log("%s: bad length %zu, expected %d", __func__, payload->length, X); \
        return R; \
    }

#define check_length_null(X) check_length_detail(X, NULL)
#define check_length(X) check_length_detail(X, )

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

static struct buffer_t* compose_ack()
{
    struct buffer_t *response = cons_buffer(3);

    response->data[0] = 0x90;
    response->data[1] = 0x40;
    response->data[2] = 0xff;

    return response;
}

static struct buffer_t* compose_completition(const uint8_t data[], size_t data_len)
{
    struct buffer_t *response = cons_buffer(3 + data_len);

    response->data[0] = 0x90;
    response->data[1] = 0x50;

    for (size_t i = 0; i < data_len; ++i) {
        response->data[2 + i] = data[i];
    }

    response->data[2 + data_len] = 0xff;

    return response;
}

static struct buffer_t* compose_empty_completition()
{
    return compose_completition(NULL, 0);
}

static struct buffer_t* compose_control_reply(uint32_t seq_number)
{
    struct buffer_t *response = cons_buffer(VOIP_HEADER_LENGTH + 1);
    struct visca_header header = {
        .payload_type = 0x0201,
        .payload_length = 0x01,
        .seq_number = seq_number
    };

    visca_header_convert_endianness_hton(&header);

    memcpy(response->data, &header, VOIP_HEADER_LENGTH);

    response->data[VOIP_HEADER_LENGTH] = 0x01; /* ACK: reply for RESET */

    return response;
}

static void ptd_directionals(const struct buffer_t *payload)
{
    uint8_t pan_speed, tilt_speed;
    int vert, horiz;

    check_length(9);

    pan_speed = payload->data[4];
    tilt_speed = payload->data[5];

    switch (payload->data[6]) {
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
            log("ptd_directionals: unexpected horizontal drive 0x%02x", payload->data[6]);
            return;
    }

    switch (payload->data[7]) {
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
            log("ptd_directionals: unexpected vertical drive 0x%02x", payload->data[7]);
            return;
    }

    bridge_directionals(vert, horiz, pan_speed, tilt_speed);
}

static void ptd_abs_rel(const struct buffer_t *payload, int rel)
{
    uint8_t speed, p[5], t[4];

    check_length(16);

    speed = payload->data[4];

    if (payload->data[5] != 0) {
        log("ptd_abs_rel: expected payload->data[5] to be 0, not 0x%02x", payload->data[5]);
        return;
    }

    for (int i = 0; i < 5; ++i) {
        p[i] = payload->data[6 + i];
    }

    for (int i = 0; i < 4; ++i) {
        t[i] = payload->data[11 + i];
    }

    if (rel) {
        bridge_relative_move(speed, p, t);
    } else {
        bridge_absolute_move(speed, p, t);
    }
}

static void ptd_pan_tilt_limit(const struct buffer_t *payload)
{
    int set, position;
    uint8_t p[5], t[4];

    check_length(16);

    set = payload->data[4];

    if (set != 0 && set != 1) {
        log("ptd_pan_tilt_limit: unexpected set byte 0x%02x", set);
    }

    position = payload->data[5];

    if (set == 1) {
        for (int i = 0; i < 5; ++i) {
            p[i] = payload->data[6 + i];
        }

        for (int i = 0; i < 4; ++i) {
            t[i] = payload->data[11 + i];
        }

        bridge_pan_tilt_limit_set(position, p, t);
    } else {
        bridge_pan_tilt_limit_clear(position);
    }
}

static void ptd_ramp_curve(const struct buffer_t *payload)
{
    int p;

    check_length(6);

    p = payload->data[4];

    if (p != 1 && p != 2 && p != 3) {
        log("ptd_ramp_curve: unexpected p %d", p);
        return;
    }

    bridge_ramp_curve(p);
}

static void ptd_slow_mode(const struct buffer_t *payload)
{
    int p;

    check_length(6);

    p = payload->data[4];

    if (p != 2 && p != 3) {
        log("ptd_slow_mode: unexpected p %d", p);
        return;
    }

    bridge_slow_mode(p);
}

static struct buffer_t* dispatch_pan_tilt_drive(const struct buffer_t *payload, uint32_t seq_number)
{
    switch (payload->data[3]) {
        case 0x01: /* directionals or stop */
            ptd_directionals(payload);
            break;
        case 0x02: /* absolute position */
            ptd_abs_rel(payload, 0);
            break;
        case 0x03: /* relative position */
            ptd_abs_rel(payload, 1);
            break;
        case 0x04: /* home */
            bridge_home();
            break;
        case 0x05: /* reset */
            bridge_reset();
            break;
        case 0x07: /* pan tilt limit */
            ptd_pan_tilt_limit(payload);
            break;
        case 0x31: /* ramp curve */
            ptd_ramp_curve(payload);
            break;
        case 0x44: /* pan-tilt slow mode */
            ptd_slow_mode(payload);
            break;
        default:
            log("dispatch_pan_tilt_drive: unexpected type 0x%02x", payload->data[3]);
            return NULL;
    }

    (void)seq_number;
    return NULL;
}

static struct buffer_t* handle_visca_command(const struct buffer_t *payload, uint32_t seq_number)
{
    log("handle_visca_command");

    if (payload->length < 5) {
        log("handle_visca_command: bad length %zu", payload->length);
        return NULL;
    }

    if (payload->data[0] != 0x81 || payload->data[1] != 0x01) {
        log("handle_visca_command: unexpected payload start %02x %02x",
                payload->data[0], payload->data[1]);
        return NULL;
    }

    switch (payload->data[2]) {
        case 0x06:
            return dispatch_pan_tilt_drive(payload, seq_number);
        default:
            log("handle_visca_command: unsupported command 0x%02x", payload->data[2]);
            return NULL;
    }
}

static struct buffer_t* handle_visca_inquiry(const struct buffer_t *payload, uint32_t seq_number)
{
    (void)payload;
    (void)seq_number;

    log("handle_visca_inquiry");

    return NULL;
}

static struct buffer_t* handle_visca_reply(const struct buffer_t *payload, uint32_t seq_number)
{
    (void)payload;
    (void)seq_number;

    log("handle_visca_reply");

    return NULL;
}

static struct buffer_t* handle_visca_device_setting_cmd(const struct buffer_t *payload,
        uint32_t seq_number)
{
    (void)payload;
    (void)seq_number;

    log("handle_visca_device_setting_cmd");

    return NULL;
}

static struct buffer_t* handle_control_command(const struct buffer_t *payload, uint32_t seq_number)
{
    log("handle_control_command");

    switch (payload->data[0]) {
        case 0x01: /* RESET */
            log("control command RESET");
            seq_number = 0;
            break;
        case 0x0F: /* ERROR */
            log("control command ERROR");

            check_length_null(2);

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
                    return NULL;
            }

            break;
        default:
            log("handle_control_command: unexpected control command type 0x%02x", payload->data[0]);
            return NULL;
    }

    return compose_control_reply(seq_number);
}

static struct buffer_t* handle_control_reply(const struct buffer_t *payload, uint32_t seq_number)
{
    (void)payload;
    (void)seq_number;

    log("handle_control_reply");

    return NULL;
}

struct buffer_t* visca_handle_message(const struct buffer_t *message)
{
    struct visca_header *header = (struct visca_header*)message->data;
    struct buffer_t payload = {
        .length = message->length - 8,
        .data = message->data + 8
    };

    log("got msg:");
    print_buffer(message, 16);

    visca_header_convert_endianness_ntoh(header);

    log("header->payload_type=0x%04x", header->payload_type);
    log("header->payload_length=%d", header->payload_length);
    log("header->seq_number=%d", header->seq_number);

    if (header->payload_length != payload.length) {
        log("assertion `header->payload_length == payload.length' failed: %d != %zu",
                header->payload_length, payload.length);
        return NULL;
    }

    switch (header->payload_type) {
        case 0x0100:
            return handle_visca_command(&payload, header->seq_number);
        case 0x0110:
            return handle_visca_inquiry(&payload, header->seq_number);
        case 0x0111:
            return handle_visca_reply(&payload, header->seq_number);
        case 0x0120:
            return handle_visca_device_setting_cmd(&payload, header->seq_number);
        case 0x0200:
            return handle_control_command(&payload, header->seq_number);
        case 0x0201:
            return handle_control_reply(&payload, header->seq_number);
        default:
            log("visca_handle_message: unexpected payload type 0x%04x", header->payload_type);
            return NULL;
    }
}

/* Command ->
       <- Ack             OR    <- Ack
       <- Completition          <- Error

   Inquiry ->
       <- Completition (+ inquiry result data)    OR    <- Error
 */

