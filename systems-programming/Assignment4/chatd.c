#include "chatd.h"

#define READ_CLOSED 0
#define READ_OK 1
#define READ_FATAL -1
#define READ_TOO_LONG -2

volatile sig_atomic_t running = 1;

void handle_sigint(int sig) {
  (void)sig;
  running = 0;
}

static const char *typeString(MessageType type) {
  switch (type) {
  case NAM:
    return "NAM";
  case SET:
    return "SET";
  case MSG:
    return "MSG";
  case WHO:
    return "WHO";
  case ERR:
    return "ERR";
  }
  return "ERR";
}

static int setType(char *code, MessageType *type) {
  if (strcmp(code, "NAM") == 0) {
    *type = NAM;
    return 1;
  }
  if (strcmp(code, "SET") == 0) {
    *type = SET;
    return 1;
  }
  if (strcmp(code, "MSG") == 0) {
    *type = MSG;
    return 1;
  }
  if (strcmp(code, "WHO") == 0) {
    *type = WHO;
    return 1;
  }
  if (strcmp(code, "ERR") == 0) {
    *type = ERR;
    return 1;
  }
  return 0;
}

static ssize_t readByte(int fd, char *ch) {
  ssize_t amount;

  while (1) {
    amount = read(fd, ch, 1);
    if (amount < 0 && errno == EINTR)
      continue;
    return amount;
  }
}

static int readField(int fd, char *buffer, size_t size) {
  size_t i = 0;
  char ch;

  if (size == 0)
    return READ_FATAL;

  while (1) {
    ssize_t amount = readByte(fd, &ch);
    if (amount <= 0) return (amount == 0) ? READ_CLOSED : READ_FATAL;
    if (ch != '\n' && ch != '\r' && ch != ' ') break; 
  }

  buffer[i++] = ch;

  while (1) {
    ssize_t amount = readByte(fd, &ch);
    if (amount == 0)
      return READ_CLOSED;
    if (amount < 0)
      return READ_FATAL;

    if (ch == '|') {
      buffer[i] = '\0';
      return READ_OK;
    }

    if (i + 1 >= size)
      return READ_FATAL;

    buffer[i] = ch;
    i++;
  }
}

static int readExact(int fd, char *buffer, int length) {
  int total = 0;

  while (total < length) {
    ssize_t amount = read(fd, buffer + total, length - total);
    if (amount == 0)
      return READ_CLOSED;
    if (amount < 0) {
      if (errno == EINTR)
        continue;
      return READ_FATAL;
    }
    total += amount;
  }

  buffer[length] = '\0';
  return READ_OK;
}

static int parseLength(char *field, int *length) {
  int value = 0;

  if (field[0] == '\0')
    return 0;

  for (int i = 0; field[i] != '\0'; i++) {
    if (!isdigit((unsigned char)field[i]))
      return 0;
    value = value * 10 + (field[i] - '0');
  }

  if (value < 1 || value > BODY_MAX)
    return 0;

  *length = value;
  return 1;
}

static void clearMessage(Message *message) {
  memset(message->sender, 0, NAME_MAX + 1);
  memset(message->receiver, 0, NAME_MAX + 1);
  memset(message->content, 0, MESSAGE_MAX + 1);
  message->length = 0;
  message->sender_length = 0;
  message->receiver_length = 0;
  message->type = ERR;
}

static int parseBody(Message *message, char *body, int body_length) {
  char *first;
  char *second;
  char *receiver;
  char *content;
  int usable_length;
  int receiver_length;
  int content_length;

  if (body_length < 1 || body[body_length - 1] != '|')
    return READ_FATAL;

  body[body_length - 1] = '\0';
  usable_length = body_length - 1;

  if (message->type == NAM || message->type == SET || message->type == WHO) {
    message->length = usable_length;

    if (message->type == NAM && usable_length > NAME_MAX)
      return READ_TOO_LONG;
    if (message->type == SET && usable_length > STATUS_MAX)
      return READ_TOO_LONG;
    if (message->type == WHO && usable_length > NAME_MAX &&
        strcmp(body, ROOM_NAME) != 0)
      return READ_TOO_LONG;

    memcpy(message->content, body, usable_length);
    message->content[usable_length] = '\0';
    return READ_OK;
  }

  if (message->type != MSG)
    return READ_FATAL;

  first = memchr(body, '|', usable_length);
  if (first == NULL)
    return READ_FATAL;

  *first = '\0';
  message->sender_length = first - body;

  receiver = first + 1;
  second = memchr(receiver, '|', usable_length - message->sender_length - 1);
  if (second == NULL)
    return READ_FATAL;

  *second = '\0';
  receiver_length = second - receiver;
  content = second + 1;
  content_length = usable_length - message->sender_length - receiver_length - 2;

  message->receiver_length = receiver_length;
  message->length = content_length;

  if (receiver_length > NAME_MAX && strcmp(receiver, ROOM_NAME) != 0)
    return READ_TOO_LONG;
  if (content_length > MESSAGE_MAX)
    return READ_TOO_LONG;

  if (message->sender_length <= NAME_MAX) {
    memcpy(message->sender, body, message->sender_length);
    message->sender[message->sender_length] = '\0';
  }

  memcpy(message->receiver, receiver, receiver_length);
  message->receiver[receiver_length] = '\0';
  memcpy(message->content, content, content_length);
  message->content[content_length] = '\0';

  return READ_OK;
}

static int readMessage(int fd, Message *message) {
  char version[8];
  char code[4];
  char length_field[6];
  char *body;
  int body_length;
  int result;

  clearMessage(message);

  result = readField(fd, version, sizeof(version));
  if (result != READ_OK)
    return result;

  result = readField(fd, code, sizeof(code));
  if (result != READ_OK)
    return result;

  result = readField(fd, length_field, sizeof(length_field));
  if (result != READ_OK)
    return result;

  if (strcmp(version, "1") != 0)
    return READ_FATAL;
  if (!setType(code, &message->type))
    return READ_FATAL;
  if (message->type == ERR)
    return READ_FATAL;
  if (!parseLength(length_field, &body_length))
    return READ_FATAL;

  body = malloc(body_length + 1);
  if (body == NULL)
    return READ_FATAL;

  result = readExact(fd, body, body_length);
  if (result == READ_OK)
    result = parseBody(message, body, body_length);

  free(body);
  return result;
}

static int writeAll(int fd, const char *buffer, size_t length) {
  size_t total = 0;

  while (total < length) {
    ssize_t amount = write(fd, buffer + total, length - total);
    if (amount < 0) {
      if (errno == EINTR)
        continue;
      return -1;
    }
    total += amount;
  }

  return 0;
}

static int sendPacket(int fd, MessageType type, const char *body) {
  char header[32];
  int body_length = strlen(body);
  int header_length;

  if (body_length > BODY_MAX)
    return -1;

  header_length = snprintf(header, sizeof(header), "1|%s|%d|", typeString(type),
                           body_length);
  if (header_length < 0 || header_length >= (int)sizeof(header))
    return -1;

  if (writeAll(fd, header, header_length) < 0)
    return -1;
  if (writeAll(fd, body, body_length) < 0)
    return -1;

  return 0;
}

static int sendError(int fd, int code, const char *explanation) {
  char body[128];
  int length;

  length = snprintf(body, sizeof(body), "%d|%s|", code, explanation);
  if (length < 0 || length >= (int)sizeof(body))
    return -1;

  return sendPacket(fd, ERR, body);
}

static int sendServerMessage(int fd, const char *sender, const char *receiver,
                             const char *content) {
  char *body;
  int body_length;
  int result;

  body_length = strlen(sender) + 1 + strlen(receiver) + 1 + strlen(content) + 1;
  if (body_length > BODY_MAX)
    return -1;

  body = malloc(body_length + 1);
  if (body == NULL)
    return -1;

  snprintf(body, body_length + 1, "%s|%s|%s|", sender, receiver, content);
  result = sendPacket(fd, MSG, body);
  free(body);
  return result;
}

static int legalName(char *name) {
  int length = strlen(name);

  if (length < 1 || length > NAME_MAX)
    return 0;

  for (int i = 0; i < length; i++) {
    unsigned char ch = (unsigned char)name[i];
    if (!isalnum(ch) && ch != '-' && ch != '_')
      return 0;
  }

  return 1;
}

static int legalStatus(char *status) {
  int length = strlen(status);

  if (length > STATUS_MAX)
    return 0;

  for (int i = 0; i < length; i++) {
    unsigned char ch = (unsigned char)status[i];
    if (ch < 32 || ch > 126)
      return 0;
  }

  return 1;
}

static int legalChatMessage(char *content) {
  int length = strlen(content);

  if (length < 1 || length > MESSAGE_MAX)
    return 0;

  for (int i = 0; i < length; i++) {
    unsigned char ch = (unsigned char)content[i];
    if (ch < 32 || ch > 126)
      return 0;
  }

  return 1;
}

static int findClient(char *name, Client client[], int nfds) {
  for (int i = 0; i < nfds - 1; i++) {
    if (client[i].state == 1 && strcmp(client[i].name, name) == 0)
      return i;
  }

  return -1;
}

static void removeClient(int index, struct pollfd fds[], Client client[],
                         int *nfds) {
  close(fds[index].fd);

  for (int i = index; i < *nfds - 1; i++)
    fds[i] = fds[i + 1];

  for (int i = index - 1; i < *nfds - 2; i++)
    client[i] = client[i + 1];

  (*nfds)--;
}

static void broadcast(Client client[], int nfds, const char *sender,
                      const char *content) {
  for (int i = 0; i < nfds - 1; i++) {
    if (client[i].state == 1)
      sendServerMessage(client[i].client_fd, sender, ROOM_NAME, content);
  }
}

static void handleName(Client *client, Client clients[], int nfds,
                       Message *message) {
  if (!legalName(message->content)) {
    sendError(client->client_fd, 3, "Illegal character");
    return;
  }

  if (findClient(message->content, clients, nfds) >= 0) {
    sendError(client->client_fd, 1, "Name in use");
    return;
  }

  strcpy(client->name, message->content);
  client->state = 1;
  sendServerMessage(client->client_fd, ROOM_NAME, client->name,
                    "Welcome to the chat!");
}

static void handleStatus(Client *client, Client clients[], int nfds,
                         Message *message) {
  char notice[NAME_MAX + STATUS_MAX + 16];

  if (!legalStatus(message->content)) {
    sendError(client->client_fd, 3, "Illegal character");
    return;
  }

  strcpy(client->status, message->content);

  if (client->status[0] != '\0') {
    snprintf(notice, sizeof(notice), "%s is now \"%s\"", client->name,
             client->status);
    broadcast(clients, nfds, ROOM_NAME, notice);
  }
}

static void handleChatMessage(Client *client, Client clients[], int nfds,
                              Message *message) {
  int recipient;

  if (strcmp(message->receiver, ROOM_NAME) != 0 &&
      !legalName(message->receiver)) {
    sendError(client->client_fd, 3, "Illegal character");
    return;
  }

  if (!legalChatMessage(message->content)) {
    sendError(client->client_fd, 3, "Illegal character");
    return;
  }

  if (strcmp(message->receiver, ROOM_NAME) == 0) {
    broadcast(clients, nfds, client->name, message->content);
    return;
  }

  recipient = findClient(message->receiver, clients, nfds);
  if (recipient < 0) {
    sendError(client->client_fd, 2, "Unknown recipient");
    return;
  }

  sendServerMessage(clients[recipient].client_fd, client->name,
                    message->receiver, message->content);
}

static int appendText(char *buffer, int size, int *used, const char *text) {
  int length = strlen(text);

  if (*used + length >= size)
    return 0;

  memcpy(buffer + *used, text, length);
  *used += length;
  buffer[*used] = '\0';
  return 1;
}

static void handleWho(Client *client, Client clients[], int nfds,
                      Message *message) {
  char response[BODY_MAX + 1];
  int used = 0;
  int target;

  response[0] = '\0';

  if (strcmp(message->content, ROOM_NAME) == 0) {
    for (int i = 0; i < nfds - 1; i++) {
      if (clients[i].state != 1)
        continue;

      if (used > 0)
        appendText(response, sizeof(response), &used, "\n");

      appendText(response, sizeof(response), &used, clients[i].name);
      if (clients[i].status[0] != '\0') {
        appendText(response, sizeof(response), &used, ": ");
        appendText(response, sizeof(response), &used, clients[i].status);
      }
    }

    sendServerMessage(client->client_fd, ROOM_NAME, client->name, response);
    return;
  }

  if (!legalName(message->content)) {
    sendError(client->client_fd, 3, "Illegal character");
    return;
  }

  target = findClient(message->content, clients, nfds);
  if (target < 0) {
    sendError(client->client_fd, 2, "Unknown recipient");
    return;
  }

  if (clients[target].status[0] == '\0') {
    sendServerMessage(client->client_fd, ROOM_NAME, client->name, "No status");
    return;
  }

  snprintf(response, sizeof(response), "%s: %s", clients[target].name,
           clients[target].status);
  sendServerMessage(client->client_fd, ROOM_NAME, client->name, response);
}

static int handleMessage(Client *client, Client clients[], int nfds,
                         Message *message) {
  if (client->state == 0 && message->type != NAM) {
    sendError(client->client_fd, 0, "Unreadable");
    return -1;
  }

  if (client->state == 1 && message->type == NAM) {
    sendError(client->client_fd, 0, "Unreadable");
    return -1;
  }

  switch (message->type) {
  case NAM:
    handleName(client, clients, nfds, message);
    break;
  case SET:
    handleStatus(client, clients, nfds, message);
    break;
  case MSG:
    handleChatMessage(client, clients, nfds, message);
    break;
  case WHO:
    handleWho(client, clients, nfds, message);
    break;
  case ERR:
    sendError(client->client_fd, 0, "Unreadable");
    return -1;
  }

  return 0;
}

int main(int argc, char **argv) {

  // Setup
  /*
   * We initialize variables after checking args
   * The program should only have one arg: the port no.
   * We also setup sockaddr_in struct for ipv4
   * We also setup our array of poll fds for people connected to the server
   * We also setup our clients array
   * We also take care of signal handling for when the server shuts down
   */
  if (argc != 2) {
    fprintf(stderr, "Error: Incorrect # of Arguments\n");
    return EXIT_FAILURE;
  }
  int port = atoi(argv[1]);
  int server_fd;
  int option = 1;

  struct sockaddr_in address;
  memset(&address, 0, sizeof(address));
  address.sin_family = AF_INET;
  address.sin_port = htons(port);
  address.sin_addr.s_addr = INADDR_ANY;
  socklen_t address_length = sizeof(address);

  Client client[SOMAXCONN];
  struct pollfd fds[SOMAXCONN + 1];
  int nfds = 1;  // Number of fds, as poll() takes nfds as 2nd arg
  int ready = 0; // Number of ready fds

  signal(SIGINT, handle_sigint);
  signal(SIGTERM, handle_sigint);

  memset(client, 0, sizeof(client));
  memset(fds, 0, sizeof(fds));

  // Server Setup
  /*
   * We setup the socket, bind, then listen
   */
  if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    perror("Socket");
    return EXIT_FAILURE;
  }
  setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));

  if (bind(server_fd, (struct sockaddr *)&address, address_length) < 0) {
    perror("Bind");
    close(server_fd);
    return EXIT_FAILURE;
  }
  if (listen(server_fd, SOMAXCONN) < 0) {
    perror("Listen");
    close(server_fd);
    return EXIT_FAILURE;
  }

  fds[0].fd = server_fd;
  fds[0].events = POLLIN;

  // Accept Loop
  /*
   * We set while(1) so the server runs indefinitely
   * We check to see if there are any "ready" fds after polling
   * If people have joined (POLLIN), and we have open spots, we add them to fds
   */
  while (running) {
    ready = poll(fds, nfds, -1);
    if (ready < 0) {
      if (errno == EINTR)
        continue;
      perror("Poll");
      close(server_fd);
      return EXIT_FAILURE;
    }

    if (fds[0].revents & POLLIN) {
      address_length = sizeof(address);
      int new_fd =
          accept(server_fd, (struct sockaddr *)&address, &address_length);
      if (new_fd >= 0) {
        if (nfds < SOMAXCONN + 1) {
          fds[nfds].fd = new_fd;
          fds[nfds].events = POLLIN;

          client[nfds - 1].client_fd = new_fd;
          memset(client[nfds - 1].name, 0, NAME_MAX + 1);
          memset(client[nfds - 1].status, 0, STATUS_MAX + 1);
          client[nfds - 1].state = 0;

          nfds++;
        } else {
          close(new_fd);
        }
      }
      ready--;
    }

    for (int i = 1; i < nfds && ready > 0; i++) {
      if (fds[i].revents == 0)
        continue;

      ready--;

      if (fds[i].revents & (POLLERR | POLLHUP | POLLNVAL)) {
        removeClient(i, fds, client, &nfds);
        i--;
        continue;
      }

      if (fds[i].revents & POLLIN) {
        Message message;
        int result = readMessage(fds[i].fd, &message);

        if (result == READ_CLOSED) {
          removeClient(i, fds, client, &nfds);
          i--;
          continue;
        }

        if (result == READ_FATAL) {
          sendError(fds[i].fd, 0, "Unreadable");
          removeClient(i, fds, client, &nfds);
          i--;
          continue;
        }

        if (result == READ_TOO_LONG) {
          sendError(fds[i].fd, 4, "Too long");
          continue;
        }

        if (handleMessage(&client[i - 1], client, nfds, &message) < 0) {
          removeClient(i, fds, client, &nfds);
          i--;
          continue;
        }
      }
    }
  }
  
  printf("Shutting down...\n");
    for (int i = 0; i < nfds; i++) {
        close(fds[i].fd);
    }
  return EXIT_SUCCESS;
}

