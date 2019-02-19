#pragma once

#include <soapH.h>
#include <soapStub.h>
#include "log.h"
#include "errors.h"

typedef struct soap soap_t;
typedef struct _tds__GetServicesResponse services_t;
typedef struct _trt__GetProfilesResponse profiles_t;

#define soap_utils_log_error(S) \
    log("soap error: error=%d faultstring='%s' faultcode='%s' faultsubcode='%s' faultdetail='%s'", \
            (S)->error, *soap_faultstring((S)), *soap_faultcode((S)), *soap_faultsubcode((S)), \
            *soap_faultdetail((S)));
#define soap_die(S, ...) do { soap_utils_log_error(S); die(ERR_SOAP, __VA_ARGS__); } while (0)

void soap_utils_set_credentials(soap_t *soap, const char *username, const char *pwd);
char* soap_utils_get_media_xaddr(services_t *services);
char* soap_utils_get_ptz_xaddr(services_t *services);
void soap_utils_get_services(soap_t *soap, const char *service_endpoint, services_t *services);
void soap_utils_get_profiles(soap_t *soap, const char *media_xaddr, profiles_t *profiles);

