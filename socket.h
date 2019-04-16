#pragma once

#include "buffer.h"
#include "epoll.h"
#include <netdb.h>
#include <sys/types.h>

int socket_create_tcp(const char *port);
int socket_create_udp(int port);
int socket_accept(int sock_fd);
int socket_send_message_tcp(int fd, const void *message, ssize_t length);
int socket_send_message_udp(int fd, const buffer_t *message, struct sockaddr *addr);
int socket_send_message_udp_event(const struct event_t *event, const buffer_t *message);
void socket_handle_error(int sock_fd);

