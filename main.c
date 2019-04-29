#include "config.h"
#include "daemonize.h"
#include "discovery.h"
#include "errors.h"
#include "log.h"
#include "worker.h"
#include "soap_global.h"
#include "soap_utils.h"
#include "address_manager.h"

#include <assert.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int g_daemonize = 0;
FILE *g_log_output_file;

static void usage(const char *progname)
{
    assert(progname != NULL);
    printf("Usage: %s [-h,--help] [-d,--daemonize]\n", progname);
}

static void parse_daemonize()
{
    if (!optarg) {
        g_daemonize = 1;
        return;
    }

    if (strcmp(optarg, "1") == 0) {
        g_daemonize = 1;
    } else if (strcmp(optarg, "0") == 0) {
        g_daemonize = 0;
    } else {
        fprintf(stderr, "Bad daemonization argument \"%s\". Accepted values are \"0\" or \"1\"\n",
                optarg);
        exit(ERR_INVALID_ARGS);
    }
}

static void parse_arguments(int argc, char *argv[])
{
    struct option const long_options[] = {
        { "daemonize",  optional_argument, NULL, 'd' },
        { "help",       no_argument,       NULL, 'h' },
        { "log",        required_argument, NULL, 'l' },
        { 0,            0,                 0,    0   }
    };
    int opt = 0, option_index = 0;

    while ((opt = getopt_long(argc, argv, "d::hl:", long_options, &option_index)) != -1) {
        switch (opt) {
        case 'd':
            parse_daemonize();
            break;
        case 'h':
            usage(argv[0]);
            exit(EXIT_SUCCESS);
            break;
        case 'l':
            g_log_output_file = fopen(optarg, "a");
            break;
        default:
            usage(argv[0]);
            exit(ERR_INVALID_ARGS);
        }
    }

    if (optind < argc) {
        fprintf(stderr, "Invalid option(s): ");

        while (optind < argc) {
            fprintf(stderr, "%s ", argv[optind++]);
        }

        fprintf(stderr, "\n");
        usage(argv[0]);
        exit(ERR_INVALID_ARGS);
    }
}

static void print_greeting()
{
    time_t t = time(NULL);
    struct tm *tl = localtime(&t);
    char greeting[512];

    strftime(greeting, sizeof(greeting), "voproxyd started on %F %T", tl);

    log("%s", greeting);
    log(" ");
}

int main(int argc, char *argv[])
{
    g_log_output_file = stdout;

    parse_arguments(argc, argv);

    if (g_daemonize) {
        openlog("voproxyd", LOG_NDELAY | LOG_PID, LOG_USER);

        daemonize();
    }

    print_greeting();

    worker_init();
    soap_global_construct();
    address_mngr_init();
    discovery_init();

    /* discovery_do(3000); */
    config_read();
    worker_start();

    config_destruct();
    soap_global_destruct();
    discovery_destruct();
    address_mngr_destruct();
}

