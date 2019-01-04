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

void socket_create_tcp(int *sock_fd)
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

void socket_create_udp(int *sock_fd)
{
    struct sockaddr_in addr = { 0 };

    *sock_fd = socket(AF_INET, (unsigned)SOCK_DGRAM | (unsigned)SOCK_NONBLOCK, 0);
    if (*sock_fd == -1) {
        die(ERR_SOCKET, "failed to create socket: %s", strerror(errno));
    }

    addr.sin_family = AF_INET;
    addr.sin_port = htons(VOPROXYD_UDP_PORT);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(*sock_fd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
        die(ERR_BIND, "failed to bind() socket: %s", strerror(errno));
    }
}

int socket_accept(int sock_fd)
{
    int client_fd, err;
    struct sockaddr addr;
    socklen_t addr_len = sizeof(struct sockaddr);
    char host_str[NI_MAXHOST], serv_str[NI_MAXSERV];

    client_fd = accept4(sock_fd, &addr, &addr_len, SOCK_NONBLOCK);
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

int socket_send_message_tcp(int fd, const void *message, ssize_t length)
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

int socket_send_message_udp(int fd, const buffer_t *message, struct sockaddr *addr)
{
    ssize_t total = message->length, total_sent = 0, remaining = total, sent;
    socklen_t addr_len = sizeof(struct sockaddr_in);

    while (total_sent < total) {
        sent = sendto(fd, message + total_sent, remaining, 0, addr, addr_len);
        if (sent == -1) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                continue;
            }

            log("failed to send message of length %zd to fd = %d: %s", total, fd, strerror(errno));
            return 0;
        }

        total_sent += sent;
        remaining -= sent;
    }

    return total_sent == total;
}

int socket_send_message_udp_event(const struct event_t *event, const buffer_t *message)
{
    return socket_send_message_udp(event->fd, message, event->addr);
}

void socket_handle_error(int sock_fd)
{
    int err;
    socklen_t err_len = sizeof(err);

    if (getsockopt(sock_fd, SOL_SOCKET, SO_ERROR, &err, &err_len) == 0 && SO_ERROR != 0) {
        die(ERR_EPOLL_EVENT, "error on socket: %d", err);
    } else {
        die(ERR_GETSOCKOPT, "failed to get error on socket: %s", strerror(errno));
    }
}

