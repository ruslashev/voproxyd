#pragma once

#include "buffer.h"
#include <netdb.h>
#include <sys/epoll.h>

enum fd_type
{
    FDT_TCP_LISTEN = 0,
    FDT_TCP,
    FDT_UDP,
    FDT_SIGNAL,
    FDT_INOTIFY,
    FDT_PIPE,
};

struct event_t
{
    int fd;
    int type;
    struct sockaddr *addr;
    socklen_t addr_len;
    int child_pid;
    char *command_output;
    size_t command_output_len;
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

void epoll_add_fd(struct ap_state *state, int fd, int type, int in);
void epoll_close_fd(struct ap_state *state, int fd);
void epoll_handle_event_errors(struct ap_state *state, const struct epoll_event *event);
void ll_free_list(struct tracking_ll_t **head);
void ll_delete_node(struct tracking_ll_t **head, struct tracking_ll_t *node);

