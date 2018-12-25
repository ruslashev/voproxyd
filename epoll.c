#define _GNU_SOURCE

#include "epoll.h"
#include "errors.h"
#include "log.h"
#include "socket.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <unistd.h>

void epoll_add_interface(struct ap_state *state, int fd)
{
    struct epoll_event ep_event = { 0 };
    ep_event.events = (unsigned)EPOLLIN | (unsigned)EPOLLRDHUP | EPOLLET;
    ep_event.data.fd = fd;

    log("add fd = %d to epoll set", fd);

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

void epoll_close_interface(struct ap_state *state, int fd)
{
    if (epoll_ctl(state->epoll_fd, EPOLL_CTL_DEL, fd, NULL) == -1) {
        close(state->epoll_fd);
        die(ERR_EPOLL_CTL, "error removing interface (fd = %d) from epoll: %s",
                fd, strerror(errno));
    }

    log("del fd = %d from epoll set", fd);

    close(fd);
}

void epoll_handle_event_errors(struct ap_state *state,
        const struct epoll_event *event)
{
    if (!(event->events & (unsigned)EPOLLERR)) {
        return;
    }

    if (event->events & (unsigned)EPOLLRDHUP) {
        return;
    }

    if (state->current == state->tcp_sock_fd) {
        handle_socket_error(state->current);
    }

    epoll_close_interface(state, state->current);

    die(ERR_UNSPECIFIED, "epoll error on fd = %d", state->current);
}

