#pragma once

#include "buffer.h"
#include <sys/epoll.h>

enum fd_type
{
    FDT_TCP_LISTEN = 0,
    FDT_TCP,
    FDT_UDP,
};

struct event_t
{
    int fd;
    int type;
    size_t buffer_rw_idx;
    buffer_t *buffer;
};

struct tracking_ll_t
{
    struct event_t *event;
    struct tracking_ll_t *next;
};

struct ap_state
{
    int epoll_fd;
    int close_after_read;
    int current;
    struct event_t *current_event;
    struct tracking_ll_t *tracked_events;
};

void epoll_add_fd(struct ap_state *state, int fd, int in, int type);
void epoll_close_fd(struct ap_state *state, int fd);
void epoll_handle_event_errors(struct ap_state *state, const struct epoll_event *event);
void ll_free_list(struct tracking_ll_t **head);

