#include "soap_ptz.h"
#include "soap_utils.h"

void soap_ptz_continuous_move(soap_t *soap, char *ptz_xaddr, char *profile_token, float pan_x, float pan_y, float zoom)
{
    struct _tptz__ContinuousMove move;
    struct _tptz__ContinuousMoveResponse move_resp;
    struct tt__PTZSpeed velocity;
    struct tt__Vector2D pantilt;
    struct tt__Vector1D zoom_s;

    pantilt.x = pan_x;
    pantilt.y = pan_y;
    /* TODO: bs */
    pantilt.space = g_profiles.Profiles->PTZConfiguration->DefaultContinuousPanTiltVelocitySpace;

    zoom_s.x = zoom;
    /* TODO: also bs */
    zoom_s.space = g_profiles.Profiles->PTZConfiguration->DefaultContinuousZoomVelocitySpace;

    velocity.PanTilt = &pantilt;
    velocity.Zoom = &zoom_s;

    move.Velocity = &velocity;
    move.ProfileToken = profile_token;

    if (soap_call___tptz__ContinuousMove(soap, ptz_xaddr, NULL, &move, &move_resp) != SOAP_OK)
        soap_die(soap, "failed to do continous move");
}

void soap_ptz_goto_home(soap_t *soap, char *ptz_xaddr, char *profile_token)
{
    struct _tptz__GotoHomePosition gohome;
    struct _tptz__GotoHomePositionResponse gohome_resp;

    gohome.ProfileToken = profile_token;
    /* TODO: identifying bs */
    gohome.Speed = g_profiles.Profiles->PTZConfiguration->DefaultPTZSpeed;

    log("call gotohomeposition");

    if (soap_call___tptz__GotoHomePosition(soap, ptz_xaddr, NULL, &gohome, &gohome_resp) != SOAP_OK)
        soap_die(soap, "failed to goto home position");
}

static void stop(soap_t *soap, char *ptz_xaddr, char *profile_token, int pantilt, int zoom)
{
    struct _tptz__Stop stop;
    struct _tptz__StopResponse stop_resp;

    enum xsd__boolean pantilt_s = soap_utils_int_to_bool(pantilt);
    enum xsd__boolean zoom_s = soap_utils_int_to_bool(zoom);

    stop.ProfileToken = profile_token;
    stop.PanTilt = &pantilt_s;
    stop.Zoom = &zoom_s;

    log("call ptz stop pantilt %d zoom %d", pantilt, zoom);

    if (soap_call___tptz__Stop(soap, ptz_xaddr, NULL, &stop, &stop_resp) != SOAP_OK)
        soap_die(soap, "failed to stop ptz");
}

void soap_ptz_stop_pantilt(soap_t *soap, char *ptz_xaddr, char *profile_token)
{
    stop(soap, ptz_xaddr, profile_token, 1, 0);
}

void soap_ptz_stop_zoom(soap_t *soap, char *ptz_xaddr, char *profile_token)
{
    stop(soap, ptz_xaddr, profile_token, 0, 1);
}

