sources = avltree.c \
          bridge.c \
          bridge_commands.c \
          bridge_inquiries.c \
          buffer.c \
          daemonize.c \
          epoll.c \
          main.c \
          socket.c \
          visca.c \
          visca_commands.c \
          visca_inquiries.c \
          worker.c
cflags = -Wall -Wextra -g -Wno-unused-function -Wno-unused-variable -Wno-unused-parameter \
         -Wno-unused-but-set-variable
ldflags =
binname = voproxyd

all:
	@gcc $(sources) $(cflags) $(ldflags) -o $(binname)
	@echo "gcc $(binname)"
	./$(binname)

