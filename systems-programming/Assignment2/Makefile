CC = gcc
CFLAGS = -Wall -Wextra -g

all: compare

compare: compare.c compare.h
	$(CC) $(CFLAGS) -o compare compare.c -lm

test: compareTest.c compare.c compare.h
	$(CC) $(CFLAGS) -DTESTING -o compareTest compareTest.c compare.c -lm
	./compareTest

asan: compare.c compare.h
	$(CC) $(CFLAGS) -fsanitize=address,undefined -o compare compare.c -lm

clean:
	rm -f compare compareTest
