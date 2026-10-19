/*
 * proxy.c - A Simple Sequential Web proxy
 *
 * Course Name: 14:332:456-Network Centric Programming
 * Assignment 2
 * Student Name: Samuel Habib
 * 
 * IMPORTANT: Give a high level description of your code here. You
 * must also provide a header comment at the beginning of each
 * function that describes what that function does.
 */ 

// High-level description:
// This web proxy listens on a specified port, accepts HTTP requests from clients,
// forwards them to a target server, retrieves the server's response, and returns
// it to the client. It also logs each request and the response size in proxy.log.

#include "csapp.h"

/* Function prototypes */
void format_log_entry(char *logstring, struct sockaddr_in *sockaddr, char *uri, int size);
void process_request(int clientfd);
void parse_url(char *url, char *hostname, char *path, int *port);
void forward_request(int serverfd, char *client_request, char *hostname, char *path);
void log_request(struct sockaddr_in *client_addr, char *uri, int size);


/* 
 * main - Entry point. Sets up a listening socket on the given port,
 *        accepts connections, and processes each request.
 */

 int main(int argc, char **argv)
 {
     int listenfd, connfd;
     socklen_t clientlen;
     struct sockaddr_in clientaddr;
     
     /* Check arguments */
     if (argc != 2) {
         fprintf(stderr, "Usage: %s <port number>\n", argv[0]);
         exit(0);
     }
 
     int port = atoi(argv[1]);
     listenfd = Open_listenfd(port);
     
     while (1) {
         clientlen = sizeof(clientaddr);
         connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen);
         process_request(connfd);
         Close(connfd);
     }
     
     exit(0);
 }


/*
 * process_request - Reads client request, parses method & URL,
 *                   forwards the request to the target server,
 *                   and sends the response back to the client.
 */
void process_request(int clientfd)
{
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
    if (!Rio_readlineb(&rio_client, buf, MAXLINE)) {
        return;   // Remove closing here; let main close clientfd
    }
    
    sscanf(buf, "%s %s %s", method, url, version);
    
    /* Silently ignore all CONNECT requests (HTTPS) */
    if (strcasecmp(method, "CONNECT") == 0) {
        return;
    }
    
    /* Only handle GET requests */
    if (strcasecmp(method, "GET") != 0) {
        return;
    }
    
    /* Parse URL to extract hostname, path, and port */
    parse_url(url, hostname, path, &port);
    
    printf("Client connected:\n%s %s %s\n", method, url, version);
    
    /* Connect to destination server */
    if ((serverfd = open_clientfd(hostname, port)) < 0) {
        return;
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
    if (hostname_start == NULL) {
        hostname_start = url;
    } else {
        hostname_start += 3;
    }
    
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
    char buf[MAXLINE];
    char request_line[MAXLINE];
    
    /* Modify the request line to use relative path */
    sprintf(request_line, "GET %s HTTP/1.0\r\n", path);
    Rio_writen(serverfd, request_line, strlen(request_line));
    
    /* Add Host header if not present */
    sprintf(buf, "Host: %s\r\n", hostname);
    Rio_writen(serverfd, buf, strlen(buf));
    
    /* End headers */
    Rio_writen(serverfd, "\r\n", 2);
}

/*
 * log_request - Logs the request and size to proxy.log.
 */
void log_request(struct sockaddr_in *client_addr, char *uri, int size)
{
    char logstring[MAXLINE];
    FILE *logfile;
    
    format_log_entry(logstring, client_addr, uri, size);
    
    logfile = Fopen("proxy.log", "a");
    Fputs(logstring, logfile);
    Fputs("\n", logfile);
    Fclose(logfile);
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

    /* Get a formatted time string */
    now = time(NULL);
    strftime(time_str, MAXLINE, "%a %d %b %Y %H:%M:%S %Z", localtime(&now));


    /* 
     * Convert the IP address in network byte order to dotted decimal
     * form. Note that we could have used inet_ntoa, but chose not to
     * because inet_ntoa is a Class 3 thread unsafe function that
     * returns a pointer to a static variable (Ch 13, CS:APP).
     */

    host = ntohl(sockaddr->sin_addr.s_addr);
    a = host >> 24;
    b = (host >> 16) & 0xff;
    c = (host >> 8) & 0xff;
    d = host & 0xff;

    /* Return the formatted log entry string */
    sprintf(logstring, "%s: %d.%d.%d.%d %s %d", time_str, a, b, c, d, uri, size);
}
