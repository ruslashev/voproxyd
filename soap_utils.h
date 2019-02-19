#pragma once

#include <soapH.h>
#include <soapStub.h>
#include "log.h"
#include "errors.h"

#define soap_log_error(S) \
    log("soap error: error=%d faultstring='%s' faultcode='%s' faultsubcode='%s' faultdetail='%s'", \
            (S)->error, *soap_faultstring((S)), *soap_faultcode((S)), *soap_faultsubcode((S)), \
            *soap_faultdetail((S)));
#define soap_die(S, ...) do { soap_log_error(S); die(ERR_SOAP, __VA_ARGS__); } while (0)

void soap_set_credentials(struct soap *soap, const char *username, const char *pwd);

