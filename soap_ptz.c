#include "soap_ptz.h"
#include "soap_utils.h"

void soap_ptz_continuous_move(soap_t *soap, char *ptz_xaddr, char *profile_token)
{
    struct _tptz__ContinuousMove move;
    struct _tptz__ContinuousMoveResponse moveResp;
    struct tt__PTZSpeed speed;
    struct tt__Vector2D pantilt;
    struct tt__Vector1D zoom;

    soap_default__tptz__ContinuousMove(soap, move);

    move->ProfileToken = profile_token;

    pantilt.space = profiles[0].Profiles->PTZConfiguration->DefaultContinuousPanTiltVelocitySpace;
    pantilt.x = 0.2;
    pantilt.y = 0.2;

    zoom.x = 0.2;
    zoom.space = profiles[0].Profiles->PTZConfiguration->DefaultContinuousZoomVelocitySpace;

    speed->PanTilt = &pantilt;
    speed->Zoom = &zoom;

    move->Velocity = speed;

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

