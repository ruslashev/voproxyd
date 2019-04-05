#pragma once

#include "soap_utils.h"

#define soap_ptz_prelude() \
    soap_t *soap = g_soap; \
    const char *ptz_xaddr = soap_utils_get_ptz_xaddr(&g_services); \
    profile_t* profile = &g_profiles.Profiles[g_config.profile_idx]; \
    char *profile_token = profile->token; \
    soap_utils_auth();

void soap_ptz_continuous_move(float pan_x, float pan_y, float zoom);
void soap_ptz_goto_home();
void soap_ptz_stop_pantilt();
void soap_ptz_stop_zoom();

