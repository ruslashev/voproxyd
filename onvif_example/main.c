#include "secrets.h"
#include "../onvif_wsdd_stubs.h"
#include "../log.h"
#include "../soap_utils.h"
#include "../deps/onvif/soapH.h"
#include "../deps/onvif/soapStub.h"
#include "../deps/onvif/wsseapi.h"
#include "../deps/onvif/nsmaps/wsdd.nsmap"
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

int ContinuousMove(soap_t* soap, profiles_t *profiles, char* ptz_xaddr)
{
    struct _tptz__ContinuousMove *move = soap_malloc(soap, sizeof(struct _tptz__ContinuousMove));

    soap_default__tptz__ContinuousMove(soap, move);

    struct _tptz__ContinuousMoveResponse *moveResp = soap_malloc(soap,
            sizeof(struct _tptz__ContinuousMoveResponse));

    log("set profiles");
    move->ProfileToken = profiles->Profiles[0].token;
    struct tt__PTZSpeed *speed = soap_malloc(soap, sizeof(struct tt__PTZSpeed));
    struct tt__Vector2D pantilt;
    struct tt__Vector1D zoom;
    pantilt.x = 0.2;
    pantilt.y = 0.2;
    pantilt.space = profiles[0].Profiles->PTZConfiguration->DefaultContinuousPanTiltVelocitySpace;
    zoom.x = 0.2;
    zoom.space = profiles[0].Profiles->PTZConfiguration->DefaultContinuousZoomVelocitySpace;

    speed->PanTilt = &pantilt;
    speed->Zoom = &zoom;

    move->Velocity = speed;

    log("call continousmove");

    int result = soap_call___tptz__ContinuousMove(soap, ptz_xaddr, NULL, move, moveResp);

    log("result=%d", result);

    if (result != SOAP_OK)
        soap_die(soap, "continousmove failed");

    return result;
}

int go_to_home_pos(soap_t* soap, profiles_t *profiles, char* ptz_xaddr)
{
    struct _tptz__GotoHomePosition *gohome = soap_malloc(soap,
            sizeof(struct _tptz__GotoHomePosition));
    struct _tptz__GotoHomePositionResponse *gohomeresp = soap_malloc(soap,
            sizeof(struct _tptz__GotoHomePositionResponse));

    gohome->ProfileToken = profiles->Profiles[0].token;
    gohome->Speed = profiles[0].Profiles->PTZConfiguration->DefaultPTZSpeed;

    log("call gotohomeposition");

    int result = soap_call___tptz__GotoHomePosition(soap, ptz_xaddr, NULL, gohome, gohomeresp);

    log("result=%d", result);

    if (result != SOAP_OK)
        soap_die(soap, "gotohomeposition failed");

    return result;
}

void worker(soap_t *soap)
{
    services_t services;
    profiles_t profiles;
    char *snapshot_uri;

    soap_utils_set_credentials(soap, ONVIF_USER, ONVIF_PASSWORD);

    log("device info:");
    soap_utils_print_device_info(soap, SERVICE_ENDPOINT);
    log(" ");

    soap_utils_get_services(soap, SERVICE_ENDPOINT, &services);

    soap_utils_get_profiles(soap, soap_utils_get_media_xaddr(&services), &profiles);

    log("iterating %d profile(s):", profiles.__sizeProfiles);
    for (int i = 0; i < profiles.__sizeProfiles; ++i) {
        log("%2d) name: %s", i + 1, profiles.Profiles[i].Name);
        log("    token: %s", profiles.Profiles[i].token);

        soap_utils_get_snapshot_uri(soap, SERVICE_ENDPOINT, profiles.Profiles[i].token, &snapshot_uri);

        log("    snapshot uri: %s", snapshot_uri);
    }

    /* go_to_home_pos(soap, &profiles, soap_utils_get_ptz_xaddr(&services)); */
}

int main()
{
    soap_t *soap = soap_new();
    if (soap == NULL)
        die(1, "failed to create soap instance");

    soap->connect_timeout = soap->recv_timeout = soap->send_timeout = 5;

    worker(soap);

    soap_destroy(soap);
    soap_end(soap);
    soap_free(soap);

    return 0;
}

