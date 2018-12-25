#define _GNU_SOURCE

#include "errors.h"
#include "log.h"
#include "socket.h"
#include "tempconfig.h"

#include <errno.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void create_listening_tcp_socket(int *sock_fd)
{
    struct addrinfo ai_hint = { 0 }, *ai_result;
    int err;

    ai_hint.ai_family = AF_INET;
    ai_hint.ai_socktype = SOCK_STREAM;
    ai_hint.ai_flags = AI_PASSIVE;

    err = getaddrinfo(NULL, VOPROXYD_TCP_PORT, &ai_hint, &ai_result);
    if (err != 0) {
        die(ERR_GETADDRINFO, "gettaddrinfo() failed: %s", gai_strerror(err));
    }

    *sock_fd = socket(ai_result->ai_family,
            (unsigned)ai_result->ai_socktype | (unsigned)SOCK_NONBLOCK,
            ai_result->ai_protocol);
    if (*sock_fd == -1) {
        freeaddrinfo(ai_result);
        die(ERR_SOCKET, "socket() failed: %s", strerror(errno));
    }

    if (bind(*sock_fd, ai_result->ai_addr, ai_result->ai_addrlen) == -1) {
        freeaddrinfo(ai_result);
        close(*sock_fd);
        die(ERR_BIND, "failed to bind a socket: %s", strerror(errno));
    }

    freeaddrinfo(ai_result);

    if (listen(*sock_fd, SOMAXCONN) == -1) {
        die(ERR_LISTEN, "listen() failed: %s", strerror(errno));
    }
}

void create_udp_socket(int *sock_fd)
{
    struct sockaddr_in addr;

    *sock_fd = socket(AF_INET, (unsigned)SOCK_DGRAM | (unsigned)SOCK_NONBLOCK, 0);
    if (*sock_fd == -1) {
        die(ERR_SOCKET, "failed to create socket: %s", strerror(errno));
    }

    memset(&addr, 0, sizeof(addr));

    addr.sin_family = AF_INET;
    addr.sin_port = htons(VOPROXYD_UDP_PORT);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(*sock_fd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
        die(ERR_BIND, "failed to bind() socket: %s", strerror(errno));
    }
}

int accept_on_socket(int sock_fd)
{
    int client_fd, err;
    struct sockaddr addr;
    socklen_t addr_len = sizeof(struct sockaddr);
    char host_str[NI_MAXHOST], serv_str[NI_MAXSERV];

    client_fd = accept4(sock_fd, &addr, &addr_len, (unsigned)SOCK_NONBLOCK);
    if (client_fd == -1) {
        die(ERR_ACCEPT, "accept4() failed: %s", strerror(errno));
    }

    err = getnameinfo(&addr, addr_len, host_str, NI_MAXHOST, serv_str, NI_MAXSERV,
            (unsigned)NI_NUMERICHOST | (unsigned)NI_NUMERICSERV);

    if (err == -1) {
        log("accept() fd = %d from unknown source: %s", client_fd, gai_strerror(err));
    } else {
        log("accept() fd = %d from %s:%s", client_fd, host_str, serv_str);
    }

    return client_fd;
}

int send_message(int fd, const char *message, ssize_t length)
{
    ssize_t total_sent = 0, remaining = length, sent;

    while (total_sent < length) {
        sent = send(fd, message + total_sent, remaining, MSG_NOSIGNAL);
        if (sent == -1) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                continue;
            }

            log("failed to send message of length %zd to fd = %d: %s", length, fd, strerror(errno));
            return 0;
        }

        total_sent += sent;
        remaining -= sent;
    }

    return total_sent == length;
}

void handle_socket_error(int sock_fd)
{
    int err;
    socklen_t err_len = sizeof(err);

    if (getsockopt(sock_fd, SOL_SOCKET, SO_ERROR, &err, &err_len) == 0 && SO_ERROR != 0) {
        die(ERR_EPOLL_EVENT, "error on socket: %d", err);
    } else {
        die(ERR_GETSOCKOPT, "failed to get error on socket: %s", strerror(errno));
    }
}

