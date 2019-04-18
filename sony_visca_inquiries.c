#include "bridge_inquiries.h"
#include "log.h"
#include "sony_visca.h"
#include "sony_visca_inquiries.h"
#include "visca.h"

static buffer_t* dispatch_09_04(const struct message_t *message)
{
    switch (message->payload[3]) {
        case 0x39:
            bridge_inq_exposure_mode();
            return cons_buffer(1);
        case 0x4b:
            bridge_inq_exposure_iris();
            return cons_buffer(4);
        case 0x4c:
            bridge_inq_exposure_gain();
            return cons_buffer(4);
        case 0x2c:
            bridge_inq_exposure_gain_limit();
            return cons_buffer(1);
        case 0x4a:
            bridge_inq_exposure_shutter();
            return cons_buffer(4);
        case 0x5d:
            bridge_inq_exposure_ae_speed();
            return cons_buffer(1);
        case 0x3e:
            bridge_inq_exposure_ex_comp_on();
            return cons_buffer(1);
        case 0x4e:
            bridge_inq_exposure_ex_comp_level();
            return cons_buffer(4);
        case 0x33:
            bridge_inq_exposure_back_light();
            return cons_buffer(1);
        case 0x3a:
            bridge_inq_exposure_spot_light();
            return cons_buffer(1);
        case 0x3d:
            bridge_inq_exposure_vis_enh_on();
            return cons_buffer(1);
        case 0x2d:
            bridge_inq_exposure_vis_enh();
            return cons_buffer(8);
        case 0x35:
            bridge_inq_color_white_balance_mode();
            return cons_buffer(1);
        case 0x43:
            bridge_inq_color_r_gain();
            return cons_buffer(4);
        case 0x44:
            bridge_inq_color_b_gain();
            return cons_buffer(4);
        case 0x56:
            bridge_inq_color_speed();
            return cons_buffer(1);
        case 0x5f:
            bridge_inq_color_chroma_suppress();
            return cons_buffer(1);
        case 0x49:
            bridge_inq_color_level();
            return cons_buffer(4);
        case 0x4f:
            bridge_inq_color_phase();
            return cons_buffer(4);
        case 0x42:
            bridge_inq_detail_level();
            return cons_buffer(1);
        case 0x5b:
            bridge_inq_gamma_mode();
            return cons_buffer(1);
        case 0x1e:
            bridge_inq_gamma_offset();
            return cons_buffer(6);
        case 0x32:
            bridge_inq_flicker_reduction_on();
            return cons_buffer(1);
        case 0x53:
            bridge_inq_noise_reduction_mode_level();
            return cons_buffer(1);
        case 0x47:
            bridge_inq_zoom_position();
            return cons_buffer(4);
        case 0x38:
            bridge_inq_focus_mode();
            return cons_buffer(1);
        case 0x48:
            bridge_inq_focus_position();
            return cons_buffer(4);
        case 0x58:
            bridge_inq_focus_sensitivity();
            return cons_buffer(1);
        case 0x28:
            bridge_inq_focus_near_limit();
            return cons_buffer(4);
        case 0x11:
            bridge_inq_focus_ir_correction();
            return cons_buffer(1);
        case 0x3f:
            bridge_inq_preset();
            return cons_buffer(1);
        case 0x01:
            bridge_inq_ir_cut_filter();
            return cons_buffer(1);
        case 0x66:
            bridge_inq_system_img_flip();
            return cons_buffer(1);
        case 0x00:
            bridge_inq_power_on();
            return cons_buffer(1);
        default:
            bad_byte_null(3);
    }

    return NULL;
}

static buffer_t* dispatch_09_05_42(const struct message_t *message)
{
    switch (message->payload[4]) {
        case 0x01:
            bridge_inq_detail_mode();
            return cons_buffer(1);
        case 0x02:
            bridge_inq_detail_bandwidth();
            return cons_buffer(1);
        case 0x03:
            bridge_inq_detail_crispening();
            return cons_buffer(1);
        case 0x04:
            bridge_inq_detail_hv_balance();
            return cons_buffer(1);
        case 0x05:
            bridge_inq_detail_bw_balance();
            return cons_buffer(1);
        case 0x06:
            bridge_inq_detail_limit();
            return cons_buffer(1);
        case 0x07:
            bridge_inq_detail_highlight_detail();
            return cons_buffer(1);
        case 0x08:
            bridge_inq_detail_superlow();
            return cons_buffer(1);
        default:
            bad_byte_null(4);
    }
}

static buffer_t* dispatch_09_05(const struct message_t *message)
{
    switch (message->payload[3]) {
        case 0x4c:
            bridge_inq_exposure_gain_point_position();
            return cons_buffer(1);
        case 0x2a:
            if (message->payload[4] == 0) {
                bridge_inq_exposure_max_shutter();
                return cons_buffer(1);
            } else {
                bridge_inq_exposure_min_shutter();
                return cons_buffer(1);
            }
        case 0x39:
            bridge_inq_exposure_low_light_basis_brightness_on();
            return cons_buffer(1);
        case 0x49:
            bridge_inq_exposure_low_light_basis_brightness();
            return cons_buffer(1);
        case 0x42:
            return dispatch_09_05_42(message);
        case 0x5b:
            bridge_inq_gamma_pattern();
            return cons_buffer(3);
        case 0x5c:
            bridge_inq_gamma_black_gamma_range();
            return cons_buffer(1);
        case 0x53:
            bridge_inq_noise_reduction_manual_setting();
            return cons_buffer(2);
        default:
            bad_byte_null(3);
    }

    return NULL;
}

static buffer_t* dispatch_09_06(const struct message_t *message)
{
    switch (message->payload[3]) {
        case 0x12:
            return bridge_inq_pan_tilt_position();
        case 0x10:
            bridge_inq_pan_tilt_status();
            return cons_buffer(2);
        case 0x31:
            bridge_inq_pan_tilt_ramp_curve();
            return cons_buffer(1);
        case 0x44:
            bridge_inq_pan_tilt_slow_mode();
            return cons_buffer(1);
        case 0x08:
            bridge_inq_system_ir_receive();
            return cons_buffer(1);
        case 0x23:
            bridge_inq_hdmi_video_format();
            return cons_buffer(1);
        case 0x06:
            bridge_inq_menu_display_status();
            return cons_buffer(1);
        default:
            bad_byte_null(3);
    }

    return NULL;
}

static buffer_t* dispatch_09_7e_01(const struct message_t *message)
{
    switch (message->payload[4]) {
        case 0x2e:
            bridge_inq_color_offset();
            return cons_buffer(1);
        case 0x3d:
            bridge_inq_color_matrix();
            return cons_buffer(1);
        case 0x7a:
            bridge_inq_color_rg();
            return cons_buffer(4);
        case 0x7b:
            bridge_inq_color_rb();
            return cons_buffer(4);
        case 0x7c:
            bridge_inq_color_gr();
            return cons_buffer(4);
        case 0x7d:
            bridge_inq_color_gb();
            return cons_buffer(4);
        case 0x7e:
            bridge_inq_color_br();
            return cons_buffer(4);
        case 0x7f:
            bridge_inq_color_bg();
            return cons_buffer(4);
        case 0x6d:
            bridge_inq_knee_setting();
            return cons_buffer(1);
        case 0x54:
            bridge_inq_knee_mode();
            return cons_buffer(1);
        case 0x6f:
            bridge_inq_knee_slope();
            return cons_buffer(4);
        case 0x6e:
            bridge_inq_knee_point();
            return cons_buffer(4);
        case 0x71:
            bridge_inq_gamma_level();
            return cons_buffer(4);
        case 0x72:
            bridge_inq_gamma_black_gamma_level();
            return cons_buffer(4);
        case 0x0b:
            bridge_inq_preset_driven_speed();
            return cons_buffer(1);
        case 0x0a:
            bridge_inq_tally_on();
            return cons_buffer(1);
        case 0x3e:
            bridge_inq_system_hphase();
            return cons_buffer(4);
        case 0x06:
            bridge_inq_system_pan_reverse();
            return cons_buffer(1);
        case 0x09:
            bridge_inq_system_tilt_reverse();
            return cons_buffer(1);
        case 0x03:
            bridge_inq_hdmi_color_space();
            return cons_buffer(1);
        default:
            bad_byte_null(4);
    }
}

static buffer_t* dispatch_09_7e_04_20(const struct message_t *message)
{
    switch (message->payload[5]) {
        case 0x03:
            bridge_inq_ptz_trace_status();
            return cons_buffer(1);
        case 0x10:
            if (message->payload[6] == 0x00) {
                bridge_inq_ptz_trace_record_status_bulk();
                return cons_buffer(4);
            } else if (message->payload[6] == 0x01) {
                bridge_inq_ptz_trace_record_status_individual();
                return cons_buffer(1);
            } else {
                bad_byte_null(6);
            }
        case 0x01:
            bridge_inq_ptz_trace_playback_prep_status();
            return cons_buffer(1);
        default:
            bad_byte_null(5);
    }
}

static buffer_t* dispatch_09_7e_04(const struct message_t *message)
{
    switch (message->payload[4]) {
        case 0x45:
            bridge_inq_gamma_black_level();
            return cons_buffer(2);
        case 0x3d:
            bridge_inq_preset_mode();
            return cons_buffer(1);
        case 0x20:
            return dispatch_09_7e_04_20(message);
        default:
            bad_byte_null(4);
    }
}

static buffer_t* dispatch_09_7e(const struct message_t *message)
{
    switch (message->payload[3]) {
        case 0x01:
            return dispatch_09_7e_01(message);
        case 0x04:
            return dispatch_09_7e_04(message);
        default:
            bad_byte_null(3);
    }

    return NULL;
}

static buffer_t* dispatch_09_00(const struct message_t *message)
{
    buffer_t* response;

    if (message->payload[3] != 0x02) {
        bad_byte_null(3);
    }

    response = cons_buffer(7);

    bridge_inq_software_version();

    return response;
}

static buffer_t* dispatch_09(const struct message_t *message)
{
    switch (message->payload[2]) {
        case 0x00:
            return dispatch_09_00(message);
        case 0x04:
            return dispatch_09_04(message);
        case 0x05:
            return dispatch_09_05(message);
        case 0x06:
            return dispatch_09_06(message);
        case 0x7e:
            return dispatch_09_7e(message);
        default:
            bad_byte_null(2);
    }
}

static buffer_t* dispatch_01(const struct message_t *message)
{
    switch (message->payload[2]) {
        case 0x7e:
            if (message->payload[3] == 0x01) {
                bridge_inq_exposure_nd_filter();
                return cons_buffer(1);
            } else if (message->payload[3] == 0x04) {
                bridge_inq_preset_mode();
                return cons_buffer(1);
            } else {
                bad_byte_null(3);
            }
        case 0x06:
            bridge_inq_pan_tilt_limit();
            return cons_buffer(9);
    }

    return NULL;
}

buffer_t* sony_visca_inquiries_dispatch(const struct message_t *message)
{
    switch (message->payload[1]) {
        case 0x01:
            return dispatch_01(message);
        case 0x09:
            return dispatch_09(message);
        default:
            bad_byte_null(1);
    }
}

