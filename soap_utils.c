#include "soap_utils.h"
#include <wsseapi.h>

void soap_utils_set_credentials(soap_t *soap, const char *username, const char *pwd)
{
    soap_wsse_delete_Security(soap);

    if (soap_wsse_add_Timestamp(soap, "Time", 10))
        soap_die(soap, "soap_utils_set_credentials: failed to add timestamp");

    if (soap_wsse_add_UsernameTokenDigest(soap, "Auth", username, pwd))
        soap_die(soap, "soap_utils_set_credentials: failed to add username token digest");
}

static char* find_xaddr(services_t *services, const char *namespace)
{
    for (int i = 0; i < services->__sizeService; i++)
        if (strcmp(services->Service[i].Namespace, namespace) == 0)
            return services->Service[i].XAddr;

    die(ERR_SOAP, "failed to find namespace '%s'", namespace);
}

char* soap_utils_get_media_xaddr(services_t *services)
{
    return find_xaddr(services, SOAP_NAMESPACE_OF_trt);
}

char* soap_utils_get_ptz_xaddr(services_t *services)
{
    return find_xaddr(services, SOAP_NAMESPACE_OF_tptz);
}

void soap_utils_get_services(soap_t *soap, const char *service_endpoint, services_t *services)
{
    struct _tds__GetServices get_services_trt;
    int result;

    soap_default__tds__GetServices(soap, &get_services_trt);

    get_services_trt.IncludeCapability = xsd__boolean__false_;

    result = soap_call___tds__GetServices(soap, service_endpoint, NULL, &get_services_trt, services);

    if (result != SOAP_OK || services->Service == NULL)
        soap_die(soap, "failed to get services");
}

void soap_utils_get_profiles(soap_t *soap, const char *media_xaddr, profiles_t *profiles)
{
    int result;
    struct _trt__GetProfiles get_profiles_trt;

    result = soap_call___trt__GetProfiles(soap, media_xaddr, NULL, &get_profiles_trt, profiles);

    if (result != SOAP_OK || profiles->Profiles == NULL)
        soap_die(soap, "failed to get profiles");
}

void soap_utils_get_device_information(soap_t *soap, const char *endpoint, device_info_t *device_info)
{
    struct _tds__GetDeviceInformation get_device_info_tds;

    if (soap_call___tds__GetDeviceInformation(soap, endpoint, NULL, &get_device_info_tds,
                device_info) != SOAP_OK)
        soap_die(soap, "failed to get device information");
}

