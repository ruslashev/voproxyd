#include "bridge.h"
#include "log.h"
#include "visca_inquiries.h"

static void dispatch_04(const buffer_t *payload, uint32_t seq_number, buffer_t *response)
{
    switch (payload->data[3]) {
        case 0x39:
            bridge_inq_exposure_mode();
            break;
        case 0x4b:
            bridge_inq_exposure_iris();
            break;
        case 0x4c:
            bridge_inq_exposure_gain();
            break;
        case 0x2c:
            bridge_inq_exposure_gain_limit();
            break;
        case 0x4a:
            bridge_inq_exposure_shutter();
            break;
        case 0x5d:
            bridge_inq_exposure_ae_speed();
            break;
        case 0x3e:
            bridge_inq_exposure_ex_comp_on();
            break;
        case 0x4e:
            bridge_inq_exposure_ex_comp_level();
            break;
        case 0x33:
            bridge_inq_exposure_back_light();
            break;
        case 0x3a:
            bridge_inq_exposure_spot_light();
            break;
        case 0x3d:
            bridge_inq_exposure_vis_enh_on();
            break;
        case 0x2d:
            bridge_inq_exposure_vis_enh();
            break;
        case 0x35:
            bridge_inq_color_white_balance_mode();
            break;
        case 0x43:
            bridge_inq_color_r_gain();
            break;
        case 0x44:
            bridge_inq_color_b_gain();
            break;
        case 0x56:
            bridge_inq_color_speed();
            break;
        case 0x5f:
            bridge_inq_color_chroma_suppress();
            break;
        case 0x49:
            bridge_inq_color_level();
            break;
        case 0x4f:
            bridge_inq_color_phase();
            break;
        case 0x42:
            bridge_inq_detail_level();
            break;
        case 0x5b:
            bridge_inq_gamma_mode();
            break;
        case 0x1e:
            bridge_inq_gamma_offset();
            break;
        case 0x32:
            bridge_inq_flicker_reduction_on();
            break;
        case 0x53:
            bridge_inq_noise_reduction_mode_level();
            break;
        case 0x47:
            bridge_inq_zoom_position();
            break;
        case 0x38:
            bridge_inq_focus_mode();
            break;
        case 0x48:
            bridge_inq_focus_position();
            break;
        case 0x58:
            bridge_inq_focus_sensitivity();
            break;
        case 0x28:
            bridge_inq_focus_near_limit();
            break;
        case 0x11:
            bridge_inq_focus_ir_correction();
            break;
        case 0x3f:
            bridge_inq_preset();
            break;
        case 0x66:
            bridge_inq_system_img_flip();
            break;
        case 0x00:
            bridge_inq_power_on();
            break;
        default:
            log("dispatch_04: unexpected byte 0x%02x", payload->data[3]);
    }
}

static void dispatch_05(const buffer_t *payload, uint32_t seq_number, buffer_t *response)
{
    switch (payload->data[3]) {
        case 0x4c:
            bridge_inq_exposure_gain_point_position();
            break;
        case 0x2a:
            if (payload->data[4] == 0) {
                bridge_inq_exposure_max_shutter();
            } else {
                bridge_inq_exposure_min_shutter();
            }
            break;
        case 0x39:
            bridge_inq_exposure_low_light_basis_brightness_on();
            break;
        case 0x49:
            bridge_inq_exposure_low_light_basis_brightness();
            break;
        case 0x42:
            switch (payload->data[4]) {
                case 0x01:
                    bridge_inq_detail_mode();
                    break;
                case 0x02:
                    bridge_inq_detail_bandwidth();
                    break;
                case 0x03:
                    bridge_inq_detail_crispening();
                    break;
                case 0x04:
                    bridge_inq_detail_hv_balance();
                    break;
                case 0x05:
                    bridge_inq_detail_bw_balance();
                    break;
                case 0x06:
                    bridge_inq_detail_limit();
                    break;
                case 0x07:
                    bridge_inq_detail_highlight_detail();
                    break;
                case 0x08:
                    bridge_inq_detail_superlow();
                    break;
            }
            break;
        case 0x5b:
            bridge_inq_gamma_pattern();
            break;
        case 0x5c:
            bridge_inq_gamma_black_gamma_range();
            break;
        case 0x53:
            bridge_inq_noise_reduction_manual_setting();
            break;
        default:
            log("dispatch_05: unexpected byte 0x%02x", payload->data[3]);
    }
}

static void dispatch_06(const buffer_t *payload, uint32_t seq_number, buffer_t *response)
{
    switch (payload->data[3]) {
        case 0x12:
            bridge_inq_pan_tilt_position();
            break;
        case 0x10:
            bridge_inq_pan_tilt_status();
            break;
        case 0x31:
            bridge_inq_pan_tilt_ramp_curve();
            break;
        case 0x44:
            bridge_inq_pan_tilt_slow_mode();
            break;
        case 0x08:
            bridge_inq_system_ir_receive();
            break;
        case 0x23:
            bridge_inq_hdmi_video_format();
            break;
        case 0x06:
            bridge_inq_menu_display_status();
            break;
        default:
            log("dispatch_06: unexpected byte 0x%02x", payload->data[3]);
    }
}

static void dispatch_7e(const buffer_t *payload, uint32_t seq_number, buffer_t *response)
{
    switch (payload->data[3]) {
        case 0x01:
            switch (payload->data[4]) {
                case 0x2e:
                    bridge_inq_color_offset();
                    break;
                case 0x3d:
                    bridge_inq_color_matrix();
                    break;
                case 0x7a:
                    bridge_inq_color_rg();
                    break;
                case 0x7b:
                    bridge_inq_color_rb();
                    break;
                case 0x7c:
                    bridge_inq_color_gr();
                    break;
                case 0x7d:
                    bridge_inq_color_gb();
                    break;
                case 0x7e:
                    bridge_inq_color_br();
                    break;
                case 0x7f:
                    bridge_inq_color_bg();
                    break;
                case 0x6d:
                    bridge_inq_knee_setting();
                    break;
                case 0x54:
                    bridge_inq_knee_mode();
                    break;
                case 0x6f:
                    bridge_inq_knee_slope();
                    break;
                case 0x6e:
                    bridge_inq_knee_point();
                    break;
                case 0x71:
                    bridge_inq_gamma_level();
                    break;
                case 0x72:
                    bridge_inq_gamma_black_gamma_level();
                    break;
                case 0x0b:
                    bridge_inq_preset_driven_speed();
                    break;
                case 0x0a:
                    bridge_inq_tally_on();
                    break;
                case 0x3e:
                    bridge_inq_system_hphase();
                    break;
                case 0x06:
                    bridge_inq_system_pan_reverse();
                    break;
                case 0x09:
                    bridge_inq_system_tilt_reverse();
                    break;
                case 0x03:
                    bridge_inq_hdmi_color_space();
                    break;
                default:
                    log("dispatch_7e: 0x01: unexpected byte 0x%02x", payload->data[4]);
                    return;
            }
            break;
        case 0x04:
            switch (payload->data[4]) {
                case 0x45:
                    bridge_inq_gamma_black_level();
                    break;
                case 0x3d:
                    bridge_inq_preset_mode();
                    break;
                default:
                    log("dispatch_7e: 0x04: unexpected byte 0x%02x", payload->data[4]);
                    return;
            }
        default:
            log("dispatch_7e: unexpected byte 0x%02x", payload->data[3]);
            return;
    }
}

void visca_inquiries_dispatch(const buffer_t *payload, uint32_t seq_number, buffer_t *response)
{
    switch (payload->data[2]) {
        case 0x00:
            bridge_inq_software_version();
            break;
        case 0x04:
            dispatch_04(payload, seq_number, response);
            break;
        case 0x05:
            dispatch_05(payload, seq_number, response);
            break;
        case 0x06:
            dispatch_06(payload, seq_number, response);
            break;
        case 0x7e:
            dispatch_7e(payload, seq_number, response);
            break;
        default:
            log("visca_inquiries_dispatch: invalid byte 0x%02x", payload->data[2]);
            return;
    }
}

