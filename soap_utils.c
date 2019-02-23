#include "soap_utils.h"
#include "onvif_wsdd_stubs.h"
#include <wsseapi.h>
#include <nsmaps/wsdd.nsmap>

void soap_utils_set_credentials(soap_t *soap, const char *username, const char *pwd)
{
    soap_wsse_delete_Security(soap);

    if (soap_wsse_add_Timestamp(soap, "Time", 10))
        soap_die(soap, "soap_utils_set_credentials: failed to add timestamp");

    if (soap_wsse_add_UsernameTokenDigest(soap, "Auth", username, pwd))
        soap_die(soap, "soap_utils_set_credentials: failed to add username token digest");
}

static char* find_xaddr(services_t *services, const char *namespace)
{
    for (int i = 0; i < services->__sizeService; i++)
        if (strcmp(services->Service[i].Namespace, namespace) == 0)
            return services->Service[i].XAddr;

    die(ERR_SOAP, "failed to find namespace '%s'", namespace);
}

char* soap_utils_get_media_xaddr(services_t *services)
{
    return find_xaddr(services, SOAP_NAMESPACE_OF_trt);
}

char* soap_utils_get_ptz_xaddr(services_t *services)
{
    return find_xaddr(services, SOAP_NAMESPACE_OF_tptz);
}

void soap_utils_get_services(soap_t *soap, const char *endpoint, services_t *services)
{
    struct _tds__GetServices get_services_trt;

    soap_default__tds__GetServices(soap, &get_services_trt);

    get_services_trt.IncludeCapability = xsd__boolean__false_;

    if (soap_call___tds__GetServices(soap, endpoint, NULL, &get_services_trt, services) != SOAP_OK
            || services->Service == NULL)
        soap_die(soap, "failed to get services");
}

void soap_utils_get_profiles(soap_t *soap, const char *media_xaddr, profiles_t *profiles)
{
    struct _trt__GetProfiles get_profiles_trt;

    if (soap_call___trt__GetProfiles(soap, media_xaddr, NULL, &get_profiles_trt, profiles) != SOAP_OK
            || profiles->Profiles == NULL)
        soap_die(soap, "failed to get profiles");
}

void soap_utils_get_device_information(soap_t *soap, const char *endpoint, device_info_t *device_info)
{
    struct _tds__GetDeviceInformation get_device_info_tds;

    if (soap_call___tds__GetDeviceInformation(soap, endpoint, NULL, &get_device_info_tds,
                device_info) != SOAP_OK)
        soap_die(soap, "failed to get device information");
}

void soap_utils_print_device_info(soap_t *soap, const char *endpoint)
{
    device_info_t device_info;

    soap_utils_get_device_information(soap, endpoint, &device_info);

    log("Manufacturer:    %s", device_info.Manufacturer);
    log("Model:           %s", device_info.Model);
    log("FirmwareVersion: %s", device_info.FirmwareVersion);
    log("SerialNumber:    %s", device_info.SerialNumber);
    log("HardwareId:      %s", device_info.HardwareId);
}

void soap_utils_get_snapshot_uri(soap_t *soap, const char *endpoint, char *profile_token,
        char **snapshot_uri)
{
    struct _trt__GetSnapshotUri get_snapshot_uri_trt;
    struct _trt__GetSnapshotUriResponse snapshot_uri_response;

    get_snapshot_uri_trt.ProfileToken = profile_token;

    if (soap_call___trt__GetSnapshotUri(soap, endpoint, NULL, &get_snapshot_uri_trt,
                &snapshot_uri_response) != SOAP_OK || snapshot_uri_response.MediaUri == NULL)
        soap_die(soap, "failed to get snapshot URI");

    *snapshot_uri = snapshot_uri_response.MediaUri->Uri;
}

void soap_utils_save_snapshot(const char *filename, const char *snapshot_uri)
{
    FILE *fd;
    soap_t *soap;
    size_t imagelen;
    char *image;

    fd = fopen(filename, "wb");
    if (!fd)
        die(ERR_OPEN, "failed to open file '%s' for writing", filename);

    /* create a temporary context to retrieve the image with HTTP GET */
    soap = soap_new();
    if (soap == NULL)
        soap_die(soap, "failed to create temporary context");

    soap->connect_timeout = soap->recv_timeout = soap->send_timeout = 5;

    if (soap_GET(soap, snapshot_uri, NULL) || soap_begin_recv(soap))
        soap_die(soap, "error retrieving snapshot");

    image = soap_http_get_body(soap, &imagelen);

    soap_end_recv(soap);

    fwrite(image, 1, imagelen, fd);
    fclose(fd);

    soap_destroy(soap);
    soap_end(soap);
    soap_free(soap);
}

