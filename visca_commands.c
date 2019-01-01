#include "bridge.h"
#include "log.h"
#include "visca.h"
#include "visca_inquiries.h"

static uint64_t parse_retarded_integer_encoding(const buffer_t *payload, size_t start, size_t n)
{
    uint64_t output = 0;

    for (size_t i = 0; i < n; ++i) {
        output = (output << 4u) | (payload->data[start + i] & 0x0f);
    }

    return output;
}

static int gain_parameter_to_db(uint8_t p)
{
    switch (p) {
        case 0x0c: return 33;
        case 0x0b: return 30;
        case 0x0a: return 27;
        case 0x09: return 24;
        case 0x08: return 21;
        case 0x07: return 18;
        case 0x06: return 15;
        case 0x05: return 12;
        case 0x04: return 9;
        case 0x03: return 6;
        case 0x02: return 3;
        case 0x01: return 0;
        case 0x00: return -3;
        default:
            log("gain_parameter_to_db: invalid byte 0x%02x", p);
            return 0;
    }
}

static void hdmi_parameter_to_specs(uint8_t p, int *w, int *h, int *f, char *l)
{
    int params[][4] = {
        [0x00] = { 1920, 1080, 59, 'p' },
        [0x02] = { 1920, 1080, 29, 'p' },
        [0x03] = { 1920, 1080, 59, 'i' },
        [0x04] = { 1280,  720, 59, 'p' },
        [0x08] = { 1920, 1080, 50, 'p' },
        [0x0a] = { 1920, 1080, 25, 'p' },
        [0x0b] = { 1920, 1080, 50, 'i' },
        [0x0c] = { 1280,  720, 50, 'p' },
        [0x18] = {  640,  480, 59, 'p' },
        [0x22] = { 3840, 2160, 29, 'p' },
        [0x26] = { 3840, 2160, 25, 'p' },
        [0x28] = { 1920, 1080, 23, 'p' },
        [0x2a] = { 3840, 2160, 23, 'p' }
    };

    switch (p) {
        case 0x00:
        case 0x02:
        case 0x03:
        case 0x04:
        case 0x08:
        case 0x0a:
        case 0x0b:
        case 0x0c:
        case 0x18:
        case 0x22:
        case 0x26:
        case 0x28:
        case 0x2a:
            *w = params[p][0];
            *h = params[p][1];
            *f = params[p][2];
            *l = params[p][3];
            break;
        default:
            *w = *h = *f = *l = 0;
    }
}

static void dispatch_exposure_mode(const buffer_t *payload, uint32_t seq_number, buffer_t *response)
{
    switch (payload->data[4]) {
        case 0x00:
            bridge_cmd_exposure_mode_full_auto();
            break;
        case 0x03:
            bridge_cmd_exposure_mode_manual();
            break;
        case 0x0a:
            bridge_cmd_exposure_mode_shutter_pri();
            break;
        case 0x0b:
            bridge_cmd_exposure_mode_iris_pri();
            break;
        case 0x0e:
            bridge_cmd_exposure_mode_gain_pri();
            break;
        default:
            bad_byte(4);
    }
}

static void dispatch_exposure_iris(const buffer_t *payload, uint32_t seq_number, buffer_t *response)
{
    switch (payload->data[4]) {
        case 0x00:
            bridge_cmd_exposure_iris_reset();
            break;
        case 0x02:
            bridge_cmd_exposure_iris_up();
            break;
        case 0x03:
            bridge_cmd_exposure_iris_down();
            break;
        default:
            bad_byte(4);
    }
}

static void dispatch_exposure_gain(const buffer_t *payload, uint32_t seq_number, buffer_t *response)
{
    switch (payload->data[4]) {
        case 0x00:
            bridge_cmd_exposure_gain_reset();
            break;
        case 0x02:
            bridge_cmd_exposure_gain_up();
            break;
        case 0x03:
            bridge_cmd_exposure_gain_down();
            break;
        default:
            bad_byte(4);
    }
}

static void dispatch_exposure_shutter(const buffer_t *payload, uint32_t seq_number, buffer_t *response)
{
    switch (payload->data[4]) {
        case 0x00:
            bridge_cmd_exposure_shutter_reset();
            break;
        case 0x02:
            bridge_cmd_exposure_shutter_fast();
            break;
        case 0x03:
            bridge_cmd_exposure_shutter_slow();
            break;
        default:
            bad_byte(4);
    }
}

static void dispatch_exposure_exp_comp(const buffer_t *payload, uint32_t seq_number, buffer_t *response)
{
    switch (payload->data[4]) {
        case 0x00:
            bridge_cmd_exposure_exp_comp_reset();
            break;
        case 0x02:
            bridge_cmd_exposure_exp_comp_up();
            break;
        case 0x03:
            bridge_cmd_exposure_exp_comp_down();
            break;
        default:
            bad_byte(4);
    }
}

static void dispatch_color_white_balance(const buffer_t *payload, uint32_t seq_number, buffer_t *response)
{
    switch (payload->data[4]) {
        case 0x00:
            bridge_cmd_color_white_balance_auto1();
            break;
        case 0x01:
            bridge_cmd_color_white_balance_indoor();
            break;
        case 0x02:
            bridge_cmd_color_white_balance_outdoor();
            break;
        case 0x03:
            bridge_cmd_color_white_balance_one_push_wb();
            break;
        case 0x04:
            bridge_cmd_color_white_balance_auto2();
            break;
        case 0x05:
            bridge_cmd_color_white_balance_manual();
            break;
        default:
            bad_byte(4);
    }
}

static void dispatch_04(const buffer_t *payload, uint32_t seq_number, buffer_t *response)
{
    uint64_t p, q, r;
    uint8_t b4 = payload->data[4], onoff = (b4 == 2) ? 1 : 0;

    switch (payload->data[3]) {
        case 0x39:
            dispatch_exposure_mode(payload, seq_number, response);
            break;
        case 0x0b:
            dispatch_exposure_iris(payload, seq_number, response);
            break;
        case 0x4b:
            p = parse_retarded_integer_encoding(payload, 6, 2);
            bridge_cmd_exposure_iris_direct(p);
            break;
        case 0x0c:
            dispatch_exposure_gain(payload, seq_number, response);
            break;
        case 0x4c:
            p = parse_retarded_integer_encoding(payload, 6, 2);
            bridge_cmd_exposure_gain_direct(gain_parameter_to_db(p));
            break;
        case 0x2c:
            bridge_cmd_exposure_gain_limit(gain_parameter_to_db(b4));
            break;
        case 0x0a:
            dispatch_exposure_shutter(payload, seq_number, response);
            break;
        case 0x4a:
            p = parse_retarded_integer_encoding(payload, 6, 2);
            bridge_cmd_exposure_shutter_direct(p);
            break;
        case 0x5d:
            bridge_cmd_exposure_ae_speed(b4);
            break;
        case 0x3e:
            bridge_cmd_exposure_exp_comp_set(onoff);
            break;
        case 0x0e:
            dispatch_exposure_exp_comp(payload, seq_number, response);
            break;
        case 0x4e:
            p = parse_retarded_integer_encoding(payload, 6, 2);
            bridge_cmd_exposure_exp_comp_direct(p);
            break;
        case 0x33:
            bridge_cmd_exposure_back_light_set(onoff);
            break;
        case 0x3a:
            bridge_cmd_exposure_spot_light_set(onoff);
            break;
        case 0x3d:
            bridge_cmd_exposure_vis_enh_set(b4 == 0x06 ? 1 : 0);
            break;
        case 0x2d:
            p = payload->data[5];
            q = payload->data[6];
            r = payload->data[7];
            bridge_cmd_exposure_vis_enh_direct(p, q, r);
            break;
        case 0x01:
            bridge_cmd_exposure_ir_cut_filter_set(onoff);
            break;
        case 0x35:
            dispatch_color_white_balance(payload, seq_number, response);
            break;
        case 0x10:
            bridge_cmd_color_one_push_trigger();
            break;
        case 0x03:
            switch (payload->data[4]) {
                case 0x00:
                    bridge_cmd_color_rgain_reset();
                    break;
                case 0x02:
                    bridge_cmd_color_rgain_up();
                    break;
                case 0x03:
                    bridge_cmd_color_rgain_down();
                    break;
                default:
                    bad_byte(4);
            }
            break;
        case 0x43:
            p = parse_retarded_integer_encoding(payload, 6, 2);
            bridge_cmd_color_rgain_direct((int)p - 128);
            break;
        case 0x04:
            switch (payload->data[4]) {
                case 0x00:
                    bridge_cmd_color_bgain_reset();
                    break;
                case 0x02:
                    bridge_cmd_color_bgain_up();
                    break;
                case 0x03:
                    bridge_cmd_color_bgain_down();
                    break;
                default:
                    bad_byte(4);
            }
            break;
        case 0x44:
            p = parse_retarded_integer_encoding(payload, 6, 2);
            bridge_cmd_color_bgain_direct((int)p - 128);
            break;
        case 0x56:
            bridge_cmd_color_speed(b4);
            break;
        case 0x5f:
            bridge_cmd_color_chroma_suppress(b4);
            break;
        case 0x09:
            switch (payload->data[4]) {
                case 0x00:
                    bridge_cmd_color_level_reset();
                    break;
                case 0x02:
                    bridge_cmd_color_level_up();
                    break;
                case 0x03:
                    bridge_cmd_color_level_down();
                    break;
                default:
                    bad_byte(4);
            }
            break;
        case 0x49:
            bridge_cmd_color_level_direct(payload->data[7]);
            break;
        case 0x0f:
            switch (payload->data[4]) {
                case 0x00:
                    bridge_cmd_color_phase_reset();
                    break;
                case 0x02:
                    bridge_cmd_color_phase_up();
                    break;
                case 0x03:
                    bridge_cmd_color_phase_down();
                    break;
                default:
                    bad_byte(4);
            }
            break;
        case 0x4f:
            bridge_cmd_color_phase_direct(payload->data[7]);
            break;
        case 0x02:
            switch (payload->data[4]) {
                case 0x00:
                    bridge_cmd_detail_level_reset();
                    break;
                case 0x02:
                    bridge_cmd_detail_level_up();
                    break;
                case 0x03:
                    bridge_cmd_detail_level_down();
                    break;
                default:
                    bad_byte(4);
            }
            break;
        case 0x42:
            p = parse_retarded_integer_encoding(payload, 6, 2);
            bridge_cmd_detail_level_direct(p);
            break;
        case 0x5b:
            bridge_cmd_gamma_mode(b4);
            break;
        case 0x1e:
            p = payload->data[7];
            q = parse_retarded_integer_encoding(payload, 8, 2);
            bridge_cmd_gamma_offset(p, q);
            break;
        case 0x32:
            bridge_cmd_flicker_reduction_mode_set(onoff);
            break;
        case 0x53:
            bridge_cmd_noise_reduction_mode_level_set(b4);
            break;
        case 0x07:
            switch (payload->data[4]) {
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
                    if ((b4 & 0xf0) == 2) {
                        b4 = b4 & 0x0f;
                        bridge_cmd_zoom_tele_var(b4);
                    } else if ((b4 & 0xf0) == 3) {
                        b4 = b4 & 0x0f;
                        bridge_cmd_zoom_wide_var(b4);
                    } else {
                        bad_byte(4);
                    }
            }
            break;
        case 0x47:
            p = parse_retarded_integer_encoding(payload, 4, 4);
            bridge_cmd_zoom_direct(p);
            break;
        case 0x06:
            bridge_cmd_zoom_clear_image_set(b4 == 0x04 ? 1 : 0);
            break;
        case 0x38:
            switch (payload->data[4]) {
                case 0x02:
                    bridge_cmd_focus_mode_auto();
                    break;
                case 0x03:
                    bridge_cmd_focus_mode_manual();
                    break;
                case 0x10:
                    bridge_cmd_focus_mode_toggle();
                    break;
                default:
                    bad_byte(4);
            }
            break;
        case 0x08:
            switch (payload->data[4]) {
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
        case 0x48:
            p = parse_retarded_integer_encoding(payload, 4, 4);
            bridge_cmd_focus_direct(p);
            break;
        case 0x18:
            switch (payload->data[4]) {
                case 0x01:
                    bridge_cmd_focus_one_push_trigger();
                    break;
                case 0x02:
                    bridge_cmd_focus_focus_inf();
                    break;
                default:
                    bad_byte(4);
            }
            break;
        case 0x28:
            p = parse_retarded_integer_encoding(payload, 4, 4);
            bridge_cmd_focus_near_limit(p);
            break;
        case 0x58:
            bridge_cmd_focus_af_sensitivity(b4 == 2 ? 1 : 0);
            break;
        case 0x11:
            bridge_cmd_focus_ir_correction(b4);
            break;
        case 0x3f:
            switch (payload->data[4]) {
                case 0x00:
                    bridge_cmd_preset_reset();
                    break;
                case 0x01:
                    bridge_cmd_preset_set();
                    break;
                case 0x02:
                    bridge_cmd_preset_recall();
                    break;
                default:
                    bad_byte(4);
            }
            break;
        case 0x66:
            bridge_cmd_system_img_flip(onoff);
            break;
        default:
            bad_byte(3);
    }
}

static void dispatch_detail(const buffer_t *payload, uint32_t seq_number, buffer_t *response)
{
    switch (payload->data[4]) {
        case 0x01:
            bridge_cmd_detail_mode();
            break;
        case 0x02:
            bridge_cmd_detail_bandwidth();
            break;
        case 0x03:
            bridge_cmd_detail_crispening();
            break;
        case 0x04:
            bridge_cmd_detail_hv_balance();
            break;
        case 0x05:
            bridge_cmd_detail_bw_balance();
            break;
        case 0x06:
            bridge_cmd_detail_limit();
            break;
        case 0x07:
            bridge_cmd_detail_highlight_detail();
            break;
        case 0x08:
            bridge_cmd_detail_superlow();
            break;
        default:
            bad_byte(4);
    }
}

static void dispatch_05(const buffer_t *payload, uint32_t seq_number, buffer_t *response)
{
    uint64_t p, q;
    uint8_t b4 = payload->data[4], onoff = (b4 == 2) ? 1 : 0;

    switch (payload->data[3]) {
        case 0x0c:
            bridge_cmd_exposure_gain_point_set(onoff);
            break;
        case 0x4c:
            p = parse_retarded_integer_encoding(payload, 4, 2);
            bridge_cmd_exposure_gain_point_position(gain_parameter_to_db(p));
            break;
        case 0x2a:
            p = parse_retarded_integer_encoding(payload, 5, 2);
            bridge_cmd_exposure_minmax_shutter_set(b4, p);
            break;
        case 0x39:
            bridge_cmd_exposure_low_light_basis_brightness_set(onoff);
            break;
        case 0x49:
            bridge_cmd_exposure_low_light_basis_brightness_direct(b4);
            break;
        case 0x42:
            dispatch_detail(payload, seq_number, response);
            break;
        case 0x5b:
            p = parse_retarded_integer_encoding(payload, 4, 3);
            bridge_cmd_gamma_pattern(p);
            break;
        case 0x5c:
            bridge_cmd_gamma_black_gamma_range(b4);
            break;
        case 0x53:
            p = payload->data[4];
            q = payload->data[5];
            bridge_cmd_noise_reduction_2d_3d_manual_setting(p, q);
            break;
        default:
            bad_byte(3);
    }
}

static void ptd_directionals(const buffer_t *payload)
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

    bridge_cmd_pan_tilt_directionals(vert, horiz, pan_speed, tilt_speed);
}

static void ptd_abs_rel(const buffer_t *payload, int rel)
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
        bridge_cmd_pan_tilt_relative_move(speed, p, t);
    } else {
        bridge_cmd_pan_tilt_absolute_move(speed, p, t);
    }
}

static void ptd_pan_tilt_limit(const buffer_t *payload)
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

        bridge_cmd_pan_tilt_limit_set(position, p, t);
    } else {
        bridge_cmd_pan_tilt_limit_clear(position);
    }
}

static void ptd_ramp_curve(const buffer_t *payload)
{
    int p;

    check_length(6);

    p = payload->data[4];

    if (p != 1 && p != 2 && p != 3) {
        log("ptd_ramp_curve: unexpected p %d", p);
        return;
    }

    bridge_cmd_pan_tilt_ramp_curve(p);
}

static void ptd_slow_mode(const buffer_t *payload)
{
    int p;

    check_length(6);

    p = payload->data[4];

    if (p != 2 && p != 3) {
        log("ptd_slow_mode: unexpected p %d", p);
        return;
    }

    bridge_cmd_pan_tilt_slow_mode(p);
}

static void dispatch_pan_tilt_drive(const buffer_t *payload, buffer_t *response, uint32_t seq_number)
{
    switch (payload->data[3]) {
        case 0x01:
            ptd_directionals(payload);
            break;
        case 0x02:
            ptd_abs_rel(payload, 0);
            break;
        case 0x03:
            ptd_abs_rel(payload, 1);
            break;
        case 0x04:
            bridge_cmd_pan_tilt_home();
            break;
        case 0x05:
            bridge_cmd_pan_tilt_reset();
            break;
        case 0x07:
            ptd_pan_tilt_limit(payload);
            break;
        case 0x31:
            ptd_ramp_curve(payload);
            break;
        case 0x44:
            ptd_slow_mode(payload);
            break;
        default:
            log("dispatch_pan_tilt_drive: unexpected type 0x%02x", payload->data[3]);
            return;
    }

    (void)response;
    (void)seq_number;
}

static void dispatch_7e_01(const buffer_t *payload, uint32_t seq_number, buffer_t *response)
{
    uint64_t p, q;
    uint8_t b5 = payload->data[5];
    int w, h, f;
    char l;

    switch (payload->data[4]) {
        case 0x53:
            bridge_cmd_exposure_nd_filter(b5);
            break;
        case 0x2e:
            switch (payload->data[6]) {
                case 0x00:
                    switch (payload->data[7]) {
                        case 0x00:
                            bridge_cmd_color_offset_reset();
                            break;
                        case 0x02:
                            bridge_cmd_color_offset_up();
                            break;
                        case 0x03:
                            bridge_cmd_color_offset_down();
                            break;
                        default:
                            bad_byte(7);
                    }
                case 0x01:
                    bridge_cmd_color_offset_direct((int)payload->data[6] - 7);
                    break;
                default:
                    bad_byte(6);
            }
            break;
        case 0x3d:
            bridge_cmd_color_matrix_select(b5);
            break;
        case 0x7a:
            p = parse_retarded_integer_encoding(payload, 5, 2);
            bridge_cmd_color_rg((int)p - 99);
            break;
        case 0x7b:
            p = parse_retarded_integer_encoding(payload, 5, 2);
            bridge_cmd_color_rb((int)p - 99);
            break;
        case 0x7c:
            p = parse_retarded_integer_encoding(payload, 5, 2);
            bridge_cmd_color_gr((int)p - 99);
            break;
        case 0x7d:
            p = parse_retarded_integer_encoding(payload, 5, 2);
            bridge_cmd_color_gb((int)p - 99);
            break;
        case 0x7e:
            p = parse_retarded_integer_encoding(payload, 5, 2);
            bridge_cmd_color_br((int)p - 99);
            break;
        case 0x7f:
            p = parse_retarded_integer_encoding(payload, 5, 2);
            bridge_cmd_color_bg((int)p - 99);
            break;
        case 0x6d:
            bridge_cmd_knee_set(payload->data[5] == 2 ? 1 : 0);
            break;
        case 0x54:
            bridge_cmd_knee_mode(b5 == 4 ? 1 : 0);
            break;
        case 0x6f:
            p = parse_retarded_integer_encoding(payload, 5, 2);
            bridge_cmd_knee_slope(p);
            break;
        case 0x6e:
            p = parse_retarded_integer_encoding(payload, 5, 2);
            bridge_cmd_knee_point(p);
            break;
        case 0x71:
            p = parse_retarded_integer_encoding(payload, 5, 2);
            bridge_cmd_gamma_level(p);
            break;
        case 0x72:
            p = parse_retarded_integer_encoding(payload, 5, 2);
            bridge_cmd_gamma_black_gamma_level(p);
            break;
        case 0x0b:
            p = payload->data[5];
            q = payload->data[6];
            bridge_cmd_preset_drive_speed(p, q);
            break;
        case 0x3e:
            bridge_cmd_system_hphase_set(payload->data[6] == 0x02 ? 1 : 0);
            break;
        case 0x5b:
            p = parse_retarded_integer_encoding(payload, 6, 3);
            bridge_cmd_system_hphase_direct(p);
            break;
        case 0x06:
            bridge_cmd_system_pan_reverse(payload->data[6]);
            break;
        case 0x09:
            bridge_cmd_system_tilt_reverse(payload->data[6]);
            break;
        case 0x0a:
            switch (payload->data[6]) {
                case 0x00:
                    bridge_cmd_tarry_set(payload->data[6] == 0x02 ? 1 : 0);
                    break;
                case 0x01:
                    p = payload->data[6];
                    switch (p) {
                        case 0:
                            bridge_cmd_tarry_tally_mode(0);
                            break;
                        case 4:
                            bridge_cmd_tarry_tally_mode(1);
                            break;
                        case 5:
                            bridge_cmd_tarry_tally_mode(2);
                            break;
                        default:
                            bad_byte(6);
                    }
                    break;
                default:
                    bad_byte(6);
            }
            break;
        case 0x1e:
            p = parse_retarded_integer_encoding(payload, 5, 2);
            hdmi_parameter_to_specs(p, &w, &h, &f, &l);
            bridge_cmd_hdmi_video_format_change(w, h, f, l);
            break;
        case 0x03:
            bridge_cmd_hdmi_color_space(payload->data[6]);
            break;
        default:
            bad_byte(4);
    }
}

static void dispatch_7e_04(const buffer_t *payload, uint32_t seq_number, buffer_t *response)
{
    uint64_t p;

    switch (payload->data[5]) {
        case 0x15:
            switch (payload->data[6]) {
                case 0x00:
                    bridge_cmd_gamma_black_level_reset();
                    break;
                case 0x02:
                    bridge_cmd_gamma_black_level_up();
                    break;
                case 0x03:
                    bridge_cmd_gamma_black_level_down();
                    break;
                default:
                    bad_byte(6);
            }
            break;
        case 0x45:
            p = parse_retarded_integer_encoding(payload, 5, 2);
            bridge_cmd_gamma_black_level_direct((int)p - 48);
            break;
        case 0x5f:
            bridge_cmd_picture_profile_mode(payload->data[5] + 1);
            break;
        case 0x36:
            bridge_cmd_zoom_teleconvert_mode(payload->data[5] == 2 ? 1 : 0);
            break;
        case 0x3d:
            bridge_cmd_preset_mode(payload->data[5]);
            break;
        case 0x20:
            switch (payload->data[6]) {
                case 0x00:
                    bridge_cmd_ptz_trace_rec(payload->data[7] == 2 ? 1 : 0, payload->data[6] & 0x0f);
                    break;
                case 0x01:
                    bridge_cmd_ptz_trace_play(payload->data[7] - 1, payload->data[6] & 0x0f);
                    break;
                case 0x02:
                    bridge_cmd_ptz_trace_delete(payload->data[6] & 0x0f);
                    break;
                default:
                    bad_byte(6);
            }
            break;
        default:
            bad_byte(5);
    }
}

static void dispatch_7e(const buffer_t *payload, uint32_t seq_number, buffer_t *response)
{
    switch (payload->data[3]) {
        case 0x01:
            dispatch_7e_01(payload, seq_number, response);
            break;
        case 0x04:
            dispatch_7e_04(payload, seq_number, response);
            break;
        default:
            bad_byte(3);
    }
}

void visca_commands_dispatch(const buffer_t *payload, uint32_t seq_number, buffer_t *response)
{
    switch (payload->data[2]) {
        case 0x04:
            dispatch_04(payload, seq_number, response);
            break;
        case 0x05:
            dispatch_05(payload, seq_number, response);
            break;
        case 0x06:
            if (payload->data[3] == 0x06) {
                bridge_cmd_menu_display_off();
            } else {
                dispatch_pan_tilt_drive(payload, response, seq_number);
            }
            break;
        case 0x7e:
            dispatch_7e(payload, seq_number, response);
            break;
        default:
            bad_byte(2);
    }
}

