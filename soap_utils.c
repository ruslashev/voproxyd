#include "soap_utils.h"
#include <wsseapi.h>

void soap_set_credentials(struct soap *soap, const char *username, const char *pwd)
{
    soap_wsse_delete_Security(soap);

    if (soap_wsse_add_Timestamp(soap, "Time", 10))
        soap_die(soap, "soap_set_credentials: failed to add timestamp");

    if (soap_wsse_add_UsernameTokenDigest(soap, "Auth", username, pwd))
        soap_die(soap, "soap_set_credentials: failed to add username token digest");
}

static char* find_xaddr(services_t *services, const char *namespace)
{
    for (int i = 0; i < services->__sizeService; i++)
        if (strcmp(services->Service[i].Namespace, namespace) == 0)
            return services->Service[i].XAddr;

    die(ERR_SOAP, "failed to find namespace '%s'", namespace);
}

char* soap_get_media_xaddr(services_t *services)
{
    return find_xaddr(services, SOAP_NAMESPACE_OF_trt);
}

char* soap_get_ptz_xaddr(services_t *services)
{
    return find_xaddr(services, SOAP_NAMESPACE_OF_tptz);
}

