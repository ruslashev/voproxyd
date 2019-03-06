#define _GNU_SOURCE

#include "buffer.h"
#include "config.h"
#include "epoll.h"
#include "errors.h"
#include "log.h"
#include "socket.h"
#include "tempconfig.h"
#include "visca.h"
#include "worker.h"
#include <errno.h>
#include <limits.h>
#include <netdb.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/inotify.h>
#include <sys/signalfd.h>
#include <unistd.h>

#define VOPROXYD_STRING_BUFFERS_EXTEND_LENGTH 4096
#define VOPROXYD_MAX_EPOLL_EVENTS 128
#define VOPROXYD_MAX_RX_MESSAGE_LENGTH 4096

static int add_udp_socket(struct ap_state *state)
{
    int udp_sock_fd;

    socket_create_udp(&udp_sock_fd);

    epoll_add_fd(state, udp_sock_fd, FDT_UDP, 1);

    return udp_sock_fd;
}

static int add_signal_handler(struct ap_state *state)
{
    sigset_t mask;
    int signal_fd;

    sigemptyset(&mask);
    sigaddset(&mask, SIGINT);

    if (sigprocmask(SIG_BLOCK, &mask, NULL) == -1)
        die(ERR_SIGNALS, "sigprocmask() failed: %s", strerror(errno));

    signal_fd = signalfd(-1, &mask, SFD_NONBLOCK | SFD_CLOEXEC);
    if (signal_fd == -1)
        die(ERR_SIGNALS, "signalfd() failed: %s", strerror(errno));

    epoll_add_fd(state, signal_fd, FDT_SIGNAL, 1);

    return signal_fd;
}

static void watch_config_file(int inotify_fd)
{
    int watch_descriptor;
    char *config_file = config_get_config_filename();

    watch_descriptor = inotify_add_watch(inotify_fd, config_file, IN_ALL_EVENTS);
    if (watch_descriptor < 0)
        die(ERR_INOTIFY, "failed to watch file \"%s\": %s", config_file, strerror(errno));

    free(config_file);
}

static int add_inotify(struct ap_state *state)
{
    int inotify_fd;

    inotify_fd = inotify_init1(IN_NONBLOCK | IN_CLOEXEC);
    if (inotify_fd < 0)
        die(ERR_INOTIFY, "failed to init inotify instance: %s", strerror(errno));

    epoll_add_fd(state, inotify_fd, FDT_INOTIFY, 1);

    watch_config_file(inotify_fd);

    return inotify_fd;
}

static int handle_tcp_message(struct ap_state *state, const uint8_t *message, ssize_t length,
        int *close)
{
    (void)state;
    (void)message;
    (void)close;

    log("handle tcp msg of len %zu\n", length);

    return 0;
}

static int handle_udp_message(const struct ap_state *state, uint8_t *message, ssize_t length)
{
    buffer_t *message_buf = cons_buffer(length);

    message_buf->data = message;

    visca_handle_message(message_buf, state->current_event);

    return 0;
}

/*
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
        epoll_close_fd(state, state->current);
        return 0;
    }

    if (message_length != -1) {
        handle_tcp_message(state, rx_message, message_length, &close);

        if (close || state->close_after_read) {
            state->close_after_read = 0;
            epoll_close_fd(state, state->current);
            return 0;
        }

        return 1;
    }

    if (errno == EAGAIN || errno == EWOULDBLOCK) {
        errno = 0;
        return 0;
    }

    epoll_close_fd(state, state->current);
    die(ERR_READ, "error reading on socket fd = %d: %s", state->current, strerror(errno));
}
*/

static int epoll_handle_read_queue_udp(struct ap_state *state)
{
    ssize_t message_length;
    uint8_t rx_message[VOPROXYD_MAX_RX_MESSAGE_LENGTH];
    struct sockaddr_in addr;
    socklen_t addr_len = sizeof(addr);
    int close = 0;

    message_length = recvfrom(state->current, rx_message, sizeof(rx_message), MSG_DONTWAIT,
            (struct sockaddr*)&addr, &addr_len);

    state->current_event->addr = (struct sockaddr*)&addr;

    log("recvfrom fd = %d message_length = %zd %s", state->current, message_length,
            (errno == EAGAIN || errno == EWOULDBLOCK) ? "eagain" : "");

    if (message_length == 0) {
        log("close connection on socket fd = %d", state->current);
        epoll_close_fd(state, state->current);
        return 0;
    }

    if (message_length == -1) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            errno = 0;
            return 0;
        }

        epoll_close_fd(state, state->current);
        die(ERR_READ, "error reading on socket fd = %d: %s", state->current, strerror(errno));
    }

    handle_udp_message(state, rx_message, message_length);

    if (close || state->close_after_read) {
        state->close_after_read = 0;
        epoll_close_fd(state, state->current);
        return 0;
    }

    return 1;
}

static void epoll_handle_signal(int signal_fd, int *running)
{
    struct signalfd_siginfo signal_info;
    int signal;

    if (read(signal_fd, &signal_info, sizeof(signal_info)) != sizeof(signal_info))
        die(ERR_READ, "failed to read signal fd: %s", strerror(errno));

    signal = signal_info.ssi_signo;

    log("received signal %d (%s)", signal, strsignal(signal));

    *running = 0;
}

static void display_event_info(const struct inotify_event *i)
{
    log("wd = %d", i->wd);

    log("mask = ");
    if (i->mask & IN_ACCESS)        log("IN_ACCESS ");
    if (i->mask & IN_ATTRIB)        log("IN_ATTRIB ");
    if (i->mask & IN_CLOSE_NOWRITE) log("IN_CLOSE_NOWRITE ");
    if (i->mask & IN_CLOSE_WRITE)   log("IN_CLOSE_WRITE ");
    if (i->mask & IN_CREATE)        log("IN_CREATE ");
    if (i->mask & IN_DELETE)        log("IN_DELETE ");
    if (i->mask & IN_DELETE_SELF)   log("IN_DELETE_SELF ");
    if (i->mask & IN_IGNORED)       log("IN_IGNORED ");
    if (i->mask & IN_ISDIR)         log("IN_ISDIR ");
    if (i->mask & IN_MODIFY)        log("IN_MODIFY ");
    if (i->mask & IN_MOVE_SELF)     log("IN_MOVE_SELF ");
    if (i->mask & IN_MOVED_FROM)    log("IN_MOVED_FROM ");
    if (i->mask & IN_MOVED_TO)      log("IN_MOVED_TO ");
    if (i->mask & IN_OPEN)          log("IN_OPEN ");
    if (i->mask & IN_Q_OVERFLOW)    log("IN_Q_OVERFLOW ");
    if (i->mask & IN_UNMOUNT)       log("IN_UNMOUNT ");
}

static void epoll_handle_inotify(int inotify_fd)
{
    char read_buffer[10 * (sizeof(struct inotify_event) + NAME_MAX + 1)]
        __attribute__ ((aligned(__alignof__(struct inotify_event))));
    ssize_t len;
    const struct inotify_event *event;

    len = read(inotify_fd, read_buffer, sizeof(read_buffer));
    if (len == -1 && errno != EAGAIN)
        die(ERR_READ, "failed to read from inotify fd: %s", strerror(errno));

    if (len <= 0)
        return;

    for (char *ptr = read_buffer; ptr < read_buffer + len;
            ptr += sizeof(struct inotify_event) + event->len) {
        event = (const struct inotify_event*)ptr;

        display_event_info(event);

        watch_config_file(inotify_fd);
    }
}

static void epoll_handle_event(struct ap_state *state, const struct epoll_event *event, int *running)
{
    int continue_reading = 1, client_fd;

    log(" ");
    log("new event on fd = %d", state->current);

    epoll_handle_event_errors(state, event);

    if ((event->events & (unsigned)EPOLLHUP)) {
        log("hangup on fd = %d", state->current);
    }

    state->close_after_read = !!((event->events & (unsigned)EPOLLHUP) |
            (event->events & (unsigned)EPOLLRDHUP));

    switch (state->current_event->type) {
        case FDT_TCP_LISTEN:
            client_fd = socket_accept(state->current);
            epoll_add_fd(state, client_fd, FDT_TCP, 1);
            break;
        case FDT_UDP:
            while (continue_reading) {
                continue_reading = epoll_handle_read_queue_udp(state);
            }
            break;
        case FDT_SIGNAL:
            epoll_handle_signal(state->current, running);
            break;
        case FDT_INOTIFY:
            epoll_handle_inotify(state->current);
            break;
        default:
            die(ERR_EPOLL_EVENT, "epoll_handle_event: unknown event type %d",
                    state->current_event->type);
    }
}

static void main_loop(struct ap_state *state)
{
    struct epoll_event events[VOPROXYD_MAX_EPOLL_EVENTS];
    int num_events, ev_idx, running = 1;

    while (running) {
        num_events = epoll_wait(state->epoll_fd, events, VOPROXYD_MAX_EPOLL_EVENTS, -1);

        if (num_events == -1 && errno != EINTR) {
            die(ERR_EPOLL_WAIT, "epoll_wait() failed: %s", strerror(errno));
        }

        for (ev_idx = 0; ev_idx < num_events; ++ev_idx) {
            state->close_after_read = 0;
            state->current_event = events[ev_idx].data.ptr;
            state->current = state->current_event->fd;
            epoll_handle_event(state, &events[ev_idx], &running);
        }
    }
}

void start_worker(void)
{
    struct ap_state state = { 0 };
    int udp_sock_fd, signal_fd, inotify_fd;

    state.epoll_fd = epoll_create1(0);
    if (state.epoll_fd == -1) {
        die(ERR_EPOLL_CREATE, "epoll_create1() failed: %s", strerror(errno));
    }

    udp_sock_fd = add_udp_socket(&state);

    signal_fd = add_signal_handler(&state);

    inotify_fd = add_inotify(&state);

    log("epoll fd = %d udp fd = %d sig fd = %d infy fd = %d", state.epoll_fd, udp_sock_fd,
            signal_fd, inotify_fd);

    main_loop(&state);

    ll_free_list(&state.tracked_events);

    close(inotify_fd);
    close(signal_fd);
    close(udp_sock_fd);
    close(state.epoll_fd);
}

