#pragma once

struct ap_state
{
    int epoll_fd;
    int sock_fd;
    int current;
    int close_after_read;
};

void epoll_add_interface(struct ap_state *state, int fd);
void epoll_close_interface(struct ap_state *state, int fd);

