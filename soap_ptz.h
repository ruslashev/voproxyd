#pragma once

#include "address_manager.h"
#include "soap_global.h"
#include "soap_utils.h"
#include "worker.h"

#define soap_ptz_prelude() \
    soap_t *soap = g_soap; \
    struct soap_instance *instance = address_mngr_get_soap_instance_from_fd(g_current_event_fd); \
    const char *ptz_xaddr = soap_utils_get_ptz_xaddr(&instance->services); \
    profile_t *profile = &instance->profiles.Profiles[instance->profile_idx]; \
    char *profile_token = profile->token; \
    soap_utils_auth();

void soap_ptz_continuous_move(float pan_x, float pan_y, float zoom);
void soap_ptz_goto_home();
void soap_ptz_stop_pantilt();
void soap_ptz_stop_zoom();

