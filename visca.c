#include "visca.h"
#include "log.h"
#include "bridge_commands.h"
#include "bridge_inquiries.h"

#undef die
#define die(...) die_detail(ERR_VISCA_PROTOCOL, __VA_ARGS__)

static uint64_t parse_retarded_integer_encoding(const buffer_t *message, size_t start, size_t n)
{
    uint64_t output = 0;

    for (size_t i = 0; i < n; ++i)
        output = (output << 4u) | (message->data[start + i] & 0x0fu);

    return output;
}

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

static void dispatch_commands_04(const buffer_t *message, const struct event_t *event)
{
    uint8_t b4 = message->data[4], b5 = message->data[5];

    switch (message->data[3]) {
    case 0x03:
        switch (message->data[4]) {
        case 0x00:
            bridge_cmd_color_rgain_reset();
            break;
        case 0x02:
            bridge_cmd_color_rgain_up();
            break;
        case 0x03:
            bridge_cmd_color_rgain_down();
            break;
        }
        break;
    case 0x04:
        switch (message->data[4]) {
        case 0x00:
            bridge_cmd_color_bgain_reset();
            break;
        case 0x02:
            bridge_cmd_color_bgain_up();
            break;
        case 0x03:
            bridge_cmd_color_bgain_down();
            break;
        }
        break;
    case 0x07:
        switch (message->data[4]) {
        case 0x00:
            bridge_cmd_zoom_stop();
            break;
        case 0x02:
            bridge_cmd_zoom_tele();
            break;
        case 0x03:
            bridge_cmd_zoom_wide();
            break;
        default:
            if ((b4 & 0xf0) == 0x20) {
                b4 = b4 & 0x0f;
                bridge_cmd_zoom_tele_var(b4);
            } else if ((b4 & 0xf0) == 0x30) {
                b4 = b4 & 0x0f;
                bridge_cmd_zoom_wide_var(b4);
            } else {
                bad_byte(4);
            }
        }
        break;
    case 0x08:
        switch (message->data[4]) {
        case 0x00:
            bridge_cmd_focus_stop();
            break;
        case 0x02:
            bridge_cmd_focus_far();
            break;
        case 0x03:
            bridge_cmd_focus_near();
            break;
        default:
            if ((b4 & 0xf0) == 2) {
                b4 = b4 & 0x0f;
                bridge_cmd_focus_far_var(b4);
            } else if ((b4 & 0xf0) == 3) {
                b4 = b4 & 0x0f;
                bridge_cmd_focus_near_var(b4);
            } else {
                bad_byte(4);
            }
        }
        break;
    case 0x0A:
        switch (message->data[4]) {
        case 0x00:
            bridge_cmd_exposure_shutter_reset();
            break;
        case 0x02:
            bridge_cmd_exposure_shutter_fast();
            break;
        case 0x03:
            bridge_cmd_exposure_shutter_slow();
            break;
        }
        break;
    case 0x0B:
        switch (message->data[4]) {
        case 0x00:
            bridge_cmd_exposure_iris_reset();
            break;
        case 0x02:
            bridge_cmd_exposure_iris_up();
            break;
        case 0x03:
            bridge_cmd_exposure_iris_down();
            break;
        }
        break;
    case 0x23:
        bridge_cmd_flicker_reduction_mode_set(b4);
        break;
    case 0x33:
        switch (message->data[4]) {
        case 0x02:
            bridge_cmd_exposure_back_light_set(1);
            break;
        case 0x03:
            bridge_cmd_exposure_back_light_set(0);
            break;
        }
        break;
    case 0x38:
        switch (message->data[4]) {
        case 0x02:
            bridge_cmd_focus_mode_auto();
            break;
        case 0x03:
            bridge_cmd_focus_mode_auto();
            break;
        case 0x10:
            bridge_cmd_focus_mode_toggle();
            break;
        }
        break;
    case 0x39:
        switch (message->data[4]) {
        case 0x00:
            bridge_cmd_exposure_mode_full_auto();
            break;
        case 0x03:
            bridge_cmd_exposure_mode_manual();
            break;
        case 0x0A:
            bridge_cmd_exposure_mode_shutter_pri();
            break;
        case 0x0B:
            bridge_cmd_exposure_mode_iris_pri();
            break;
        case 0x0D:
            bridge_cmd_exposure_mode_gain_pri();
            break;
        }
        break;
    case 0x3F:
        switch (message->data[4]) {
        case 0x00:
            bridge_cmd_memory_reset(b5);
            break;
        case 0x01:
            bridge_cmd_memory_set(b5);
            break;
        case 0x02:
            bridge_cmd_memory_recall(b5);
            break;
        }
        break;
    case 0x47:
        bridge_cmd_zoom_direct(parse_retarded_integer_encoding(message, 4, 4));
        break;
    case 0x48:
        bridge_cmd_focus_direct(parse_retarded_integer_encoding(message, 4, 4));
        break;
    case 0x4A:
        bridge_cmd_exposure_shutter_direct(parse_retarded_integer_encoding(message, 6, 2));
        break;
    case 0x4B:
        bridge_cmd_exposure_iris_direct(parse_retarded_integer_encoding(message, 6, 2));
        break;
    case 0x66:
        switch (message->data[4]) {
        case 0x02:
            bridge_cmd_system_img_flip(1);
            break;
        case 0x03:
            bridge_cmd_system_img_flip(0);
            break;
        }
        break;
    }
}

static void ptd_abs_rel(const buffer_t *message, int rel)
{
    uint8_t pan_speed, tilt_speed;
    uint64_t pan_pos, tilt_pos;

    pan_speed = message->data[4];
    tilt_speed = message->data[5];

    pan_pos = parse_retarded_integer_encoding(message, 6, 4);
    tilt_pos = parse_retarded_integer_encoding(message, 10, 4);

    if (rel)
        bridge_cmd_pan_tilt_relative_position(pan_speed, tilt_speed, pan_pos, tilt_pos);
    else
        bridge_cmd_pan_tilt_absolute_position(pan_speed, tilt_speed, pan_pos, tilt_pos);
}

static void dispatch_commands_06(const buffer_t *message, const struct event_t *event)
{
    switch (message->data[3]) {
    case 0x01: {
        uint8_t pan_speed, tilt_speed;
        int vert, horiz;

        check_length(9);

        pan_speed = message->data[4];
        tilt_speed = message->data[5];

        switch (message->data[6]) {
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
            log("ptd_directionals: unexpected horizontal drive 0x%02x", message->data[6]);
            return;
        }

        switch (message->data[7]) {
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
            log("ptd_directionals: unexpected vertical drive 0x%02x", message->data[7]);
            return;
        }

        bridge_cmd_pan_tilt_directionals(vert, horiz, pan_speed, tilt_speed);
        break;
    }
    case 0x02:
        ptd_abs_rel(message, 0);
        break;
    case 0x03:
        ptd_abs_rel(message, 1);
        break;
    case 0x04:
        bridge_cmd_pan_tilt_home();
        break;
    case 0x05:
        bridge_cmd_pan_tilt_reset();
        break;
    default:
        bad_byte(3);
    }
}

static void dispatch_commands(const buffer_t *message, const struct event_t *event)
{
    switch (message->data[2]) {
    case 0x04:
        dispatch_commands_04(message, event);
        break;
    case 0x06:
        dispatch_commands_06(message, event);
        break;
    }
}

static buffer_t* dispatch_queries_04(const buffer_t *message)
{
    switch (message->data[3]) {
    case 0x47:
        bridge_inq_zoom_pos();
        return cons_buffer(4);
    case 0x38:
        bridge_inq_focus_af_mode();
        return cons_buffer(1);
    case 0x48:
        bridge_inq_focus_pos();
        return cons_buffer(4);
    case 0x35:
        bridge_inq_wb_mode();
        return cons_buffer(1);
    case 0x43:
        bridge_inq_r_gain();
        return cons_buffer(4);
    case 0x39:
        bridge_inq_ae_mode();
        return cons_buffer(1);
    case 0x4A:
        bridge_inq_shutter_pos();
        return cons_buffer(4);
    case 0x4B:
        bridge_inq_iris_pos();
        return cons_buffer(4);
    case 0x4D:
        bridge_inq_bright_pos();
        return cons_buffer(4);
    case 0x3E:
        bridge_inq_exp_comp_mode();
        return cons_buffer(1);
    case 0x4E:
        bridge_inq_exp_comp_pos();
        return cons_buffer(4);
    case 0x33:
        bridge_inq_backlight_mode();
        return cons_buffer(1);
    case 0x50:
        bridge_inq_noise2_d_mode();
        return cons_buffer(1);
    case 0x53:
        bridge_inq_noise2_d_l();
        return cons_buffer(1);
    case 0x54:
        bridge_inq_noise3_d_l();
        return cons_buffer(1);
    case 0x55:
        bridge_inq_flicker_mode();
        return cons_buffer(1);
    case 0x05:
        bridge_inq_aperture_mode();
        return cons_buffer(1);
    case 0x42:
        bridge_inq_aperture();
        return cons_buffer(4);
    case 0x63:
        bridge_inq_picture_effect_mode();
        return cons_buffer(1);
    case 0x61:
        bridge_inq_lr_reverse();
        return cons_buffer(1);
    case 0x66:
        bridge_inq_picture_flip();
        return cons_buffer(1);
    case 0x49:
        bridge_inq_color_gain();
        return cons_buffer(4);
    case 0x2C:
        bridge_inq_gain_limit();
        return cons_buffer(1);
    case 0xA4:
        bridge_inq_flip();
        return cons_buffer(1);
    case 0xAA:
        bridge_inq_af_zone();
        return cons_buffer(1);
    case 0x4F:
        bridge_inq_color_hue();
        return cons_buffer(4);
    case 0xA9:
        bridge_inq_awb_sensitivity();
        return cons_buffer(1);
    default:
        bad_byte_null(3);
    }
}

static buffer_t* dispatch_queries(const buffer_t *message)
{
    switch (message->data[2]) {
    case 0x06:
        if (message->data[3] != 0x12)
            bad_byte(3);
        return bridge_inq_pan_tilt_position();
        break;
    case 0x04:
        return dispatch_queries_04(message);
    default:
        bad_byte_null(2);
    }
}

void visca_handle_message(const buffer_t *message, const struct event_t *event)
{
    buffer_t *response, *inquiry_data;

    if (message->data[0] != 0x81)
        bad_byte(0);

    switch (message->data[1]) {
    case 0x01:
        log("visca: handle command");

        response = compose_ack();
        visca_send_response(event, response);
        free(response);

        dispatch_commands(message, event);

        response = compose_empty_completition();
        visca_send_response(event, response);
        free(response);

        break;
    case 0x09:
        log("visca: handle inquiry");

        inquiry_data = dispatch_queries(message);

        if (inquiry_data == NULL) {
            log("visca_handle_message: empty inquiry data");
            return;
        }

        response = compose_completition(inquiry_data);
        visca_send_response(event, response);
        free(response);
        free(inquiry_data);

        break;
    default:
        bad_byte(1);
    }
}

