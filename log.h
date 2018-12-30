#pragma once

#include "errors.h"

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

#ifdef DAEMONIZE
#include <syslog.h>
#define log(...) syslog(LOG_NOTICE, __VA_ARGS__)
#else
#include <stdio.h>
#define log(...) do { printf(__VA_ARGS__); puts(""); } while (0)
#endif

#define die_detail(X, ...) do { log(__VA_ARGS__); exit(X); } while (0)

#define die(X, ...) die_detail(X, __VA_ARGS__)

