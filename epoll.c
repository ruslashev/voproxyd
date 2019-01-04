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

static struct event_t* mk_event_t()
{
    struct event_t *event = calloc(1, sizeof(struct event_t));

    if (!event) {
        die(ERR_NOMEM, "failed to calloc(%zd)", sizeof(struct event_t));
    }

    return event;
}

static void ll_free_entitiy(struct tracking_ll_t *node)
{
    free(node->event);
    free(node);
}

void ll_free_list(struct tracking_ll_t **head)
{
    struct tracking_ll_t *it = *head;

    while (it) {
        it = it->next;
        ll_free_entitiy(*head);
        *head = it;
    }
}

static void ll_push_event(struct tracking_ll_t **head, struct event_t *event)
{
    struct tracking_ll_t *new_node = malloc(sizeof(struct tracking_ll_t));
    if (!new_node) {
        die(ERR_NOMEM, "failed to malloc(%zd)", sizeof(struct tracking_ll_t));
    }

    new_node->event = event;
    new_node->next = *head;
    *head = new_node;
}

void ll_delete_node(struct tracking_ll_t **head, struct tracking_ll_t *node)
{
    while (*head != node) {
        head = &(*head)->next;
    }

    if (*head == NULL) {
        log("ll_delete_node: can't find node");
        return;
    }

    *head = node->next;

    ll_free_entitiy(node);
}

void epoll_add_fd(struct ap_state *state, int fd, int in, int type)
{
    struct event_t *event = mk_event_t();
    struct epoll_event ep_event = { 0 };

    event->fd = fd;
    event->type = type;
    event->addr = NULL;

    ep_event.events = (unsigned)(in ? EPOLLIN : EPOLLOUT) | (unsigned)EPOLLRDHUP | EPOLLET;
    ep_event.data.ptr = event;

    log("add fd = %d to epoll set", fd);

    ll_push_event(&state->tracked_events, event);

    if (epoll_ctl(state->epoll_fd, EPOLL_CTL_ADD, event->fd, &ep_event) != -1) {
        return;
    }

    if (errno == EEXIST) {
        log("epoll_add_fd: fd fd = %d already exists", fd);
        return;
    }

    close(state->epoll_fd);

    die(ERR_EPOLL_CTL, "error adding fd = %d to epoll: %s", fd, strerror(errno));
}

void epoll_close_fd(struct ap_state *state, int fd)
{
    if (epoll_ctl(state->epoll_fd, EPOLL_CTL_DEL, fd, NULL) == -1) {
        close(state->epoll_fd);
        die(ERR_EPOLL_CTL, "error removing fd = %d from epoll: %s", fd, strerror(errno));
    }

    log("del fd = %d from epoll set", fd);

    close(fd);
}

void epoll_handle_event_errors(struct ap_state *state, const struct epoll_event *event)
{
    if (!(event->events & (unsigned)EPOLLERR)) {
        return;
    }

    if (event->events & (unsigned)EPOLLRDHUP) {
        return;
    }

    if (state->current_event->type == FDT_TCP_LISTEN) {
        socket_handle_error(state->current);
    }

    epoll_close_fd(state, state->current);

    die(ERR_UNSPECIFIED, "epoll error on fd = %d", state->current);
}

