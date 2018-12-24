#define _GNU_SOURCE

#include "errors.h"
#include "log.h"
#include "tempconfig.h"
#include "worker.h"

#include <errno.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <unistd.h>

#define VOPROXYD_STRING_BUFFERS_EXTEND_LENGTH 4096
#define VOPROXYD_MAX_EPOLL_EVENTS 128
#define VOPROXYD_MAX_RX_MESSAGE_LENGTH 4096

struct ap_state
{
    int epoll_fd;
    int sock_fd;
    int current;
    int close_after_read;
};

static void create_listening_socket(int *sock_fd)
{
    struct addrinfo ai_hint = { 0 }, *ai_result;
    int err;

    ai_hint.ai_family = AF_INET;
    ai_hint.ai_socktype = SOCK_STREAM;
    ai_hint.ai_flags = AI_PASSIVE;

    err = getaddrinfo(NULL, VOPROXYD_PORT, &ai_hint, &ai_result);
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

static int accept_on_socket(int sock_fd)
{
    int client_fd, err;
    struct sockaddr addr;
    socklen_t addr_len = sizeof(struct sockaddr);
    char host_str[NI_MAXHOST], serv_str[NI_MAXSERV];

    client_fd = accept4(sock_fd, &addr, &addr_len, (unsigned)SOCK_NONBLOCK);
    if (client_fd == -1) {
        die(ERR_ACCEPT, "accept4() failed: %s", strerror(errno));
    }

    err = getnameinfo(&addr, addr_len, host_str, NI_MAXHOST, serv_str,
            NI_MAXSERV, (unsigned)NI_NUMERICHOST | (unsigned)NI_NUMERICSERV);

    if (err == -1) {
        log("accept() on socket fd = %d -> fd = %d from unknown source: %s",
                sock_fd, client_fd, gai_strerror(err));
    } else {
        log("accept() on socket fd = %d -> fd = %d from %s:%s",
                sock_fd, client_fd, host_str, serv_str);
    }

    return client_fd;
}

static int send_message(int fd, const char *message, ssize_t length)
{
    ssize_t total_sent = 0, remaining = length, sent;

    while (total_sent < length) {
        sent = send(fd, message + total_sent, remaining, MSG_NOSIGNAL);
        if (sent == -1) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                continue;
            }

            log("failed to send message of length %zd to fd = %d: %s", length,
                    fd, strerror(errno));
            return 0;
        }

        total_sent += sent;
        remaining -= sent;
    }

    return total_sent == length;
}

static void epoll_add_interface(struct ap_state *state, int fd)
{
    struct epoll_event ep_event = { 0 };
    ep_event.events = (unsigned)EPOLLIN | (unsigned)EPOLLRDHUP | EPOLLET;
    ep_event.data.fd = fd;

    log("new fd = %d", fd);

    if (epoll_ctl(state->epoll_fd, EPOLL_CTL_ADD, fd, &ep_event) != -1) {
        return;
    }

    if (errno == EEXIST) {
        log("epoll_add_interface: interface fd = %d already exists", fd);
        return;
    }

    close(state->epoll_fd);

    die(ERR_EPOLL_CTL, "error adding interface (fd = %d) to epoll: %s", fd,
            strerror(errno));
}

static void epoll_close_interface(struct ap_state *state, int fd)
{
    if (epoll_ctl(state->epoll_fd, EPOLL_CTL_DEL, fd, NULL) == -1) {
        close(state->epoll_fd);
        die(ERR_EPOLL_CTL, "error removing interface (fd = %d) from epoll: %s",
                fd, strerror(errno));
    }

    log("del fd = %d", fd);

    close(fd);
}

static int handle_message(struct ap_state *state, const uint8_t *message, ssize_t length, int *close)
{
    (void)state;
    (void)message;
    (void)length;
    *close = 0;

    printf("recv msg of len %d\n", length);

    return 0;
}

static int epoll_handle_read_queue(struct ap_state *state, int *continue_reading)
{
    ssize_t message_length;
    uint8_t rx_message[VOPROXYD_MAX_RX_MESSAGE_LENGTH];
    int err, close = 0;

    *continue_reading = 0;

    message_length = read(state->current, rx_message, sizeof(rx_message));

    if (message_length == 0) {
        log("close connection on socket fd = %d", state->current);
        epoll_close_interface(state, state->current);
        return 0;
    }

    if (message_length != -1) {
        if ((err = handle_message(state, rx_message, message_length, &close))) {
            return err;
        }

        if (close || state->close_after_read) {
            state->close_after_read = 0;
            epoll_close_interface(state, state->current);
            return 0;
        }

        *continue_reading = 1;
        return 0;
    }

    if (errno == EAGAIN || errno == EWOULDBLOCK) {
        return 0;
    }

    log("error on socket fd = %d: %s", state->current, strerror(errno));
    epoll_close_interface(state, state->current);
    return 0;
}

static void epoll_handle_event_errors(struct ap_state *state, const struct epoll_event *event)
{
    int err;
    socklen_t err_len = sizeof(err);

    if (!(event->events & (unsigned)EPOLLERR)) {
        return;
    }

    if (event->events & (unsigned)EPOLLRDHUP) {
        return;
    }

    if (state->current == state->sock_fd) {
        if (getsockopt(state->current, SOL_SOCKET, SO_ERROR, &err, &err_len) == 0
                && SO_ERROR != 0) {
            die(ERR_EPOLL_EVENT, "error on socket: %d", err);
        } else {
            die(ERR_GETSOCKOPT, "failed to get error on socket: %s", strerror(errno));
        }
    }

    epoll_close_interface(state, state->current);

    die(ERR_UNSPECIFIED, "epoll error on fd = %d", state->current);
}

static int epoll_handle_event(struct ap_state *state, const struct epoll_event *event, int *running)
{
    int err, continue_reading = 1, client_fd;

    *running = 1;

    epoll_handle_event_errors(state, event);

    if ((event->events & (unsigned)EPOLLHUP) && !(event->events & (unsigned)EPOLLIN)) {
        log("hangup on fd = %d", state->current);
    }

    state->close_after_read = !!((event->events & (unsigned)EPOLLHUP)
            | (event->events & (unsigned)EPOLLRDHUP));

    if (state->current == state->sock_fd) {
        client_fd = accept_on_socket(state->current);
        epoll_add_interface(state, client_fd);
        return 0;
    }

    while (continue_reading) {
        err = epoll_handle_read_queue(state, &continue_reading);
        if (err) {
            *running = 0;
            return err;
        }
    }

    return 0;
}

static int main_loop(struct ap_state *state)
{
    struct epoll_event events[VOPROXYD_MAX_EPOLL_EVENTS];
    int num_events, ev_idx, running = 1, err = 0;

    while (running) {
        num_events = epoll_wait(state->epoll_fd, events,
                VOPROXYD_MAX_EPOLL_EVENTS, -1);

        if (num_events == -1 && errno != EINTR) {
            die(ERR_EPOLL_WAIT, "epoll_wait() failed: %s", strerror(errno));
        }

        for (ev_idx = 0; ev_idx < num_events && running && !err; ++ev_idx) {
            state->close_after_read = 0;
            state->current = events[ev_idx].data.fd;
            err = epoll_handle_event(state, &events[ev_idx], &running);
        }

        if (err) {
            break;
        }
    }

    close(state->sock_fd);
    close(state->epoll_fd);

    return err;
}

int start_worker(void)
{
    struct ap_state state = { 0 };

    state.epoll_fd = epoll_create1(0);
    if (state.epoll_fd == -1) {
        die(ERR_EPOLL_CREATE, "epoll_create1() failed: %s", strerror(errno));
    }

    create_listening_socket(&state.sock_fd);

    epoll_add_interface(&state, state.sock_fd);

    log("epoll fd = %d, socket fd = %d", state.epoll_fd, state.sock_fd);

    return main_loop(&state);
}

