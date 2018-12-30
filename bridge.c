#include "bridge.h"
#include "log.h"

static void log_p5t4(uint8_t p[5], uint8_t t[4])
{
    log("%02x%02x%02x%02x%02x %02x%02x%02x%02x", p[0], p[1], p[2], p[3], p[4],
            t[0], t[1], t[2], t[3]);
}

void bridge_directionals(int vert, int horiz, uint8_t pan_speed, uint8_t tilt_speed)
{
    log("bridge_directionals: % d, % d, (pan %d, tilt %d)", vert, horiz, pan_speed, tilt_speed);
}

void bridge_relative_move(uint8_t speed, uint8_t p[5], uint8_t t[4])
{
    log("bridge_relative_move: %d,", speed);
    log_p5t4(p, t);
}

void bridge_absolute_move(uint8_t speed, uint8_t p[5], uint8_t t[4])
{
    log("bridge_absolute_move: %d,", speed);
    log_p5t4(p, t);
}

void bridge_home()
{
    log("bridge_home");
}

void bridge_reset()
{
    log("bridge_reset");
}

void bridge_pan_tilt_limit_set(uint8_t position, uint8_t p[5], uint8_t t[4])
{
    log("bridge_pan_tilt_limit_set: %d,", position);
    log_p5t4(p, t);
}

void bridge_pan_tilt_limit_clear(uint8_t position)
{
    log("bridge_pan_tilt_limit_clear: %d", position);
}

void bridge_ramp_curve(int p)
{
    log("bridge_ramp_curve: %d", p);
}

void bridge_slow_mode(int p)
{
    log("bridge_slow_mode: %d", p);
}

void bridge_inq_color_bg()
{
    log("bridge_inq_color_bg STUB");
}

void bridge_inq_color_b_gain()
{
    log("bridge_inq_color_b_gain STUB");
}

void bridge_inq_color_br()
{
    log("bridge_inq_color_br STUB");
}

void bridge_inq_color_chroma_suppress()
{
    log("bridge_inq_color_chroma_suppress STUB");
}

void bridge_inq_color_gb()
{
    log("bridge_inq_color_gb STUB");
}

void bridge_inq_color_gr()
{
    log("bridge_inq_color_gr STUB");
}

void bridge_inq_color_level()
{
    log("bridge_inq_color_level STUB");
}

void bridge_inq_color_matrix()
{
    log("bridge_inq_color_matrix STUB");
}

void bridge_inq_color_offset()
{
    log("bridge_inq_color_offset STUB");
}

void bridge_inq_color_phase()
{
    log("bridge_inq_color_phase STUB");
}

void bridge_inq_color_rb()
{
    log("bridge_inq_color_rb STUB");
}

void bridge_inq_color_rg()
{
    log("bridge_inq_color_rg STUB");
}

void bridge_inq_color_r_gain()
{
    log("bridge_inq_color_r_gain STUB");
}

void bridge_inq_color_speed()
{
    log("bridge_inq_color_speed STUB");
}

void bridge_inq_color_white_balance_mode()
{
    log("bridge_inq_color_white_balance_mode STUB");
}

void bridge_inq_detail_bandwidth()
{
    log("bridge_inq_detail_bandwidth STUB");
}

void bridge_inq_detail_bw_balance()
{
    log("bridge_inq_detail_bw_balance STUB");
}

void bridge_inq_detail_crispening()
{
    log("bridge_inq_detail_crispening STUB");
}

void bridge_inq_detail_highlight_detail()
{
    log("bridge_inq_detail_highlight_detail STUB");
}

void bridge_inq_detail_hv_balance()
{
    log("bridge_inq_detail_hv_balance STUB");
}

void bridge_inq_detail_level()
{
    log("bridge_inq_detail_level STUB");
}

void bridge_inq_detail_limit()
{
    log("bridge_inq_detail_limit STUB");
}

void bridge_inq_detail_mode()
{
    log("bridge_inq_detail_mode STUB");
}

void bridge_inq_detail_superlow()
{
    log("bridge_inq_detail_superlow STUB");
}

void bridge_inq_exposure_ae_speed()
{
    log("bridge_inq_exposure_ae_speed STUB");
}

void bridge_inq_exposure_back_light()
{
    log("bridge_inq_exposure_back_light STUB");
}

void bridge_inq_exposure_ex_comp_level()
{
    log("bridge_inq_exposure_ex_comp_level STUB");
}

void bridge_inq_exposure_ex_comp_on()
{
    log("bridge_inq_exposure_ex_comp_on STUB");
}

void bridge_inq_exposure_gain()
{
    log("bridge_inq_exposure_gain STUB");
}

void bridge_inq_exposure_gain_limit()
{
    log("bridge_inq_exposure_gain_limit STUB");
}

void bridge_inq_exposure_gain_point_position()
{
    log("bridge_inq_exposure_gain_point_position STUB");
}

void bridge_inq_exposure_iris()
{
    log("bridge_inq_exposure_iris STUB");
}

void bridge_inq_exposure_low_light_basis_brightness()
{
    log("bridge_inq_exposure_low_light_basis_brightness STUB");
}

void bridge_inq_exposure_low_light_basis_brightness_on()
{
    log("bridge_inq_exposure_low_light_basis_brightness_on STUB");
}

void bridge_inq_exposure_max_shutter()
{
    log("bridge_inq_exposure_max_shutter STUB");
}

void bridge_inq_exposure_min_shutter()
{
    log("bridge_inq_exposure_min_shutter STUB");
}

void bridge_inq_exposure_mode()
{
    log("bridge_inq_exposure_mode STUB");
}

void bridge_inq_exposure_nd_filter()
{
    log("bridge_inq_exposure_nd_filter STUB");
}

void bridge_inq_exposure_shutter()
{
    log("bridge_inq_exposure_shutter STUB");
}

void bridge_inq_exposure_spot_light()
{
    log("bridge_inq_exposure_spot_light STUB");
}

void bridge_inq_exposure_vis_enh()
{
    log("bridge_inq_exposure_vis_enh STUB");
}

void bridge_inq_exposure_vis_enh_on()
{
    log("bridge_inq_exposure_vis_enh_on STUB");
}

void bridge_inq_flicker_reduction_on()
{
    log("bridge_inq_flicker_reduction_on STUB");
}

void bridge_inq_focus_ir_correction()
{
    log("bridge_inq_focus_ir_correction STUB");
}

void bridge_inq_focus_mode()
{
    log("bridge_inq_focus_mode STUB");
}

void bridge_inq_focus_near_limit()
{
    log("bridge_inq_focus_near_limit STUB");
}

void bridge_inq_focus_position()
{
    log("bridge_inq_focus_position STUB");
}

void bridge_inq_focus_sensitivity()
{
    log("bridge_inq_focus_sensitivity STUB");
}

void bridge_inq_gamma_black_gamma_level()
{
    log("bridge_inq_gamma_black_gamma_level STUB");
}

void bridge_inq_gamma_black_gamma_range()
{
    log("bridge_inq_gamma_black_gamma_range STUB");
}

void bridge_inq_gamma_black_level()
{
    log("bridge_inq_gamma_black_level STUB");
}

void bridge_inq_gamma_level()
{
    log("bridge_inq_gamma_level STUB");
}

void bridge_inq_gamma_mode()
{
    log("bridge_inq_gamma_mode STUB");
}

void bridge_inq_gamma_offset()
{
    log("bridge_inq_gamma_offset STUB");
}

void bridge_inq_gamma_pattern()
{
    log("bridge_inq_gamma_pattern STUB");
}

void bridge_inq_hdmi_color_space()
{
    log("bridge_inq_hdmi_color_space STUB");
}

void bridge_inq_hdmi_video_format()
{
    log("bridge_inq_hdmi_video_format STUB");
}

void bridge_inq_knee_mode()
{
    log("bridge_inq_knee_mode STUB");
}

void bridge_inq_knee_point()
{
    log("bridge_inq_knee_point STUB");
}

void bridge_inq_knee_setting()
{
    log("bridge_inq_knee_setting STUB");
}

void bridge_inq_knee_slope()
{
    log("bridge_inq_knee_slope STUB");
}

void bridge_inq_menu_display_status()
{
    log("bridge_inq_menu_display_status STUB");
}

void bridge_inq_noise_reduction_manual_setting()
{
    log("bridge_inq_noise_reduction_manual_setting STUB");
}

void bridge_inq_noise_reduction_mode_level()
{
    log("bridge_inq_noise_reduction_mode_level STUB");
}

void bridge_inq_pan_tilt_limit()
{
    log("bridge_inq_pan_tilt_limit STUB");
}

void bridge_inq_pan_tilt_position()
{
    log("bridge_inq_pan_tilt_position STUB");
}

void bridge_inq_pan_tilt_ramp_curve()
{
    log("bridge_inq_pan_tilt_ramp_curve STUB");
}

void bridge_inq_pan_tilt_slow_mode()
{
    log("bridge_inq_pan_tilt_slow_mode STUB");
}

void bridge_inq_pan_tilt_status()
{
    log("bridge_inq_pan_tilt_status STUB");
}

void bridge_inq_power_on()
{
    log("bridge_inq_power_on STUB");
}

void bridge_inq_preset()
{
    log("bridge_inq_preset STUB");
}

void bridge_inq_preset_driven_speed()
{
    log("bridge_inq_preset_driven_speed STUB");
}

void bridge_inq_preset_mode()
{
    log("bridge_inq_preset_mode STUB");
}

void bridge_inq_software_version()
{
    log("bridge_inq_software_version STUB");
}

void bridge_inq_system_hphase()
{
    log("bridge_inq_system_hphase STUB");
}

void bridge_inq_system_img_flip()
{
    log("bridge_inq_system_img_flip STUB");
}

void bridge_inq_system_ir_receive()
{
    log("bridge_inq_system_ir_receive STUB");
}

void bridge_inq_system_pan_reverse()
{
    log("bridge_inq_system_pan_reverse STUB");
}

void bridge_inq_system_tilt_reverse()
{
    log("bridge_inq_system_tilt_reverse STUB");
}

void bridge_inq_tally_on()
{
    log("bridge_inq_tally_on STUB");
}

void bridge_inq_zoom_position()
{
    log("bridge_inq_zoom_position STUB");
}

