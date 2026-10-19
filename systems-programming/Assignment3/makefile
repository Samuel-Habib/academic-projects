CC     = gcc
CFLAGS = -Wall -Wextra -g

SRCS = input.c token.c resolve.c execute.c
OBJS = $(SRCS:.c=.o)

MAIN_SRC = mysh.c
TEST_SRC = shellTest.c

mysh: $(MAIN_SRC:.c=.o) $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

test: $(TEST_SRC:.c=.o) $(OBJS)
	$(CC) $(CFLAGS) -o shellTest $^
	./shellTest

crash: crash.c
	$(CC) $(CFLAGS) -o $@ $<

%.o: %.c mysh.h
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f *.o mysh shellTest crash
