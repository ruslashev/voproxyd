#pragma once

#include <sys/types.h>

void create_listening_tcp_socket(int *sock_fd);
void create_udp_socket(int *sock_fd);
int accept_on_socket(int sock_fd);
int send_message(int fd, const void *message, ssize_t length);
void handle_socket_error(int sock_fd);

