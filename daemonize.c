#define _GNU_SOURCE

#include "daemonize.h"
#include "errors.h"
#include "log.h"

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

int daemonize(void)
{
    pid_t pid;
    int proc_status;
    int fd;

    pid = fork();
    if (pid == -1) {
        log("Failed to fork process: %s", strerror(errno));
        return ERR_FORK;
    }

    if (pid > 0) {
        if (waitpid(pid, &proc_status, 0) == -1) {
            log("failure waiting for child: %s", strerror(errno));
            return ERR_WAITPID;
        }
        exit(EXIT_SUCCESS);
    }

    if (setsid() == -1) {
        log("setsid() failed: %s", strerror(errno));
        return ERR_SETSID;
    }

    pid = fork();
    if (pid == -1) {
        log("Failed to fork process: %s", strerror(errno));
        return ERR_FORK;
    }

    if (pid > 0) {
        _exit(EXIT_SUCCESS);
    }

    umask(0);

    if (chdir("/") == -1) {
        log("Failed to chdir(\"/\"): %s", strerror(errno));
        return ERR_CHDIR_ROOT;
    }

    fd = open("/dev/null", O_RDWR | O_CLOEXEC);
    if (fd == -1) {
        log("Failed to open(\"/dev/null\"): %s", strerror(errno));
        return ERR_OPEN_DEVNULL;
    }

    if (dup3(fd, STDIN_FILENO, O_CLOEXEC) != STDIN_FILENO) {
        log("Failed to dup3 stdout: %s", strerror(errno));
        return ERR_DUP;
    }

    if (dup3(fd, STDOUT_FILENO, O_CLOEXEC) != STDOUT_FILENO) {
        log("Failed to dup3 stdout: %s", strerror(errno));
        return ERR_DUP;
    }

    if (dup3(fd, STDERR_FILENO, O_CLOEXEC) != STDERR_FILENO) {
        log("Failed to dup3 stderr: %s", strerror(errno));
        return ERR_DUP;
    }

    return 0;
}

