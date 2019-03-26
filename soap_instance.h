#pragma once

#include <soapH.h>

typedef struct soap soap_t;

extern soap_t *g_soap;

void soap_instance_construct();
void soap_instance_destruct();

