#define _GNU_SOURCE

#include "epoll.h"
#include "errors.h"
#include "log.h"
#include "socket.h"
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

static int handle_message(struct ap_state *state, const uint8_t *message,
        ssize_t length, int *close)
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

    if (state->current == state->listen_sock_fd) {
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

    close(state->listen_sock_fd);
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

    create_listening_socket(&state.listen_sock_fd);

    epoll_add_interface(&state, state.listen_sock_fd);

    log("epoll fd = %d, listen socket fd = %d", state.epoll_fd,
            state.listen_sock_fd);

    return main_loop(&state);
}

