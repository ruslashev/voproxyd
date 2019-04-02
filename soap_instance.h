#pragma once

#include <soapH.h>

typedef struct soap soap_t;
typedef struct _tds__GetServicesResponse services_t;
typedef struct _trt__GetProfilesResponse profiles_t;
typedef struct _tds__GetDeviceInformationResponse device_info_t;

extern soap_t *g_soap;
extern services_t g_services;
extern profiles_t g_profiles;

void soap_instance_construct();
void soap_instance_destruct();

