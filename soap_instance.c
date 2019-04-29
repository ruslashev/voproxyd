#include "soap_instance.h"
#include "soap_utils.h"
#include "log.h"

#define MAX_URL_STRING_LEN 256

struct soap_instance* soap_instance_allocate(const char *ip, const char *port)
{
    struct soap_instance *instance = malloc(sizeof(struct soap_instance));
    if (!instance)
        die(ERR_NOMEM, "failed to allocate soap_instance");

    instance->services = malloc(sizeof(services_t));
    if (instance->services == NULL)
        die(ERR_ALLOC, "failed to allocate services");

    instance->profiles = malloc(sizeof(profiles_t));
    if (instance->profiles == NULL)
        die(ERR_ALLOC, "failed to allocate profiles");

    instance->service_endpoint = malloc(MAX_URL_STRING_LEN);
    if (instance->service_endpoint == NULL)
        die(ERR_ALLOC, "failed to allocate service endpoint string");

    soap_instance_set_endpoint(instance, ip, port);

    log("getting services for %s", instance->service_endpoint);
    soap_utils_get_services(g_soap, instance->service_endpoint, instance->services);
    soap_utils_get_profiles(g_soap, soap_utils_get_media_xaddr(instance->services), instance->profiles);

    instance->profile_idx = 0;

    instance->current_preset = 0;
    instance->preset_range_min = 1;
    instance->preset_range_max = 3;

    return instance;
}

void soap_instance_print_info(struct soap_instance *instance)
{
    log("instance addr = %s", instance->service_endpoint);

    soap_utils_print_device_info(g_soap, instance->service_endpoint);

    soap_utils_list_profiles(instance->profiles);
}

void soap_instance_set_endpoint(struct soap_instance* instance, const char *ip, const char *port)
{
    if (snprintf(instance->service_endpoint, MAX_URL_STRING_LEN, "http://%s:%s/onvif/device_service",
                ip, port) >= MAX_URL_STRING_LEN)
        die(ERR_WRITE, "serivce endpoint string overflow");
}

void soap_instance_deallocate(struct soap_instance *instance)
{
    free(instance->services);
    free(instance->profiles);
    free(instance->service_endpoint);
    free(instance);
}

