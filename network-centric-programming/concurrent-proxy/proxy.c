/*
 * proxy.c - A Simple Sequential Web proxy (extended for concurrency)
 *
 * Course Name: 14:332:456-Network Centric Programming
 * Assignment 3
 * Student Name: Samuel Habib
 *
 * IMPORTANT: Give a high level description of your code here. You
 * must also provide a header comment at the beginning of each
 * function that describes what that function does.
 */

// High-level description:
// This web proxy listens on a specified port, accepts HTTP requests from clients,
// forwards them to a target server, retrieves the server's response, and returns
// it to the client. It also handles concurrency via processes (-p) or threads (-t),
// supports HTTPS CONNECT tunneling, and logs each request to proxy.log.

#include "csapp.h"
#include <ctype.h>

/* Function prototypes */
int valid_port(char *str);
void format_log_entry(char *logstring, struct sockaddr_in *sockaddr, char *uri, int size);
void process_request(int clientfd);
void parse_url(char *url, char *hostname, char *path, int *port);
void forward_request(int serverfd, char *client_request, char *hostname, char *path);
void log_request(struct sockaddr_in *client_addr, char *uri, int size);

/* Additional prototypes for concurrency and tunneling */
void *thread_main(void *vargp);
int Open_clientfd_ts(char *hostname, int port);
void format_tunnel_log_entry(char *logstring, struct sockaddr_in *sockaddr,
                             char *hostname, int port, int sent, int received);
void log_tunnel(struct sockaddr_in *client_addr, char *hostname, int port,
                int bytes_sent, int bytes_received);
void tunnel_request(int clientfd, char *hostname, int port,
                    struct sockaddr_in *client_addr);

/* Globals */
int use_threads = 0;
pthread_mutex_t dns_mutex = PTHREAD_MUTEX_INITIALIZER;

/*
 * main - Entry point. Sets up a listening socket on the given port,
 *        accepts connections, and processes each request (process/thread).
 */
int main(int argc, char **argv)
{
    struct flock lock;
    memset(&lock, 0, sizeof(lock));
    pid_t pid;

    int listenfd, connfd;
    socklen_t clientlen;
    struct sockaddr_in clientaddr;
    pthread_t tid;
    int option = 0;  // 1 = process, 2 = thread
    int port;

    // Check args
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <option> <port number> \n", argv[0]);
        exit(0);
    }

    // options
    if (strcasecmp(argv[1], "-p") == 0 || strcasecmp(argv[1], "--process") == 0) {
        option = 1;
    }
    if (strcasecmp(argv[1], "-t") == 0 || strcasecmp(argv[1], "--thread") == 0) {
        option = 2;
    }
    if (!option) {
        printf("Usage: %s [options]\n", argv[0]);
        fprintf(stderr, "    -p, --process      Use processes for concurrency\n");
        fprintf(stderr, "    -t, --thread       Use threads for concurrency\n");
        exit(1);
    }
    use_threads = (option == 2);

    // Validate port
    if (valid_port(argv[2]) == -1) {
        fprintf(stderr, "%s please specify a port number in the range 1024–49151\n", argv[2]);
        exit(1);
    }
    port = atoi(argv[2]);
    listenfd = Open_listenfd(port);

    // Concurrency loop
    if (option == 1) {
        // Process-based
        while (1) {
            clientlen = sizeof(clientaddr);
            connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen);
            if ((pid = Fork()) == 0) { // CHILD
                lock.l_type = F_WRLCK;
                fcntl(connfd, F_SETLKW, &lock);

                process_request(connfd);

                lock.l_type = F_UNLCK;
                fcntl(connfd, F_SETLKW, &lock);
                Close(connfd);
                exit(0);
            } else {
                Close(connfd);
            }
        }
    }

    if (option == 2) {
        // Thread-based
        while (1) {
            clientlen = sizeof(clientaddr);
            connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen);
            int *connp = Malloc(sizeof(int));
            *connp = connfd;
            Pthread_create(&tid, NULL, thread_main, connp);
        }
    }

    exit(0);
}

/*
 * thread_main - Thread entry point: detach and handle request.
 */
void *thread_main(void *vargp)
{
    int connfd = *((int *)vargp);
    free(vargp);
    Pthread_detach(Pthread_self());
    process_request(connfd);
    Close(connfd);
    return NULL;
}

/*
 * valid_port - Takes in a string argument for port, stores the original
 *              checks to see if it's a number and within the application
 *              port range and then returns -1 or 0 depending on if it passes
 */
int valid_port(char *str)
{
    char *og = str;
    while (*str) {
        if (!isdigit(*str)) return -1;
        str++;
    }
    int num = atoi(og);
    if (num < 1023 || num > 49151) return -1;
    return 0;
}

/*
 * process_request - Reads client request, parses method & URL,
 *                   forwards the request to the target server,
 *                   and sends the response back to the client.
 */
void process_request(int clientfd)
{
    printf("Accepted connection\n");
    fflush(stdout);
    char buf[MAXLINE], method[MAXLINE], url[MAXLINE], version[MAXLINE];
    char hostname[MAXLINE], path[MAXLINE];
    int port, serverfd;
    rio_t rio_client, rio_server;
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);

    /* Get client address for logging */
    getpeername(clientfd, (SA *)&client_addr, &client_len);

    /* Read request line */
    Rio_readinitb(&rio_client, clientfd);
    if (!Rio_readlineb(&rio_client, buf, MAXLINE)) return;
    sscanf(buf, "%s %s %s", method, url, version);

    /* Handle CONNECT for HTTPS tunneling */
    if (strcasecmp(method, "CONNECT") == 0) {
        char host[MAXLINE];
        int prt;
        sscanf(url, "%[^:]:%d", host, &prt);
        tunnel_request(clientfd, host, prt, &client_addr);
        return;
    }

    /* Only handle GET requests */
    if (strcasecmp(method, "GET") != 0) return;

    /* Parse URL to extract hostname, path, and port */
    parse_url(url, hostname, path, &port);

    printf("Client connected:\n%s %s %s\n", method, url, version);
    fflush(stdout);

    /* Connect to destination server */
    if ((serverfd = Open_clientfd(hostname, port)) < 0) {
        printf("NOT connected to dest server \n");
        return;
    } else {
        printf("connected to dest server \n");
    }

    /* Forward request to server */
    forward_request(serverfd, buf, hostname, path);

    /* Read server's response and forward to client while counting bytes */
    Rio_readinitb(&rio_server, serverfd);
    int total_bytes = 0;
    ssize_t n;
    while ((n = Rio_readnb(&rio_server, buf, MAXLINE)) > 0) {
        Rio_writen(clientfd, buf, n);
        total_bytes += n;
    }

    /* Log the successful request */
    log_request(&client_addr, url, total_bytes);

    Close(serverfd);
}

/*
 * parse_url - Extracts the hostname, path, and port from a URL.
 */
void parse_url(char *url, char *hostname, char *path, int *port)
{
    char *hostname_start = strstr(url, "://");
    if (hostname_start == NULL) hostname_start = url;
    else hostname_start += 3;
    char *hostname_end = strchr(hostname_start, '/');
    if (hostname_end == NULL) {
        strcpy(path, "/");
        hostname_end = hostname_start + strlen(hostname_start);
    } else {
        strcpy(path, hostname_end);
        *hostname_end = '\0';
    }
    char *port_start = strchr(hostname_start, ':');
    if (port_start == NULL) {
        *port = 80;
        strcpy(hostname, hostname_start);
    } else {
        *port = atoi(port_start + 1);
        *port_start = '\0';
        strcpy(hostname, hostname_start);
    }
}

/*
 * forward_request - Sends a TCP request to the server using a relative path,
 *                   including required headers such as Host.
 */
void forward_request(int serverfd, char *client_request, char *hostname, char *path)
{
    char buf[MAXLINE], request_line[MAXLINE];
    sprintf(request_line, "GET %s HTTP/1.0\r\n", path);
    Rio_writen(serverfd, request_line, strlen(request_line));
    sprintf(buf, "Host: %s\r\n", hostname);
    Rio_writen(serverfd, buf, strlen(buf));
    Rio_writen(serverfd, "\r\n", 2);
}

/*
 * log_request - Logs the request and size to proxy.log.
 */
void log_request(struct sockaddr_in *client_addr, char *uri, int size)
{
    char logstring[MAXLINE];
    int logfd;
    format_log_entry(logstring, client_addr, uri, size);
    if ((logfd = Open("proxy.log", O_WRONLY|O_CREAT|O_APPEND, 0644)) < 0) {
        perror("Failed to open proxy.log");
        return;
    }
    dprintf(logfd, "%s\n", logstring);
    Close(logfd);
}

/*
 * format_log_entry - Create a formatted log entry in logstring.
 *
 * The inputs are the socket address of the requesting client
 * (sockaddr), the URI from the request (uri), and the size in bytes
 * of the response from the server (size).
 */
void format_log_entry(char *logstring, struct sockaddr_in *sockaddr,
                      char *uri, int size)
{
    time_t now;
    char time_str[MAXLINE];
    unsigned long host;
    unsigned char a, b, c, d;

    now = time(NULL);
    strftime(time_str, MAXLINE, "%a %d %b %Y %H:%M:%S %Z", localtime(&now));

    host = ntohl(sockaddr->sin_addr.s_addr);
    a = host >> 24;
    b = (host >> 16) & 0xff;
    c = (host >> 8) & 0xff;
    d = host & 0xff;

    sprintf(logstring, "%s: %d.%d.%d.%d %s %d", time_str, a, b, c, d, uri, size);
}

/*
 * format_tunnel_log_entry - Create a formatted log entry for CONNECT tunneling
 */
void format_tunnel_log_entry(char *logstring, struct sockaddr_in *sockaddr,
                             char *hostname, int port, int sent, int received)
{
    time_t now = time(NULL);
    char ts[MAXLINE];
    strftime(ts, MAXLINE, "%a %d %b %Y %H:%M:%S %Z", localtime(&now));
    unsigned long addr = ntohl(sockaddr->sin_addr.s_addr);
    unsigned char a = addr>>24, b=(addr>>16)&0xFF, c=(addr>>8)&0xFF, d=addr&0xFF;
    sprintf(logstring,
            "%s: CONNECT from %d.%d.%d.%d to %s:%d, Sent: %d bytes, Received: %d bytes",
            ts, a,b,c,d, hostname, port, sent, received);
}

/*
 * log_tunnel - Logs the CONNECT metadata to proxy.log.
 */
void log_tunnel(struct sockaddr_in *client_addr, char *hostname, int port,
                int bytes_sent, int bytes_received)
{
    char logstring[MAXLINE];
    int logfd = Open("proxy.log", O_WRONLY|O_CREAT|O_APPEND, 0644);
    struct flock lock = {0}; lock.l_type = F_WRLCK;
    fcntl(logfd, F_SETLKW, &lock);
    format_tunnel_log_entry(logstring, client_addr, hostname, port,
                             bytes_sent, bytes_received);
    dprintf(logfd, "%s\n", logstring);
    lock.l_type = F_UNLCK; fcntl(logfd, F_SETLK, &lock);
    Close(logfd);
}

/*
 * Open_clientfd_ts - Thread-safe wrapper around Open_clientfd
 *                    to protect DNS lookups with a mutex.
 */
int Open_clientfd_ts(char *hostname, int port)
{
    pthread_mutex_lock(&dns_mutex);
    int fd = Open_clientfd(hostname, port);
    pthread_mutex_unlock(&dns_mutex);
    return fd;
}

/*
 * tunnel_request - Establishes a CONNECT tunnel and relays raw data
 *                  between client and server until closed or timed out.
 */
void tunnel_request(int clientfd, char *hostname, int port,
                    struct sockaddr_in *client_addr)
{
    int serverfd;
    char buf[MAXLINE];
    ssize_t n;
    int sent = 0, received = 0;
    fd_set reads;
    struct timeval timeout;
    int maxfd;

    serverfd = use_threads ? Open_clientfd_ts(hostname, port)
                           : Open_clientfd(hostname, port);
    if (serverfd < 0) return;

    printf("CONNECT to %s:%d\n", hostname, port); fflush(stdout);
    sprintf(buf, "HTTP/1.0 200 Connection Established\r\n\r\n");
    Rio_writen(clientfd, buf, strlen(buf));

    maxfd = (clientfd > serverfd ? clientfd : serverfd) + 1;
    timeout.tv_sec = 60; timeout.tv_usec = 0;

    while (1) {
        FD_ZERO(&reads);
        FD_SET(clientfd, &reads);
        FD_SET(serverfd, &reads);
        if (Select(maxfd, &reads, NULL, NULL, &timeout) <= 0) break;
        if (FD_ISSET(clientfd, &reads)) {
            n = Read(clientfd, buf, MAXLINE);
            if (n <= 0) break;
            Rio_writen(serverfd, buf, n); sent += n;
        }
        if (FD_ISSET(serverfd, &reads)) {
            n = Read(serverfd, buf, MAXLINE);
            if (n <= 0) break;
            Rio_writen(clientfd, buf, n); received += n;
        }
    }
    Close(serverfd);
    printf("Tunnel closed: sent %d, received %d\n", sent, received);
    fflush(stdout);
    log_tunnel(client_addr, hostname, port, sent, received);
}

