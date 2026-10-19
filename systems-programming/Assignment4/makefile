CC = gcc
CFLAGS = -Wall -Wextra -g
.PHONY: all clean

all: chatd

chatd: chatd.c chatd.h
	$(CC) $(CFLAGS) -o chatd chatd.c

clean:
	rm -f chatd *o