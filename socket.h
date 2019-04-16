#pragma once

#include "buffer.h"
#include "epoll.h"
#include <netdb.h>
#include <sys/types.h>

void socket_create_tcp(int *sock_fd, const char *port);
void socket_create_udp(int *sock_fd, int port);
int socket_accept(int sock_fd);
int socket_send_message_tcp(int fd, const void *message, ssize_t length);
int socket_send_message_udp(int fd, const buffer_t *message, struct sockaddr *addr);
int socket_send_message_udp_event(const struct event_t *event, const buffer_t *message);
void socket_handle_error(int sock_fd);

