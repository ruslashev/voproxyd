#pragma once

#include "soap_utils.h"

void soap_ptz_continuous_move(soap_t *soap, char *ptz_xaddr, char *profile_token);
void soap_ptz_goto_home(soap_t *soap, char *ptz_xaddr, char *profile_token);

