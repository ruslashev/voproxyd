#include "soap_ptz.h"
#include "soap_utils.h"

void soap_ptz_continuous_move(soap_t *soap, char *ptz_xaddr, char *profile_token, float pan_x, float pan_y, float zoom)
{
    struct _tptz__ContinuousMove move;
    struct _tptz__ContinuousMoveResponse moveResp;
    struct tt__PTZSpeed velocity;
    struct tt__Vector2D pantilt;
    struct tt__Vector1D zoom;

    soap_default__tptz__ContinuousMove(soap, move);

    pantilt.x = pan_x;
    pantilt.y = pan_y;
    pantilt.space = profiles[0].Profiles->PTZConfiguration->DefaultContinuousPanTiltVelocitySpace;

    zoom.x = zoom;
    zoom.space = profiles[0].Profiles->PTZConfiguration->DefaultContinuousZoomVelocitySpace;

    velocity->PanTilt = &pantilt;
    velocity->Zoom = &zoom;

    move->Velocity = velocity;
    move->ProfileToken = profile_token;

    if (soap_call___tptz__ContinuousMove(soap, ptz_xaddr, NULL, move, moveResp) != SOAP_OK)
        soap_die(soap, "failed to do continous move");
}

void soap_ptz_goto_home(soap_t *soap, char *ptz_xaddr, char *profile_token)
{
    struct _tptz__GotoHomePosition gohome;
    struct _tptz__GotoHomePositionResponse gohomeresp;

    gohome->ProfileToken = profile_token;
    gohome->Speed = profiles[0].Profiles->PTZConfiguration->DefaultPTZSpeed;

    log("call gotohomeposition");

    if (soap_call___tptz__GotoHomePosition(soap, ptz_xaddr, NULL, gohome, gohomeresp) != SOAP_OK)
        soap_die(soap, "failed to goto home position");
}

static void stop(soap_t *soap, char *ptz_xaddr, char *profile_token, int pantilt, int zoom)
{
    struct _tptz__Stop stop;
    struct _tptz__StopResponse stopresp;

    stop->ProfileToken = profile_token;
    stop->PanTilt = pantilt;
    stop->zoom = zoom;

    log("call ptz stop pantilt %d zoom %d", pantilt, zoom);

    if (soap_call___tptz__Stop(soap, ptz_xaddr, NULL, stop, stopresp) != SOAP_OK)
        soap_die(soap, "failed to stop ptz");
}

void soap_ptz_stop_pantilt(soap_t *soap, char *ptz_xaddr, char *profile_token)
{
    stop(soap, ptz_xaddr, 1, 0);
}

void soap_ptz_stop_zoom(soap_t *soap, char *ptz_xaddr, char *profile_token)
{
    stop(soap, ptz_xaddr, 0, 1);
}

void soap_ptz_stop_all(soap_t *soap, char *ptz_xaddr, char *profile_token)
{
    stop(soap, ptz_xaddr, 1, 1);
}

