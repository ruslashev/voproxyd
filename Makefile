sources = main.c daemonize.c worker.c socket.c epoll.c visca.c
cflags = -Wall -Wextra -g
ldflags =
binname = voproxyd

all:
	gcc $(sources) $(cflags) $(ldflags) -o $(binname)
	./$(binname)

