#include "soap_instance.h"
#include "errors.h"
#include "log.h"

soap_t *g_soap;
services_t g_services;
profiles_t g_profiles;

void soap_instance_construct()
{
    g_soap = soap_new();
    if (g_soap == NULL)
        die(ERR_SOAP, "failed to create soap instance");

    g_soap->connect_timeout = g_soap->recv_timeout = g_soap->send_timeout = 5;
}

void soap_instance_destruct()
{
    soap_destroy(g_soap);
    soap_end(g_soap);
    soap_free(g_soap);
}

