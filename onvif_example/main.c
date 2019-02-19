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

#define ONVIF_USER          "x"
#define ONVIF_PASSWORD      "x"

#define SERVICE_ENDPOINT    "http://x.x.x.x:2000/onvif/device_service"

void get_services(struct soap *soap, struct _tds__GetServices *get_services_trt, services_t *services)
{
    int result;

    log("getting device services");

    soap_wsse_add_UsernameTokenDigest(soap, "user", ONVIF_USER, ONVIF_PASSWORD);

    result = soap_call___tds__GetServices(soap, SERVICE_ENDPOINT, NULL, get_services_trt, services);

    log("result=%d", result);

    if (services->Service == NULL)
        soap_die(soap, "failed to get services");
}

int get_profiles(struct soap *soap, struct _trt__GetProfiles *get_profiles_trt,
                  profiles_t *profiles, char *media_xaddr)
{
    int result;

    log("getting profiles");

    /* TODO needed? */
    soap_wsse_add_UsernameTokenDigest(soap, "user", ONVIF_USER, ONVIF_PASSWORD);

    result = soap_call___trt__GetProfiles(soap, media_xaddr, NULL, get_profiles_trt, profiles);

    log("result=%d", result);

    if (result != SOAP_OK)
        soap_die(soap, "failed to get profiles");

    if (profiles->Profiles == NULL)
        die(1, "null profiles");

    if (profiles->Profiles->Name != NULL)
        log("profiles name: %s", profiles->Profiles->Name);

    if (profiles->Profiles->VideoEncoderConfiguration != NULL)
        log("profiles token: %s", profiles->Profiles->VideoEncoderConfiguration->Name);

    return result;
}

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

    soap_wsse_add_UsernameTokenDigest(soap, "user", ONVIF_USER, ONVIF_PASSWORD);

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

void worker(struct soap *soap)
{
    struct _trt__GetProfiles get_profiles_trt;
    profiles_t profiles;
    struct _tds__GetServices get_services_trt;
    services_t services;
    char *media_xaddr;
    char *ptz_xaddr;

    soap_default__tds__GetServices(soap, &get_services_trt);

    get_services_trt.IncludeCapability = xsd__boolean__false_;

    get_services(soap, &get_services_trt, &services);

    media_xaddr = soap_get_media_xaddr(&services);
    if (media_xaddr == NULL)
        die(1, "get_media_xaddr() failed");

    ptz_xaddr = soap_get_ptz_xaddr(&services);
    if (ptz_xaddr == NULL)
        die(1, "get_ptz_xaddr() failed");

    get_profiles(soap, &get_profiles_trt, &profiles, media_xaddr);

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

