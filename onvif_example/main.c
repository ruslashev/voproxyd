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

int ContinuousMove(struct soap* soap, profiles_t *profiles, char* ptz_xaddr)
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

int go_to_home_pos(struct soap* soap, profiles_t *profiles, char* ptz_xaddr)
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

void print_device_info(soap_t *soap)
{
    device_info_t device_info;

    soap_utils_get_device_information(soap, SERVICE_ENDPOINT, &device_info);

    log("Manufacturer:    %s", device_info.Manufacturer);
    log("Model:           %s", device_info.Model);
    log("FirmwareVersion: %s", device_info.FirmwareVersion);
    log("SerialNumber:    %s", device_info.SerialNumber);
    log("HardwareId:      %s", device_info.HardwareId);
}

void worker(struct soap *soap)
{
    services_t services;
    char *media_xaddr;
    char *ptz_xaddr;
    profiles_t profiles;

    soap_utils_set_credentials(soap, ONVIF_USER, ONVIF_PASSWORD);

    print_device_info(soap);

    soap_utils_get_services(soap, SERVICE_ENDPOINT, &services);

    media_xaddr = soap_utils_get_media_xaddr(&services);

    ptz_xaddr = soap_utils_get_ptz_xaddr(&services);

    soap_utils_get_profiles(soap, media_xaddr, &profiles);

    if (profiles.Profiles->Name != NULL)
        log("profiles name: %s", profiles.Profiles->Name);

    if (profiles.Profiles->VideoEncoderConfiguration != NULL)
        log("profiles token: %s", profiles.Profiles->VideoEncoderConfiguration->Name);

    go_to_home_pos(soap, &profiles, ptz_xaddr);
}

int main()
{
    struct soap *soap = soap_new();
    if (soap == NULL)
        die(1, "failed to create soap instance");

    soap->connect_timeout = soap->recv_timeout = soap->send_timeout = 5;

    worker(soap);

    soap_destroy(soap);
    soap_end(soap);
    soap_free(soap);

    return 0;
}

