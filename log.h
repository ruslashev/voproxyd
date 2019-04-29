#pragma once

#include "errors.h"

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <syslog.h>
#include <math.h>

extern int g_daemonize;
extern FILE *g_log_output_file;

#define _log_syslog(...) syslog(LOG_NOTICE, __VA_ARGS__)
#define _log_stream(...) \
    do { \
        fprintf(g_log_output_file, __VA_ARGS__); \
        fprintf(g_log_output_file, "\n"); \
        fflush(g_log_output_file); \
    } while (0)

#define log(...) \
    do { \
        if (g_daemonize && g_log_output_file == stdout) \
            _log_syslog(__VA_ARGS__); \
        else \
            _log_stream(__VA_ARGS__); \
    } while (0)

#define die_detail(X, ...) do { log(__VA_ARGS__); exit(X); } while (0)

#define die(X, ...) die_detail(X, __VA_ARGS__)

