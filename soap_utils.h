#pragma once

#include "log.h"
#include "errors.h"
#include "soap_global.h"
#include "config.h"

#define soap_utils_log_error(S) \
    log("soap error #%d: %s in %s/%s: %s", (S)->error, *soap_faultstring((S)), \
            *soap_faultcode((S)), *soap_faultsubcode((S)), *soap_faultdetail((S)));
#define soap_die(S, ...) do { soap_utils_log_error(S); die(ERR_SOAP, __VA_ARGS__); } while (0)

#define soap_utils_int_to_bool(X) ((X) ? xsd__boolean__true_ : xsd__boolean__false_ )

#define soap_utils_auth() \
    soap_utils_set_credentials(g_soap, g_config.username, g_config.password);

void soap_utils_set_credentials(soap_t *soap, const char *username, const char *pwd);
char* soap_utils_get_media_xaddr(services_t *services);
char* soap_utils_get_ptz_xaddr(services_t *services);
void soap_utils_get_services(soap_t *soap, const char *service_endpoint, services_t *services);
void soap_utils_get_profiles(soap_t *soap, const char *media_xaddr, profiles_t *profiles);
void soap_utils_get_device_information(soap_t *soap, const char *service_endpoint,
        device_info_t *device_info);
void soap_utils_print_device_info();
void soap_utils_get_snapshot_uri(soap_t *soap, const char *endpoint, char *profile_token,
        char **snapshot_uri);
void soap_utils_save_snapshot(const char *filename, const char *snapshot_uri);
void soap_utils_list_profiles();

