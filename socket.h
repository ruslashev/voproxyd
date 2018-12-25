#pragma once

#include <sys/types.h>

void create_listening_socket(int *sock_fd);
int accept_on_socket(int sock_fd);
int send_message(int fd, const char *message, ssize_t length);

