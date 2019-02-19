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

char* get_services_xaddr(struct soap *soap, struct _tds__GetServices *get_services_trt,
        struct _tds__GetServicesResponse *services)
{
    int result;

    get_services_trt->IncludeCapability = xsd__boolean__false_;

    printf("[%d][%s][---- Getting Device Services ----]\n", __LINE__, __func__);

    soap_wsse_add_UsernameTokenDigest(soap, "user", ONVIF_USER, ONVIF_PASSWORD);

    result = soap_call___tds__GetServices(soap, SERVICE_ENDPOINT, NULL, get_services_trt, services);

    printf("[%d][%s][result = %d]\n", __LINE__, __func__, result);

    if (services->Service == NULL)
        soap_die(soap, "failed to get services");

    for (int i = 0; i < services->__sizeService; i++) {
        if (!strcmp(services->Service[i].Namespace, SOAP_NAMESPACE_OF_trt)) {
            printf("[%d][%s][MediaServiceAddress:%s]\n", __LINE__, __func__,
                    services->Service[i].XAddr);
            return services->Service[i].XAddr;
        }
    }

    return NULL;
}

char* get_ptz_xaddr(struct _tds__GetServicesResponse *services)
{
    for (int i = 0; i < services->__sizeService; i++) {
        if (!strcmp(services->Service[i].Namespace, "http://www.onvif.org/ver20/ptz/wsdl")) {
            printf("[%d][%s][MediaServiceAddress:%s]\n", __LINE__, __func__,
                    services->Service[i].XAddr);
            return services->Service[i].XAddr;
        }
    }

    return NULL;
}

int get_profiles(struct soap *soap, struct _trt__GetProfiles *get_profiles_trt,
                  struct _trt__GetProfilesResponse *profiles, char *services_xaddr)
{
    int result;

    printf("[%d][%s][---- Getting Profiles ----]\n", __LINE__, __func__);

    /* TODO needed? */
    soap_wsse_add_UsernameTokenDigest(soap, "user", ONVIF_USER, ONVIF_PASSWORD);

    result = soap_call___trt__GetProfiles(soap, services_xaddr, NULL, get_profiles_trt, profiles);

    printf("[%d][%s][result = %d][soap_error = %d]\n", __LINE__, __func__, result, soap->error);

    if (result == SOAP_EOF) {
        printf("[%d][%s][Error Number:%d] [Fault Code:%s] [Fault Reason:%s]\n", __LINE__, __func__,
                soap->error, *soap_faultcode(soap), *soap_faultstring(soap));
        return -1;
    }

    if (profiles->Profiles != NULL) {
        if (profiles->Profiles->Name != NULL)
            printf("[%d][%s][Profiles Name:%s]\n",__LINE__, __func__, profiles->Profiles->Name);
        if (profiles->Profiles->VideoEncoderConfiguration != NULL)
            printf("[%d][%s][Profiles Token:%s]\n",__LINE__, __func__,
                    profiles->Profiles->VideoEncoderConfiguration->Name);
    }

    return result;
}

int ContinuousMove(struct soap* soap, struct _trt__GetProfilesResponse *profiles, char* ptz_xaddr)
{
    struct _tptz__ContinuousMove *move = soap_malloc(soap, sizeof(struct _tptz__ContinuousMove));

    soap_default__tptz__ContinuousMove(soap, move);

    struct _tptz__ContinuousMoveResponse *moveResp = soap_malloc(soap,
            sizeof(struct _tptz__ContinuousMoveResponse));

    printf("[%d][%s] setprofiles\n", __LINE__, __func__);
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

    printf("[%d][%s] setArg speed\n", __LINE__, __func__);
    move->Velocity = speed;
    printf("[%d][%s] setArg timeout\n", __LINE__, __func__);
    printf("[%d][%s] tokendigest\n", __LINE__, __func__);
    soap_wsse_add_UsernameTokenDigest(soap, "user", ONVIF_USER, ONVIF_PASSWORD);
    printf("[%d][%s] [endpoint : %s]\n", __LINE__, __func__, ptz_xaddr);
    int result = soap_call___tptz__ContinuousMove(soap, ptz_xaddr, NULL, move, moveResp);

    printf("[%d][%s][result = %d][soap_error = %d]\n", __LINE__, __func__, result, soap->error);
    if (result == SOAP_EOF) {
        printf("[%d][%s][Error Number:%d] [Fault Code:%s] [Fault Reason:%s]\n", __LINE__, __func__,
                soap->error, *soap_faultcode(soap), *soap_faultstring(soap));
        return SOAP_ERR;
    }

    return result;
}

int go_to_home_pos(struct soap* soap, struct _trt__GetProfilesResponse *profiles, char* ptz_xaddr)
{
    struct _tptz__GotoHomePosition *gohome = soap_malloc(soap,
            sizeof(struct _tptz__GotoHomePosition));
    struct _tptz__GotoHomePositionResponse *gohomeresp = soap_malloc(soap,
            sizeof(struct _tptz__GotoHomePositionResponse));

    gohome->ProfileToken = profiles->Profiles[0].token;
    gohome->Speed = profiles[0].Profiles->PTZConfiguration->DefaultPTZSpeed;

    int result = soap_call___tptz__GotoHomePosition(soap, ptz_xaddr, NULL, gohome, gohomeresp);

    printf("[%d][%s][result = %d][soap_error = %d]\n", __LINE__, __func__, result, soap->error);
    if (result == SOAP_EOF) {
        printf("[%d][%s][Error Number:%d][Fault Code:%s] [Fault Reason:%s]\n", __LINE__, __func__,
                soap->error, *soap_faultcode(soap), *soap_faultstring(soap));
        return SOAP_ERR;
    }

    return result;
}

void worker(struct soap *soap)
{
    struct _trt__GetProfiles get_profiles_trt;
    struct _trt__GetProfilesResponse profiles;
    struct _tds__GetServices get_services_trt;
    struct _tds__GetServicesResponse services;
    char *services_xaddr;
    char *ptz_xaddr;

    soap_default__tds__GetServices(soap, &get_services_trt);

    services_xaddr = get_services_xaddr(soap, &get_services_trt, &services);
    if (services_xaddr == NULL)
        die(1, "get_services_xaddr() failed");

    ptz_xaddr = get_ptz_xaddr(&services);
    if (ptz_xaddr == NULL)
        die(1, "get_ptz_xaddr() failed");

    get_profiles(soap, &get_profiles_trt, &profiles, services_xaddr);

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

