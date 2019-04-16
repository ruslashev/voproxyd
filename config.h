#pragma once

#define INI_INLINE_COMMENT_PREFIXES ";#"
#define INI_HANDLER_LINENO 1
#define INI_USE_STACK 0
#define INI_MAX_LINE 1024
#define INI_ALLOW_REALLOC 1

#include "deps/inih/ini.h"

struct config
{
    const char *username;
    const char *password; /* blame onvif for requiring to pass plaintext passwords */
};

extern struct config g_config;

char* config_get_config_filename();
void config_read();

