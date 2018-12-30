#define _GNU_SOURCE

#include "buffer.h"
#include "epoll.h"
#include "errors.h"
#include "log.h"
#include "socket.h"
#include "tempconfig.h"
#include "visca.h"
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

static int handle_tcp(struct ap_state *state, const uint8_t *message, ssize_t length, int *close)
{
    (void)state;
    (void)message;

    log("parse tcp msg of len %zu\n", length);

    return 0;
}

static int handle_udp(const struct ap_state *state, uint8_t *message_bytes, ssize_t length,
        struct sockaddr *addr)
{
    buffer_t *message = cons_buffer(length), *response;

    message->data = message_bytes;

    response = visca_handle_message(message);

    if (response->length != 0) {
        log("debug visca_handle_message:");
        print_buffer(response, 16);

        send_message_udp(state->current, response, addr);

        free(response);
    }

    return 0;
}

static int epoll_handle_read_queue_tcp(struct ap_state *state)
{
    ssize_t message_length;
    uint8_t rx_message[VOPROXYD_MAX_RX_MESSAGE_LENGTH];
    int close = 0;

    log("about to read on fd = %d", state->current);

    message_length = read(state->current, rx_message, sizeof(rx_message));

    log("read on fd = %d message_length = %zd %s", state->current, message_length,
            (errno == EAGAIN || errno == EWOULDBLOCK) ? "(eagain | ewouldblock)" : "");

    if (message_length == 0) {
        log("close connection on socket fd = %d", state->current);
        epoll_close_interface(state, state->current);
        return 0;
    }

    if (message_length != -1) {
        handle_tcp(state, rx_message, message_length, &close);

        if (close || state->close_after_read) {
            state->close_after_read = 0;
            epoll_close_interface(state, state->current);
            return 0;
        }

        return 1;
    }

    if (errno == EAGAIN || errno == EWOULDBLOCK) {
        return 0;
    }

    epoll_close_interface(state, state->current);
    die(ERR_READ, "error reading on socket fd = %d: %s", state->current, strerror(errno));
}

static int epoll_handle_read_queue_udp(struct ap_state *state)
{
    ssize_t message_length;
    uint8_t rx_message[VOPROXYD_MAX_RX_MESSAGE_LENGTH];
    struct sockaddr_in addr;
    socklen_t addr_len = sizeof(addr);
    int close = 0;

    message_length = recvfrom(state->current, rx_message, sizeof(rx_message), MSG_DONTWAIT,
            (struct sockaddr*)&addr, &addr_len);

    log("recvfrom fd = %d message_length = %zd %s", state->current, message_length,
            (errno == EAGAIN || errno == EWOULDBLOCK) ? "(eagain | ewouldblock)" : "");

    if (message_length == 0) {
        log("close connection on socket fd = %d", state->current);
        epoll_close_interface(state, state->current);
        return 0;
    }

    if (message_length == -1) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            errno = 0;
            return 0;
        }

        epoll_close_interface(state, state->current);
        die(ERR_READ, "error reading on socket fd = %d: %s", state->current, strerror(errno));
    }

    handle_udp(state, rx_message, message_length, (struct sockaddr*)&addr);

    if (close || state->close_after_read) {
        state->close_after_read = 0;
        epoll_close_interface(state, state->current);
        return 0;
    }

    return 1;
}

static void epoll_handle_event(struct ap_state *state, const struct epoll_event *event)
{
    int continue_reading = 1, client_fd;

    log("new event on fd = %d", state->current);

    epoll_handle_event_errors(state, event);

    if ((event->events & (unsigned)EPOLLHUP)) {
        log("hangup on fd = %d", state->current);
    }

    state->close_after_read = !!((event->events & (unsigned)EPOLLHUP) |
            (event->events & (unsigned)EPOLLRDHUP));

    if (state->current == state->tcp_sock_fd) {
        client_fd = accept_on_socket(state->current);
        epoll_add_interface(state, client_fd);
        return;
    }

    while (continue_reading) {
        continue_reading = epoll_handle_read_queue_udp(state);
    }
}

static void main_loop(struct ap_state *state)
{
    struct epoll_event events[VOPROXYD_MAX_EPOLL_EVENTS];
    int num_events, ev_idx, running = 1;

    while (running) {
        num_events = epoll_wait(state->epoll_fd, events,
                VOPROXYD_MAX_EPOLL_EVENTS, -1);

        if (num_events == -1 && errno != EINTR) {
            die(ERR_EPOLL_WAIT, "epoll_wait() failed: %s", strerror(errno));
        }

        for (ev_idx = 0; ev_idx < num_events; ++ev_idx) {
            state->close_after_read = 0;
            state->current = events[ev_idx].data.fd;
            epoll_handle_event(state, &events[ev_idx]);
        }
    }

    close(state->udp_sock_fd);
    close(state->tcp_sock_fd);
    close(state->epoll_fd);
}

void start_worker(void)
{
    struct ap_state state = { 0 };

    state.epoll_fd = epoll_create1(0);
    if (state.epoll_fd == -1) {
        die(ERR_EPOLL_CREATE, "epoll_create1() failed: %s", strerror(errno));
    }

    /* create_listening_tcp_socket(&state.tcp_sock_fd); */
    /* epoll_add_interface(&state, state.tcp_sock_fd); */
    state.tcp_sock_fd = -1;

    create_udp_socket(&state.udp_sock_fd);
    epoll_add_interface(&state, state.udp_sock_fd);

    log("epoll fd = %d, tcp fd = %d udp fd = %d", state.epoll_fd,
            state.tcp_sock_fd, state.udp_sock_fd);

    main_loop(&state);
}

