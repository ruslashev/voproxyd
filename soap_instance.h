#pragma once

#include "soap_header.h"

struct soap_instance
{
    char *service_endpoint;
    services_t services;
    profiles_t profiles;
};

struct soap_instance* soap_instance_allocate(const char *ip, const char *port);
void soap_instance_print_info(struct soap_instance *instance);
void soap_instance_deallocate(struct soap_instance *instance);

