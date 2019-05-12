#include "soap_global.h"
#include "errors.h"
#include "log.h"

soap_t *g_soap;

void soap_global_construct()
{
    g_soap = soap_new();
    if (g_soap == NULL)
        die(ERR_SOAP, "failed to create soap instance");

    g_soap->connect_timeout = g_soap->recv_timeout = g_soap->send_timeout = 3;
}

void soap_global_destruct()
{
    soap_destroy(g_soap);
    soap_end(g_soap);
    soap_free(g_soap);
}

