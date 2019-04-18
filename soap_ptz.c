#include "soap_ptz.h"
#include "soap_utils.h"

#define soap_ptz_prelude() \
    soap_t *soap = g_soap; \
    struct soap_instance *instance = address_mngr_get_soap_instance_from_fd(g_current_event_fd); \
    const char *ptz_xaddr = soap_utils_get_ptz_xaddr(instance->services); \
    profile_t *profile = &instance->profiles->Profiles[instance->profile_idx]; \
    char *profile_token = profile->token; \
    soap_utils_auth();

void soap_ptz_continuous_move(float pan_x, float pan_y, float zoom)
{
    struct _tptz__ContinuousMove move;
    struct _tptz__ContinuousMoveResponse move_resp;
    struct tt__PTZSpeed velocity;
    struct tt__Vector2D pantilt;
    struct tt__Vector1D zoom_s;

    soap_ptz_prelude();

    pantilt.x = pan_x;
    pantilt.y = pan_y;
    pantilt.space = profile->PTZConfiguration->DefaultContinuousPanTiltVelocitySpace;

    zoom_s.x = zoom;
    zoom_s.space = profile->PTZConfiguration->DefaultContinuousZoomVelocitySpace;

    velocity.PanTilt = &pantilt;
    velocity.Zoom = &zoom_s;

    move.Velocity = &velocity;
    move.ProfileToken = profile_token;

    if (soap_call___tptz__ContinuousMove(soap, ptz_xaddr, NULL, &move, &move_resp) != SOAP_OK)
        soap_die(soap, "failed to do continous move");
}

void soap_ptz_goto_home()
{
    struct _tptz__GotoHomePosition gohome;
    struct _tptz__GotoHomePositionResponse gohome_resp;

    soap_ptz_prelude();

    gohome.ProfileToken = profile_token;
    gohome.Speed = profile->PTZConfiguration->DefaultPTZSpeed;

    log("call gotohomeposition");

    if (soap_call___tptz__GotoHomePosition(soap, ptz_xaddr, NULL, &gohome, &gohome_resp) != SOAP_OK)
        soap_die(soap, "failed to goto home position");
}

static void stop(int pantilt, int zoom)
{
    struct _tptz__Stop stop;
    struct _tptz__StopResponse stop_resp;

    enum xsd__boolean pantilt_s = soap_utils_int_to_bool(pantilt);
    enum xsd__boolean zoom_s = soap_utils_int_to_bool(zoom);

    soap_ptz_prelude();

    stop.ProfileToken = profile_token;
    stop.PanTilt = &pantilt_s;
    stop.Zoom = &zoom_s;

    log("call ptz stop pantilt %d zoom %d", pantilt, zoom);

    if (soap_call___tptz__Stop(soap, ptz_xaddr, NULL, &stop, &stop_resp) != SOAP_OK)
        soap_die(soap, "failed to stop ptz");
}

void soap_ptz_stop_pantilt()
{
    stop(1, 0);
}

void soap_ptz_stop_zoom()
{
    stop(0, 1);
}

static void get_capabilities()
{
    struct _tptz__GetServiceCapabilities x;
    struct _tptz__GetServiceCapabilitiesResponse x_resp;

    soap_ptz_prelude();

    log("call getcapabilities");

    if (soap_call___tptz__GetServiceCapabilities(soap, ptz_xaddr, NULL, &x, &x_resp) != SOAP_OK)
        soap_die(soap, "failed to get capabilities");

    enum xsd__boolean *status_position = x_resp.Capabilities->StatusPosition;

    if (status_position == NULL)
        log("it's NULL");
    else {
        log("try to dereference");

        enum xsd__boolean status_position_what = *status_position;

        log("StatusPosition: %d", soap_utils_bool_to_int(status_position_what));

        /* log("MoveStatus: %d", soap_utils_bool_to_int(x_resp.Capabilities->MoveStatus)); */
    }
}

void soap_ptz_get_position(float *pan, float *tilt, float *zoom)
{
    struct _tptz__GetStatus getstatus;
    struct _tptz__GetStatusResponse getstatus_resp;

    get_capabilities();

    soap_ptz_prelude();

    getstatus.ProfileToken = profile_token;

    log("call getstatus");

    if (soap_call___tptz__GetStatus(soap, ptz_xaddr, NULL, &getstatus, &getstatus_resp) != SOAP_OK)
        soap_die(soap, "failed to get status");

    if (pan)
        *pan = getstatus_resp.PTZStatus->Position->PanTilt->x;
    if (tilt)
        *tilt = getstatus_resp.PTZStatus->Position->PanTilt->y;
    if (zoom)
        *zoom = getstatus_resp.PTZStatus->Position->Zoom->x;
}

