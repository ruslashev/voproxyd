#pragma once

#include <sys/epoll.h>

struct ap_state
{
    int epoll_fd;
    int listen_sock_fd;
    int current;
    int close_after_read;
};

void epoll_add_interface(struct ap_state *state, int fd);
void epoll_close_interface(struct ap_state *state, int fd);
void epoll_handle_event_errors(struct ap_state *state,
        const struct epoll_event *event);

