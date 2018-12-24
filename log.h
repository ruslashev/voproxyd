#pragma once

#ifdef DAEMONIZE
#include <syslog.h>

#define log(...) syslog(LOG_NOTICE, __VA_ARGS__)
#else
#define log(...) do { printf(__VA_ARGS__); puts(""); } while (0)
#endif

