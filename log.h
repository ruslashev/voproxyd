#pragma once

#include "errors.h"

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <syslog.h>
#include <math.h>

extern int g_daemonize;
extern int g_log_output_fd;

#define _log_syslog(...) syslog(LOG_NOTICE, __VA_ARGS__)
#define _log_stdout(...) \
    do { \
        dprintf(g_log_output_fd, __VA_ARGS__); \
        dprintf(g_log_output_fd, "\n"); \
    } while (0)

#define log(...) \
    do { \
        if (g_daemonize) \
            _log_syslog(__VA_ARGS__); \
        else \
            _log_stdout(__VA_ARGS__); \
    } while (0)

#define die_detail(X, ...) do { log(__VA_ARGS__); exit(X); } while (0)

#define die(X, ...) die_detail(X, __VA_ARGS__)

