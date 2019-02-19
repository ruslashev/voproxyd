#pragma once

enum {
    ERR_SUCCESS        = 0,
    ERR_UNSPECIFIED    = 1,
    ERR_INVALID_ARGS   = 2,
    ERR_FORK           = 3,
    ERR_SETSID         = 4,
    ERR_CHDIR_ROOT     = 5,
    ERR_CLOSE          = 6,
    ERR_OPEN_DEVNULL   = 7,
    ERR_DUP            = 8,
    ERR_GETADDRINFO    = 9,
    ERR_SOCKET         = 10,
    ERR_BIND           = 11,
    ERR_SIGNALFD       = 12,
    ERR_LISTEN         = 13,
    ERR_EPOLL_CREATE   = 14,
    ERR_EPOLL_CTL      = 15,
    ERR_ALLOC          = 16,
    ERR_EPOLL_WAIT     = 17,
    ERR_ACCEPT         = 18,
    ERR_FCNTL          = 19,
    ERR_PIPE           = 20,
    ERR_EXEC           = 21,
    ERR_WAITPID        = 22,
    ERR_READ           = 23,
    ERR_NOMEM          = 24,
    ERR_GETSOCKOPT     = 25,
    ERR_EPOLL_EVENT    = 26,
    ERR_VISCA_PROTOCOL = 27,
    ERR_SOAP           = 28
};

