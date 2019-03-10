#include "soap_instance.h"

soap_t *g_soap;

void soap_instance_construct()
{
    g_soap = soap_new();
    if (g_soap == NULL)
        die(1, "failed to create soap instance");

    g_soap->connect_timeout = g_soap->recv_timeout = g_soap->send_timeout = 5;
}

void soap_instance_destruct()
{
    soap_destroy(soap);
    soap_end(soap);
    soap_free(soap);
}

