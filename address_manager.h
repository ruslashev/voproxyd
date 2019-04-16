#pragma once

#include "soap_instance.h"

void address_mngr_init();
void address_mngr_add_address();
struct soap_instance* address_mngr_get_soap_instance_from_fd(int fd);
void address_mngr_destruct();

