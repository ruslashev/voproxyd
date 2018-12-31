#include "bridge.h"
#include "log.h"
#include "visca.h"
#include "visca_inquiries.h"

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
            bridge_cmd_exposure_iris_open();
            break;
        case 0x03:
            bridge_cmd_exposure_iris_close();
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

static void dispatch_04(const buffer_t *payload, uint32_t seq_number, buffer_t *response)
{
    switch (payload->data[3]) {
        case 0x39:
            dispatch_exposure_mode(payload, seq_number, response);
            break;
        case 0x0b:
            dispatch_exposure_iris(payload, seq_number, response);
            break;
        case 0x4b:
            bridge_cmd_exposure_iris_direct();
            break;
        case 0x0c:
            dispatch_exposure_gain(payload, seq_number, response);
            break;
        case 0x4c:
            bridge_cmd_exposure_gain_direct();
            break;
        case 0x2c:
            bridge_cmd_exposure_gain_limit();
            break;
        case 0x0a:
            dispatch_exposure_shutter(payload, seq_number, response);
            break;
        case 0x4a:
            bridge_cmd_exposure_shutter_direct();
            break;
        case 0x5d:
            bridge_cmd_exposure_ae_speed();
            break;
        case 0x3e:
            bridge_cmd_exposure_exp_comp_set();
            break;
        case 0x0e:
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
            break;
        case 0x4e:
            bridge_cmd_exposure_exp_comp_direct();
            break;
        case 0x33:
            bridge_cmd_exposure_back_light_set();
            break;
        case 0x3a:
            bridge_cmd_exposure_spot_light_set();
            break;
        case 0x3d:
            switch (payload->data[4]) {
                case 0x03:
                    bridge_cmd_exposure_vis_enh_set();
                    break;
                case 0x06:
                    bridge_cmd_exposure_vis_enh_set();
                    break;
                default:
                    bad_byte(4);
            }
            break;
        case 0x2d:
            bridge_cmd_exposure_vis_enh_direct();
            break;
        case 0x01:
            bridge_cmd_exposure_ir_cut_filter_set();
            break;
        case 0x35:
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
            bridge_cmd_color_rgain_direct();
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
            bridge_cmd_color_bgain_direct();
            break;
        case 0x56:
            bridge_cmd_color_speed();
            break;
        case 0x5f:
            bridge_cmd_color_chroma_suppress();
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
            bridge_cmd_color_level_direct();
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
            bridge_cmd_color_phase_direct();
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
            bridge_cmd_detail_level_direct();
            break;
        case 0x5b:
            bridge_cmd_gamma_mode();
            break;
        case 0x1e:
            bridge_cmd_gamma_offset();
            break;
        case 0x32:
            bridge_cmd_flicker_reduction_mode_set();
            break;
        case 0x53:
            bridge_cmd_noise_reduction_mode_level_set();
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
                    bad_byte(4);
            }
            break;
        case 0x47:
            bridge_cmd_zoom_direct();
            break;
        case 0x06:
            switch (payload->data[4]) {
                case 0x03:
                    bridge_cmd_zoom_clear_image_set();
                    break;
                case 0x04:
                    bridge_cmd_zoom_clear_image_set();
                    break;
                default:
                    bad_byte(4);
            }
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
                    bad_byte(4);
            }
            break;
        case 0x48:
            bridge_cmd_focus_direct();
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
            bridge_cmd_focus_near_limit();
            break;
        case 0x58:
            bridge_cmd_focus_af_sensitivity();
            break;
        case 0x11:
            bridge_cmd_focus_ir_correction();
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
    switch (payload->data[3]) {
        case 0x0c:
            bridge_cmd_exposure_gain_point_set();
            break;
        case 0x4c:
            bridge_cmd_exposure_gain_point_position();
            break;
        case 0x2a:
            bridge_cmd_exposure_minmax_shutter_set();
            break;
        case 0x39:
            bridge_cmd_exposure_low_light_basis_brightness_set();
            break;
        case 0x49:
            bridge_cmd_exposure_low_light_basis_brightness_direct();
            break;
        case 0x42:
            dispatch_detail(payload, seq_number, response);
            break;
        case 0x5b:
            bridge_cmd_gamma_pattern();
            break;
        case 0x5c:
            bridge_cmd_gamma_black_gamma_range();
            break;
        case 0x53:
            bridge_cmd_noise_reduction_2d_3d_manual_setting();
            break;
        default:
            bad_byte(3);
    }
}

static void dispatch_7e_01(const buffer_t *payload, uint32_t seq_number, buffer_t *response)
{
    switch (payload->data[5]) {
        case 0x53:
            bridge_cmd_exposure_nd_filter();
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
                    bridge_cmd_color_offset_direct();
                    break;
                default:
                    bad_byte(6);
            }
            break;
        case 0x3d:
            bridge_cmd_color_matrix_select();
            break;
        case 0x7a:
            bridge_cmd_color_rg();
            break;
        case 0x7b:
            bridge_cmd_color_rb();
            break;
        case 0x7c:
            bridge_cmd_color_gr();
            break;
        case 0x7d:
            bridge_cmd_color_gb();
            break;
        case 0x7e:
            bridge_cmd_color_br();
            break;
        case 0x7f:
            bridge_cmd_color_bg();
            break;
        case 0x6d:
            bridge_cmd_knee_set();
            break;
        case 0x54:
            bridge_cmd_knee_mode();
            break;
        case 0x6f:
            bridge_cmd_knee_slope();
            break;
        case 0x6e:
            bridge_cmd_knee_point();
            break;
        case 0x71:
            bridge_cmd_gamma_level();
            break;
        case 0x72:
            bridge_cmd_gamma_black_gamma_level();
            break;
        case 0x0b:
            bridge_cmd_preset_drive_speed();
            break;
        case 0x3e:
            bridge_cmd_system_hphase_set();
            break;
        case 0x5b:
            bridge_cmd_system_hphase_direct();
            break;
        case 0x06:
            bridge_cmd_system_pan_reverse();
            break;
        case 0x09:
            bridge_cmd_system_tilt_reverse();
            break;
        case 0x0a:
            switch (payload->data[6]) {
                case 0x00:
                    bridge_cmd_tarry_set();
                    break;
                case 0x01:
                    bridge_cmd_tarry_tally_mode();
                    break;
                default:
                    bad_byte(6);
            }
            break;
        case 0x1e:
            bridge_cmd_hdmi_video_format_change();
            break;
        case 0x03:
            bridge_cmd_hdmi_color_space();
            break;
        default:
            bad_byte(5);
    }
}

static void dispatch_7e_04(const buffer_t *payload, uint32_t seq_number, buffer_t *response)
{
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
            bridge_cmd_gamma_black_level_direct();
            break;
        case 0x5f:
            bridge_cmd_picture_profile_mode();
            break;
        case 0x36:
            bridge_cmd_zoom_teleconvert_mode();
            break;
        case 0x3d:
            bridge_cmd_preset_mode();
            break;
        case 0x20:
            switch (payload->data[6]) {
                case 0x00:
                    bridge_cmd_ptz_trace_rec();
                    break;
                case 0x01:
                    bridge_cmd_ptz_trace_play();
                    break;
                case 0x02:
                    bridge_cmd_ptz_trace_delete();
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
        case 0x7e:
            dispatch_7e(payload, seq_number, response);
            break;
        default:
            bad_byte(2);
    }
}

