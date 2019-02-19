#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "../deps/onvif/soapH.h"
#include "../deps/onvif/soapStub.h"
#include "../deps/onvif/wsseapi.h"
#include "../deps/onvif/wsddapi.h"
#include "../deps/onvif/nsmaps/wsdd.nsmap"

#define ONVIF_USER          "x"
#define ONVIF_PASSWORD      "x"

#define SERVICE_ENDPOINT    "http://x.x.x.x:2000/onvif/device_service"

#define log(...) do { printf(__VA_ARGS__); puts(""); } while (0)

#define die(...) do { log(__VA_ARGS__); exit(1); } while (0)

void wsdd_event_Hello(struct soap *soap, unsigned int InstanceId, const char *SequenceId, unsigned int MessageNumber, const char *MessageID, const char *RelatesTo, const char *EndpointReference, const char *Types, const char *Scopes, const char *MatchBy, const char *XAddrs, unsigned int MetadataVersion)
{ }

void wsdd_event_Bye(struct soap *soap, unsigned int InstanceId, const char *SequenceId, unsigned int MessageNumber, const char *MessageID, const char *RelatesTo, const char *EndpointReference, const char *Types, const char *Scopes, const char *MatchBy, const char *XAddrs, unsigned int *MetadataVersion)
{ }

soap_wsdd_mode wsdd_event_Probe(struct soap *soap, const char *MessageID, const char *ReplyTo, const char *Types, const char *Scopes, const char *MatchBy, struct wsdd__ProbeMatchesType *ProbeMatches)
{
    return SOAP_WSDD_ADHOC;
}

void wsdd_event_ProbeMatches(struct soap *soap, unsigned int InstanceId, const char *SequenceId, unsigned int MessageNumber, const char *MessageID, const char *RelatesTo, struct wsdd__ProbeMatchesType *ProbeMatches)
{ }

soap_wsdd_mode wsdd_event_Resolve(struct soap *soap, const char *MessageID, const char *ReplyTo, const char *EndpointReference, struct wsdd__ResolveMatchType *match)
{
    return SOAP_WSDD_ADHOC;
}

void wsdd_event_ResolveMatches(struct soap *soap, unsigned int InstanceId, const char * SequenceId, unsigned int MessageNumber, const char *MessageID, const char *RelatesTo, struct wsdd__ResolveMatchType *match)
{ }

struct soap* create_soap_instance()
{
    struct soap* soap = soap_new();

    /* soap->recv_timeout = 50; */
    /* soap_set_namespaces(soap, namespaces); */

    return soap;
}

char* GetDeviceServices(struct soap* soap, struct _tds__GetServices *getServices, struct _tds__GetServicesResponse *getServicesResponse)
{
    char* mediaEndPoint = NULL;
    getServices->IncludeCapability = xsd__boolean__false_;

    printf("[%d][%s][---- Getting Device Services ----]\n", __LINE__, __func__);
    soap_wsse_add_UsernameTokenDigest(soap,"user", ONVIF_USER, ONVIF_PASSWORD);
    int result = soap_call___tds__GetServices(soap, SERVICE_ENDPOINT, NULL, getServices, getServicesResponse);
    printf("[%d][%s<%s!>][result = %d][soap_error = %d]\n", __LINE__, __func__, result ? "失败":"成功", result, soap->error);

    int i;
    if (getServicesResponse->Service != NULL) {
        for(i = 0; i < getServicesResponse->__sizeService; i++) {
            if (!strcmp(getServicesResponse->Service[i].Namespace, SOAP_NAMESPACE_OF_trt)) {
                mediaEndPoint = getServicesResponse->Service[i].XAddr;
                printf("[%d][%s][MediaServiceAddress:%s]\n",__LINE__, __func__, mediaEndPoint);
                break;
            }
        }
    }

    return mediaEndPoint;
}

char* GetPTZServier(struct _tds__GetServicesResponse *getServicesResponse)
{
    char *ep = "";
    int i;
    if (getServicesResponse->Service != NULL) {
        for(i = 0; i < getServicesResponse->__sizeService; i++) {
            //SOAP_NAMESPACE_OF_tptz
            if (!strcmp(getServicesResponse->Service[i].Namespace, "http://www.onvif.org/ver20/ptz/wsdl")) {
                ep = getServicesResponse->Service[i].XAddr;
                printf("[%d][%s][MediaServiceAddress:%s]\n",__LINE__, __func__, ep);
                break;
            }
        }
    }
    return ep;
}

int GetProfiles(struct soap* soap, struct _trt__GetProfiles *trt__GetProfiles,
                  struct _trt__GetProfilesResponse *trt__GetProfilesResponse, char* media_ep)
{
    int result=0;
    printf("[%d][%s][---- Getting Profiles ----]\n", __LINE__, __func__);
    soap_wsse_add_UsernameTokenDigest(soap,"user", ONVIF_USER, ONVIF_PASSWORD);
    result = soap_call___trt__GetProfiles(soap, media_ep, NULL, trt__GetProfiles, trt__GetProfilesResponse);

    printf("[%d][%s<%s!>][result = %d][soap_error = %d]\n", __LINE__, __func__, result ? "失败":"成功", result, soap->error);
    if (result == SOAP_EOF) {
        printf("[%d][%s][Error Number:%d] [Falut Code:%s] [Falut Reason:%s]\n", __LINE__, __func__, soap->error, *soap_faultcode(soap), *soap_faultstring(soap));
        return -1;
    }
    if(trt__GetProfilesResponse->Profiles != NULL) {
        if(trt__GetProfilesResponse->Profiles->Name != NULL)
            printf("[%d][%s][Profiles Name:%s]\n",__LINE__, __func__, trt__GetProfilesResponse->Profiles->Name);
        if(trt__GetProfilesResponse->Profiles->VideoEncoderConfiguration != NULL)
            printf("[%d][%s][Profiles Token:%s]\n",__LINE__, __func__, trt__GetProfilesResponse->Profiles->VideoEncoderConfiguration->Name);
    }
    return result;
}

int ContinuousMove(struct soap* soap, struct _trt__GetProfilesResponse *profiles, char* ptz_ep)
{
    struct _tptz__ContinuousMove *move = (struct _tptz__ContinuousMove*)soap_malloc(soap, sizeof(struct _tptz__ContinuousMove));
    soap_default__tptz__ContinuousMove(soap, move);

    struct _tptz__ContinuousMoveResponse *moveResp = (struct _tptz__ContinuousMoveResponse*)soap_malloc(soap, sizeof(struct _tptz__ContinuousMoveResponse));

    printf("[%d][%s] setprofiles\n", __LINE__, __func__);
    move->ProfileToken = profiles->Profiles[0].token;
    struct tt__PTZSpeed *speed = (struct tt__PTZSpeed*)soap_malloc(soap, sizeof(struct tt__PTZSpeed));
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
    printf("[%d][%s] [endpoint : %s]\n", __LINE__, __func__, ptz_ep);
    int result = soap_call___tptz__ContinuousMove(soap, ptz_ep, NULL, move, moveResp);

    printf("[%d][%s<%s!>][result = %d][soap_error = %d]\n", __LINE__, __func__, result ? "失败":"成功", result, soap->error);
    if (result == SOAP_EOF) {
        printf("[%d][%s][Error Number:%d] [Falut Code:%s] [Falut Reason:%s]\n", __LINE__, __func__, soap->error, *soap_faultcode(soap), *soap_faultstring(soap));
        return SOAP_ERR;
    }
    return result;
}

int GotoHomePos(struct soap* soap, struct _trt__GetProfilesResponse *profiles, char* ep)
{
    // get/set/goto 3setp;
    struct _tptz__GotoHomePosition *gohome = (struct _tptz__GotoHomePosition*)soap_malloc(soap, sizeof(struct _tptz__GotoHomePosition));
    struct _tptz__GotoHomePositionResponse *gohomeresp = (struct _tptz__GotoHomePositionResponse*)soap_malloc(soap, sizeof(struct _tptz__GotoHomePositionResponse));

    gohome->ProfileToken = profiles->Profiles[0].token;
    gohome->Speed = profiles[0].Profiles->PTZConfiguration->DefaultPTZSpeed;

    int result = soap_call___tptz__GotoHomePosition(soap, ep, NULL, gohome, gohomeresp);

    printf("[%d][%s<%s!>][result = %d][soap_error = %d]\n", __LINE__, __func__, result ? "失败":"成功", result, soap->error);
    if (result == SOAP_EOF) {
        printf("[%d][%s][Error Number:%d][Falut Code:%s] [Falut Reason:%s]\n", __LINE__, __func__, soap->error, *soap_faultcode(soap), *soap_faultstring(soap));
        return SOAP_ERR;
    }
    return result;

}

void worker(struct soap *soap)
{
    struct _trt__GetProfiles trt__GetProfiles;
    struct _trt__GetProfilesResponse profiles;

    struct _tds__GetServices getServices;
    soap_default__tds__GetServices(soap, &getServices);
    struct _tds__GetServicesResponse getServicesResponse;

    char *ptz_ep;
    char *media_ep = GetDeviceServices(soap, &getServices, &getServicesResponse);
    if (media_ep != NULL) {
        ptz_ep = GetPTZServier(&getServicesResponse);
        GetProfiles(soap, &trt__GetProfiles, &profiles, media_ep);
        GotoHomePos(soap, &profiles, ptz_ep);
    }
}

int main()
{
    struct soap *soap = create_soap_instance();
    if (soap == NULL)
        die("failed to create soap instance");

    worker(soap);

    soap_destroy(soap);
    soap_end(soap);
    soap_free(soap);

    return 0;
}

