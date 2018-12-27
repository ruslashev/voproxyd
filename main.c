#include "errors.h"
#include "daemonize.h"
#include "log.h"
#include "worker.h"

#include <assert.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static void usage(const char *progname)
{
    assert(progname != NULL);
    printf("Usage %s [-h,--help]\n", progname);
}

static void parse_arguments(int argc, char *argv[])
{
    static struct option const long_options[] = {
        { "help",       no_argument,       NULL, 'h' },
        { 0,            0,                 0,    0   }
    };
    int opt = 0, option_index = 0;

    while ((opt = getopt_long(argc, argv, "h", long_options, &option_index)) != -1) {
        switch (opt) {
        case 'h':
            usage(argv[0]);
            exit(EXIT_SUCCESS);
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

int main(int argc, char *argv[])
{
    parse_arguments(argc, argv);

#ifdef DAEMONIZE
    openlog("voproxyd", LOG_NDELAY | LOG_PID, LOG_USER);

    daemonize();
#endif

    start_worker();
}

