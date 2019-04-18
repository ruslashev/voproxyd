#include "bridge_commands.h"
#include "log.h"
#include "soap_ptz.h"
#include "soap_utils.h"
#include "soap_instance.h"

static void log_p5t4(uint8_t p[5], uint8_t t[4])
{
    log("%02x%02x%02x%02x%02x %02x%02x%02x%02x", p[0], p[1], p[2], p[3], p[4],
            t[0], t[1], t[2], t[3]);
}

void bridge_cmd_color_bgain_direct(int p)
{
    log("bridge_cmd_color_bgain_direct STUB");
}

void bridge_cmd_color_bgain_down()
{
    log("bridge_cmd_color_bgain_down STUB");
}

void bridge_cmd_color_bgain_reset()
{
    log("bridge_cmd_color_bgain_reset STUB");
}

void bridge_cmd_color_bgain_up()
{
    log("bridge_cmd_color_bgain_up STUB");
}

void bridge_cmd_color_bg(int p)
{
    log("bridge_cmd_color_bg STUB");
}

void bridge_cmd_color_br(int p)
{
    log("bridge_cmd_color_br STUB");
}

void bridge_cmd_color_chroma_suppress(uint8_t strength)
{
    log("bridge_cmd_color_chroma_suppress STUB");
}

void bridge_cmd_color_gb(int p)
{
    log("bridge_cmd_color_gb STUB");
}

void bridge_cmd_color_gr(int p)
{
    log("bridge_cmd_color_gr STUB");
}

void bridge_cmd_color_level_direct(uint8_t p)
{
    log("bridge_cmd_color_level_direct STUB");
}

void bridge_cmd_color_level_down()
{
    log("bridge_cmd_color_level_down STUB");
}

void bridge_cmd_color_level_reset()
{
    log("bridge_cmd_color_level_reset STUB");
}

void bridge_cmd_color_level_up()
{
    log("bridge_cmd_color_level_up STUB");
}

void bridge_cmd_color_matrix_select(uint8_t p)
{
    log("bridge_cmd_color_matrix_select STUB");
}

void bridge_cmd_color_offset_direct(int p)
{
    log("bridge_cmd_color_offset_direct STUB");
}

void bridge_cmd_color_offset_down()
{
    log("bridge_cmd_color_offset_down STUB");
}

void bridge_cmd_color_offset_reset()
{
    log("bridge_cmd_color_offset_reset STUB");
}

void bridge_cmd_color_offset_up()
{
    log("bridge_cmd_color_offset_up STUB");
}

void bridge_cmd_color_one_push_trigger()
{
    log("bridge_cmd_color_one_push_trigger STUB");
}

void bridge_cmd_color_phase_direct(uint8_t p)
{
    log("bridge_cmd_color_phase_direct STUB");
}

void bridge_cmd_color_phase_down()
{
    log("bridge_cmd_color_phase_down STUB");
}

void bridge_cmd_color_phase_reset()
{
    log("bridge_cmd_color_phase_reset STUB");
}

void bridge_cmd_color_phase_up()
{
    log("bridge_cmd_color_phase_up STUB");
}

void bridge_cmd_color_rb(int p)
{
    log("bridge_cmd_color_rb STUB");
}

void bridge_cmd_color_rgain_direct(int p)
{
    log("bridge_cmd_color_rgain_direct STUB");
}

void bridge_cmd_color_rgain_down()
{
    log("bridge_cmd_color_rgain_down STUB");
}

void bridge_cmd_color_rgain_reset()
{
    log("bridge_cmd_color_rgain_reset STUB");
}

void bridge_cmd_color_rgain_up()
{
    log("bridge_cmd_color_rgain_up STUB");
}

void bridge_cmd_color_rg(int p)
{
    log("bridge_cmd_color_rg STUB");
}

void bridge_cmd_color_speed(uint8_t speed)
{
    log("bridge_cmd_color_speed STUB");
}

void bridge_cmd_color_white_balance_auto1()
{
    log("bridge_cmd_color_white_balance_auto1 STUB");
}

void bridge_cmd_color_white_balance_auto2()
{
    log("bridge_cmd_color_white_balance_auto2 STUB");
}

void bridge_cmd_color_white_balance_indoor()
{
    log("bridge_cmd_color_white_balance_indoor STUB");
}

void bridge_cmd_color_white_balance_manual()
{
    log("bridge_cmd_color_white_balance_manual STUB");
}

void bridge_cmd_color_white_balance_one_push_wb()
{
    log("bridge_cmd_color_white_balance_one_push_wb STUB");
}

void bridge_cmd_color_white_balance_outdoor()
{
    log("bridge_cmd_color_white_balance_outdoor STUB");
}

void bridge_cmd_detail_bandwidth()
{
    log("bridge_cmd_detail_bandwidth STUB");
}

void bridge_cmd_detail_bw_balance()
{
    log("bridge_cmd_detail_bw_balance STUB");
}

void bridge_cmd_detail_crispening()
{
    log("bridge_cmd_detail_crispening STUB");
}

void bridge_cmd_detail_highlight_detail()
{
    log("bridge_cmd_detail_highlight_detail STUB");
}

void bridge_cmd_detail_hv_balance()
{
    log("bridge_cmd_detail_hv_balance STUB");
}

void bridge_cmd_detail_level_direct(uint8_t aperture_gain)
{
    log("bridge_cmd_detail_level_direct STUB");
}

void bridge_cmd_detail_level_down()
{
    log("bridge_cmd_detail_level_down STUB");
}

void bridge_cmd_detail_level_reset()
{
    log("bridge_cmd_detail_level_reset STUB");
}

void bridge_cmd_detail_level_up()
{
    log("bridge_cmd_detail_level_up STUB");
}

void bridge_cmd_detail_limit()
{
    log("bridge_cmd_detail_limit STUB");
}

void bridge_cmd_detail_mode()
{
    log("bridge_cmd_detail_mode STUB");
}

void bridge_cmd_detail_superlow()
{
    log("bridge_cmd_detail_superlow STUB");
}

void bridge_cmd_exposure_ae_speed(uint8_t p)
{
    log("bridge_cmd_exposure_ae_speed STUB");
}

void bridge_cmd_exposure_back_light_set(uint8_t on)
{
    log("bridge_cmd_exposure_back_light_set STUB");
}

void bridge_cmd_exposure_exp_comp_direct(uint8_t p)
{
    log("bridge_cmd_exposure_exp_comp_direct STUB");
}

void bridge_cmd_exposure_exp_comp_down()
{
    log("bridge_cmd_exposure_exp_comp_down STUB");
}

void bridge_cmd_exposure_exp_comp_reset()
{
    log("bridge_cmd_exposure_exp_comp_reset STUB");
}

void bridge_cmd_exposure_exp_comp_set(uint8_t onoff)
{
    log("bridge_cmd_exposure_exp_comp_set STUB");
}

void bridge_cmd_exposure_exp_comp_up()
{
    log("bridge_cmd_exposure_exp_comp_up STUB");
}

void bridge_cmd_exposure_gain_direct(int db)
{
    log("bridge_cmd_exposure_gain_direct STUB");
}

void bridge_cmd_exposure_gain_down()
{
    log("bridge_cmd_exposure_gain_down STUB");
}

void bridge_cmd_exposure_gain_limit(int db)
{
    log("bridge_cmd_exposure_gain_limit STUB");
}

void bridge_cmd_exposure_gain_point_position(int db)
{
    log("bridge_cmd_exposure_gain_point_position STUB");
}

void bridge_cmd_exposure_gain_point_set(uint8_t on)
{
    log("bridge_cmd_exposure_gain_point_set STUB");
}

void bridge_cmd_exposure_gain_reset()
{
    log("bridge_cmd_exposure_gain_reset STUB");
}

void bridge_cmd_exposure_gain_up()
{
    log("bridge_cmd_exposure_gain_up STUB");
}

void bridge_cmd_exposure_ir_cut_filter_set(uint8_t on)
{
    log("bridge_cmd_exposure_ir_cut_filter_set STUB");
}

void bridge_cmd_exposure_iris_direct(uint8_t position)
{
    log("bridge_cmd_exposure_iris_direct STUB");
}

void bridge_cmd_exposure_iris_down()
{
    log("bridge_cmd_exposure_iris_down STUB");
}

void bridge_cmd_exposure_iris_reset()
{
    log("bridge_cmd_exposure_iris_reset STUB");
}

void bridge_cmd_exposure_iris_up()
{
    log("bridge_cmd_exposure_iris_up STUB");
}

void bridge_cmd_exposure_low_light_basis_brightness_direct(uint8_t p)
{
    log("bridge_cmd_exposure_low_light_basis_brightness_direct STUB");
}

void bridge_cmd_exposure_low_light_basis_brightness_set(uint8_t on)
{
    log("bridge_cmd_exposure_low_light_basis_brightness_set STUB");
}

void bridge_cmd_exposure_minmax_shutter_set(uint8_t min, uint8_t p)
{
    log("bridge_cmd_exposure_minmax_shutter_set STUB");
}

void bridge_cmd_exposure_mode_full_auto()
{
    log("bridge_cmd_exposure_mode_full_auto STUB");
}

void bridge_cmd_exposure_mode_gain_pri()
{
    log("bridge_cmd_exposure_mode_gain_pri STUB");
}

void bridge_cmd_exposure_mode_iris_pri()
{
    log("bridge_cmd_exposure_mode_iris_pri STUB");
}

void bridge_cmd_exposure_mode_manual()
{
    log("bridge_cmd_exposure_mode_manual STUB");
}

void bridge_cmd_exposure_mode_shutter_pri()
{
    log("bridge_cmd_exposure_mode_shutter_pri STUB");
}

void bridge_cmd_exposure_nd_filter(uint8_t p)
{
    log("bridge_cmd_exposure_nd_filter STUB");
}

void bridge_cmd_exposure_shutter_direct(uint8_t position)
{
    log("bridge_cmd_exposure_shutter_direct STUB");
}

void bridge_cmd_exposure_shutter_fast()
{
    log("bridge_cmd_exposure_shutter_fast STUB");
}

void bridge_cmd_exposure_shutter_reset()
{
    log("bridge_cmd_exposure_shutter_reset STUB");
}

void bridge_cmd_exposure_shutter_slow()
{
    log("bridge_cmd_exposure_shutter_slow STUB");
}

void bridge_cmd_exposure_spot_light_set(uint8_t on)
{
    log("bridge_cmd_exposure_spot_light_set STUB");
}

void bridge_cmd_exposure_vis_enh_direct(uint8_t brightn, uint8_t brightn_comp, uint8_t comp_lvl)
{
    log("bridge_cmd_exposure_vis_enh_direct STUB");
}

void bridge_cmd_exposure_vis_enh_set(uint8_t on)
{
    log("bridge_cmd_exposure_vis_enh_set STUB");
}

void bridge_cmd_flicker_reduction_mode_set(uint8_t on)
{
    log("bridge_cmd_flicker_reduction_mode_set STUB");
}

void bridge_cmd_focus_af_sensitivity(uint8_t normal)
{
    log("bridge_cmd_focus_af_sensitivity STUB");
}

void bridge_cmd_focus_direct(uint32_t p)
{
    log("bridge_cmd_focus_direct STUB");
}

void bridge_cmd_focus_far()
{
    log("bridge_cmd_focus_far STUB");
}

void bridge_cmd_focus_far_var(uint8_t p)
{
    log("bridge_cmd_focus_far_var STUB");
}

void bridge_cmd_focus_focus_inf()
{
    log("bridge_cmd_focus_focus_inf STUB");
}

void bridge_cmd_focus_ir_correction(uint8_t ir_light)
{
    log("bridge_cmd_focus_ir_correction STUB");
}

void bridge_cmd_focus_mode_auto()
{
    log("bridge_cmd_focus_mode_auto STUB");
}

void bridge_cmd_focus_mode_manual()
{
    log("bridge_cmd_focus_mode_manual STUB");
}

void bridge_cmd_focus_mode_toggle()
{
    log("bridge_cmd_focus_mode_toggle STUB");
}

void bridge_cmd_focus_near()
{
    log("bridge_cmd_focus_near STUB");
}

void bridge_cmd_focus_near_limit(uint32_t p)
{
    log("bridge_cmd_focus_near_limit STUB");
}

void bridge_cmd_focus_near_var(uint8_t p)
{
    log("bridge_cmd_focus_near_var STUB");
}

void bridge_cmd_focus_one_push_trigger()
{
    log("bridge_cmd_focus_one_push_trigger STUB");
}

void bridge_cmd_focus_stop()
{
    log("bridge_cmd_focus_stop STUB");
}

void bridge_cmd_gamma_black_gamma_level(uint8_t p)
{
    log("bridge_cmd_gamma_black_gamma_level STUB");
}

void bridge_cmd_gamma_black_gamma_range(uint8_t p)
{
    log("bridge_cmd_gamma_black_gamma_range STUB");
}

void bridge_cmd_gamma_black_level_direct(int p)
{
    log("bridge_cmd_gamma_black_level_direct STUB");
}

void bridge_cmd_gamma_black_level_down()
{
    log("bridge_cmd_gamma_black_level_down STUB");
}

void bridge_cmd_gamma_black_level_reset()
{
    log("bridge_cmd_gamma_black_level_reset STUB");
}

void bridge_cmd_gamma_black_level_up()
{
    log("bridge_cmd_gamma_black_level_up STUB");
}

void bridge_cmd_gamma_level(uint8_t p)
{
    log("bridge_cmd_gamma_level STUB");
}

void bridge_cmd_gamma_mode(uint8_t setting)
{
    log("bridge_cmd_gamma_mode STUB");
}

void bridge_cmd_gamma_offset(uint8_t polarity, uint8_t offset_width)
{
    log("bridge_cmd_gamma_offset STUB");
}

void bridge_cmd_gamma_pattern(uint32_t p)
{
    log("bridge_cmd_gamma_pattern STUB");
}

void bridge_cmd_hdmi_color_space(uint8_t rgb)
{
    log("bridge_cmd_hdmi_color_space STUB");
}

void bridge_cmd_hdmi_video_format_change(int w, int h, int f, char l)
{
    log("bridge_cmd_hdmi_video_format_change STUB");
}

void bridge_cmd_knee_mode(uint8_t manual)
{
    log("bridge_cmd_knee_mode STUB");
}

void bridge_cmd_knee_point(uint8_t p)
{
    log("bridge_cmd_knee_point STUB");
}

void bridge_cmd_knee_set(uint8_t on)
{
    log("bridge_cmd_knee_set STUB");
}

void bridge_cmd_knee_slope(uint8_t p)
{
    log("bridge_cmd_knee_slope STUB");
}

void bridge_cmd_menu_display_off()
{
    log("bridge_cmd_menu_display_off STUB");
}

void bridge_cmd_noise_reduction_2d_3d_manual_setting(uint8_t lvl_2d, uint8_t lvl_3d)
{
    log("bridge_cmd_noise_reduction_2d_3d_manual_setting STUB");
}

void bridge_cmd_noise_reduction_mode_level_set(uint8_t p)
{
    log("bridge_cmd_noise_reduction_mode_level_set STUB");
}

void bridge_cmd_pan_tilt_absolute_move(uint8_t speed, uint8_t p[5], uint8_t t[4])
{
    log("bridge_cmd_pan_tilt_absolute_move: %d, STUB", speed);
    log_p5t4(p, t);
}

void bridge_cmd_pan_tilt_directionals(int vert, int horiz, uint8_t pan_speed, uint8_t tilt_speed)
{
    /* speeds from 0 to 24 in visca */
    float trans_speed_x = (float)pan_speed / 24.f,
          trans_speed_y = (float)tilt_speed / 24.f,
          pan_x = (float)horiz * trans_speed_x,
          pan_y = (float)vert * trans_speed_y;

    log("bridge_cmd_pan_tilt_directionals: % d, % d, (%d, %d) -> (%.2f, %.2f)", vert, horiz,
            pan_speed, tilt_speed, pan_x, pan_y);

    if (vert == 0 && horiz == 0) {
        soap_ptz_stop_pantilt();
        return;
    }

    soap_ptz_continuous_move(pan_x, pan_y, 0);
}

void bridge_cmd_pan_tilt_home()
{
    log("bridge_cmd_pan_tilt_home");

    soap_ptz_goto_home();
}

void bridge_cmd_pan_tilt_limit_clear(uint8_t position)
{
    log("bridge_cmd_pan_tilt_limit_clear: %d STUB", position);
}

void bridge_cmd_pan_tilt_limit_set(uint8_t position, uint8_t p[5], uint8_t t[4])
{
    log("bridge_cmd_pan_tilt_limit_set: %d, STUB", position);
    log_p5t4(p, t);
}

void bridge_cmd_pan_tilt_ramp_curve(int p)
{
    log("bridge_cmd_pan_tilt_ramp_curve: %d STUB", p);
}

void bridge_cmd_pan_tilt_relative_move(uint8_t speed, uint8_t p[5], uint8_t t[4])
{
    log("bridge_cmd_pan_tilt_relative_move: %d, STUB", speed);
    log_p5t4(p, t);
}

void bridge_cmd_pan_tilt_reset()
{
    log("bridge_cmd_pan_tilt_reset STUB");
}

void bridge_cmd_pan_tilt_slow_mode(int p)
{
    log("bridge_cmd_pan_tilt_slow_mode: %d STUB", p);
}

void bridge_cmd_picture_profile_mode(uint8_t p)
{
    log("bridge_cmd_picture_profile_mode STUB");
}

void bridge_cmd_preset_drive_speed(uint8_t pn_speed, uint8_t direction_speed)
{
    log("bridge_cmd_preset_drive_speed STUB");
}

void bridge_cmd_preset_mode(uint8_t p)
{
    log("bridge_cmd_preset_mode STUB");
}

void bridge_cmd_preset_recall()
{
    log("bridge_cmd_preset_recall STUB");
}

void bridge_cmd_preset_reset()
{
    log("bridge_cmd_preset_reset STUB");
}

void bridge_cmd_preset_set()
{
    log("bridge_cmd_preset_set STUB");
}

void bridge_cmd_ptz_trace_delete(uint8_t trace_num)
{
    log("bridge_cmd_ptz_trace_delete STUB");
}

void bridge_cmd_ptz_trace_play(uint8_t start, uint8_t trace_num)
{
    log("bridge_cmd_ptz_trace_play");
}

void bridge_cmd_ptz_trace_rec(uint8_t start, uint8_t trace_num)
{
    log("bridge_cmd_ptz_trace_rec");
}

void bridge_cmd_system_hphase_direct(uint32_t p)
{
    log("bridge_cmd_system_hphase_direct STUB");
}

void bridge_cmd_system_hphase_set(uint8_t on)
{
    log("bridge_cmd_system_hphase_set STUB");
}

void bridge_cmd_system_img_flip(uint8_t on)
{
    log("bridge_cmd_system_img_flip STUB");
}

void bridge_cmd_system_pan_reverse(uint8_t on)
{
    log("bridge_cmd_system_pan_reverse STUB");
}

void bridge_cmd_system_tilt_reverse(uint8_t on)
{
    log("bridge_cmd_system_tilt_reverse STUB");
}

void bridge_cmd_tarry_set(uint8_t on)
{
    log("bridge_cmd_tarry_set STUB");
}

void bridge_cmd_tarry_tally_mode(uint8_t mode)
{
    log("bridge_cmd_tarry_tally_mode STUB");
}

void bridge_cmd_zoom_clear_image_set(uint8_t on)
{
    log("bridge_cmd_zoom_clear_image_set STUB");
}

void bridge_cmd_zoom_direct(uint32_t p)
{
    log("bridge_cmd_zoom_direct STUB");
}

void bridge_cmd_zoom_stop()
{
    log("bridge_cmd_zoom_stop");

    soap_ptz_stop_zoom();
}

void bridge_cmd_zoom_tele()
{
    log("bridge_cmd_zoom_tele");

    soap_ptz_continuous_move(0, 0, 0.5f);
}

void bridge_cmd_zoom_teleconvert_mode(uint8_t on)
{
    log("bridge_cmd_zoom_teleconvert_mode STUB");
}

void bridge_cmd_zoom_tele_var(uint8_t p)
{
    /* speed from 0 to 7 */
    float speed = (float)p / 7.f;

    log("bridge_cmd_zoom_tele_var %d -> %.2f", p, speed);

    soap_ptz_continuous_move(0, 0, speed);
}

void bridge_cmd_zoom_wide()
{
    log("bridge_cmd_zoom_wide");

    soap_ptz_continuous_move(0, 0, -0.5f);
}

void bridge_cmd_zoom_wide_var(uint8_t p)
{
    /* speed from 0 to 7 */
    float speed = (float)p / 7.f;

    log("bridge_cmd_zoom_wide_var %d", p);

    soap_ptz_continuous_move(0, 0, -speed);
}

void bridge_cmd_memory_reset(uint8_t num)
{
    log("bridge_cmd_memory_reset %d STUB", num);
}

void bridge_cmd_memory_set(uint8_t num)
{
    log("bridge_cmd_memory_set %d STUB", num);
}

void bridge_cmd_memory_recall(uint8_t num)
{
    log("bridge_cmd_memory_recall %d STUB", num);
}

void bridge_cmd_pan_tilt_relative_position(int pan_speed, int tilt_speed, int pan_pos, int tilt_pos)
{
    log("bridge_cmd_pan_tilt_relative_position pan_speed=%d, tilt_speed=%d, pan_pos=%d, tilt_pos=%d"
            "STUB", pan_speed, tilt_speed, pan_pos, tilt_pos);
}

void bridge_cmd_pan_tilt_absolute_position(int pan_speed, int tilt_speed, int pan_pos, int tilt_pos)
{
    log("bridge_cmd_pan_tilt_absolute_position pan_speed=%d, tilt_speed=%d, pan_pos=%d, tilt_pos=%d"
            "STUB", pan_speed, tilt_speed, pan_pos, tilt_pos);
}

