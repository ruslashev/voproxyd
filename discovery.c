#include "discovery.h"
#include "soap_header.h"
#include "soap_utils.h"
#include <wsddapi.h>

#if 1
static struct soap *soap_listen;

void discovery_init()
{
    soap_listen = soap_new1(SOAP_IO_UDP);

    soap_listen->connect_flags |= SO_BROADCAST;

    if (!soap_valid_socket(soap_bind(soap_listen, NULL, 0, 1000))) {
        soap_print_fault(soap_listen, stderr);
        soap_die(soap_listen, "failed to bind wsdd listening soap instance");
    }

    struct ip_mreq mcast_group;
    memset(&mcast_group, 0, sizeof(mcast_group));
    mcast_group.imr_multiaddr.s_addr = inet_addr("239.255.255.250");
    mcast_group.imr_interface.s_addr = INADDR_ANY;
    setsockopt(soap_listen->master, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char*)&mcast_group, sizeof(mcast_group));
}

void discovery_do(int milliseconds)
{
    const char *multicast_url = "soap.udp://239.255.255.250:3702";
    const char *type = "", *scope = "onvif://www.onvif.org/";

    log("starting discovery..");

    if (soap_wsdd_Probe(soap_listen, SOAP_WSDD_ADHOC, SOAP_WSDD_TO_TS, multicast_url,
                soap_wsa_rand_uuid(soap_listen), NULL, type, scope, "") != SOAP_OK) {
        soap_print_fault(soap_listen, stderr);
        soap_die(soap_listen, "failed to wsdd probe");
    }

    soap_wsdd_listen(soap_listen, -milliseconds * 1000);

    log("discovery stop");
    log(" ");
}

void discovery_destruct()
{
    soap_destroy(soap_listen);
    soap_end(soap_listen);
    soap_free(soap_listen);
}
#else
static struct soap *invoker;
static struct soap *serv;

void discovery_init()
{
    invoker = soap_new(); /* = soap_new1(SOAP_IO_UDP); */
    serv = soap_new(); /* = soap_new1(SOAP_IO_UDP); */

    /* invoker->connect_flags |= SO_BROADCAST; */
    /* serv->connect_flags |= SO_BROADCAST; */

    if (!soap_valid_socket(soap_bind(serv, NULL, 0, 1000))) {
        soap_print_fault(serv, stderr);
        soap_die(serv, "failed to bind wsdd listening soap instance");
    }

    /* struct ip_mreq mcast_group; */
    /* memset(&mcast_group, 0, sizeof(mcast_group)); */
    /* mcast_group.imr_multiaddr.s_addr = inet_addr("239.255.255.250"); */
    /* mcast_group.imr_interface.s_addr = INADDR_ANY; */
    /* setsockopt(serv->master, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char*)&mcast_group, sizeof(mcast_group)); */
}

void discovery_do(int milliseconds)
{
    const char *multicast_url = "soap.udp://239.255.255.250:3702";
    const char *type = "", *scope = "onvif://www.onvif.org/";

    log("starting discovery..");

    if (soap_wsdd_Probe(invoker, SOAP_WSDD_ADHOC, SOAP_WSDD_TO_TS, multicast_url,
                soap_wsa_rand_uuid(invoker), NULL, type, scope, "") != SOAP_OK) {
        soap_print_fault(invoker, stderr);
        soap_die(invoker, "failed to wsdd probe");
    }

    soap_wsdd_listen(serv, -milliseconds * 1000);

    log("discovery stop");
    log(" ");
}

void discovery_destruct()
{
    soap_destroy(invoker);
    soap_end(invoker);
    soap_free(invoker);

    soap_destroy(serv);
    soap_end(serv);
    soap_free(serv);
}
#endif

