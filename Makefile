sources = main.c daemonize.c
cflags = -Wall -Wextra -g
ldflags =
binname = voproxyd

all:
	gcc $(sources) $(cflags) $(ldflags) -o $(binname)
	./$(binname)

