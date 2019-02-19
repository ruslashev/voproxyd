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

