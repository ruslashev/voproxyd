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

void daemonize(void)
{
    pid_t pid;
    int proc_status;
    int fd;

    pid = fork();
    if (pid == -1) {
        die(ERR_FORK, "Failed to fork process: %s", strerror(errno));
    }

    if (pid > 0) {
        if (waitpid(pid, &proc_status, 0) == -1) {
            die(ERR_WAITPID, "failure waiting for child: %s", strerror(errno));
        }
        exit(EXIT_SUCCESS);
    }

    if (setsid() == -1) {
        die(ERR_SETSID, "setsid() failed: %s", strerror(errno));
    }

    pid = fork();
    if (pid == -1) {
        die(ERR_FORK, "Failed to fork process: %s", strerror(errno));
    }

    if (pid > 0) {
        _exit(EXIT_SUCCESS);
    }

    umask(0);

    if (chdir("/") == -1) {
        die(ERR_CHDIR_ROOT, "Failed to chdir(\"/\"): %s", strerror(errno));
    }

    fd = open("/dev/null", (unsigned)O_RDWR | (unsigned)O_CLOEXEC);
    if (fd == -1) {
        die(ERR_OPEN_DEVNULL, "Failed to open(\"/dev/null\"): %s", strerror(errno));
    }

    if (dup3(fd, STDIN_FILENO, O_CLOEXEC) != STDIN_FILENO) {
        die(ERR_DUP, "Failed to dup3 stdin: %s", strerror(errno));
    }

    if (dup3(fd, STDOUT_FILENO, O_CLOEXEC) != STDOUT_FILENO) {
        die(ERR_DUP, "Failed to dup3 stdout: %s", strerror(errno));
    }

    if (dup3(fd, STDERR_FILENO, O_CLOEXEC) != STDERR_FILENO) {
        die(ERR_DUP, "Failed to dup3 stderr: %s", strerror(errno));
    }
}

