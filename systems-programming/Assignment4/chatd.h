#include <ctype.h>
#include <errno.h>
#include <netinet/in.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <signal.h>

#define NAME_MAX 32
#define STATUS_MAX 64
#define MESSAGE_MAX 80
#define BODY_MAX 99999
#define ROOM_NAME "#all"

typedef enum { NAM, SET, MSG, WHO, ERR } MessageType;

// Notes on the following two:
/*
 * +1 is added to account for \0
 * state is meant to be from
 *  - 0, waiting for name
 *  - 1, logged in
 */
typedef struct {
  int client_fd;
  char name[NAME_MAX + 1];
  char status[STATUS_MAX + 1];
  int state;
} Client;

typedef struct {
  char sender[NAME_MAX + 1];
  char receiver[NAME_MAX + 1];
  char content[MESSAGE_MAX + 1];
  int length;
  int sender_length;
  int receiver_length;
  MessageType type;
} Message;

struct Node {
  Client client;
  struct Node *next;
};

