#pragma once

#include "soap_instance.h"

void address_mngr_init();
void address_mngr_add_address();
struct soap_instance* address_mngr_get_soap_instance_from_fd(int fd);
struct soap_instance* address_mngr_find_soap_instance_matching_ip(const char *ip);
void address_mngr_destruct();

