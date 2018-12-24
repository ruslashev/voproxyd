sources = main.c daemonize.c worker.c
cflags = -Wall -Wextra -g
ldflags =
binname = voproxyd

all:
	gcc $(sources) $(cflags) $(ldflags) -o $(binname)
	# ./$(binname)

