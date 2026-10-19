#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <poll.h>
#include <time.h>
#include "logger.h"

#define MC_IP               "239.0.0.1"
#define MC_PORT             8000
#define CMD_PERIOD_MS       5000    /* 5 s */
#define CTRL_LOST_MS        15000   /* 15 s */
#define MAX_ID              32

// message types
typedef enum { MSG_BEAT, MSG_SET, MSG_BYE } MsgTag;

typedef struct {          /* common header */
    uint8_t  tag;         /* MsgTag */
    char     id[MAX_ID];
} __attribute__((packed)) Header;

typedef struct {
    Header  h;
    int32_t interval;     /* network byte‑order not needed on localhost */
} __attribute__((packed)) SetMsg;

typedef struct { Header h; } __attribute__((packed)) BeatMsg;

typedef struct { Header h; } __attribute__((packed)) ByeMsg;


static int  mc_socket(void);
static void mc_send(int s, const void *p, size_t n);
static long now_ms(void);
static int  prompt_int(const char *msg);
static void controller_loop(int s, const char *id, int interval, FILE *logf);
static void client_loop(int s, const char *id, FILE *logf);

int main(int ac, char **av)
{
    if (ac < 2 || strlen(av[1]) >= MAX_ID) {
        fprintf(stderr, "Usage: %s <id> [-c <0‑30>]\n", av[0]);
        return 1;
    }
    const char *id = av[1];

    FILE *logf = init_logger(id);
    if (!logf) return 1;

    int s = mc_socket();
    if (s < 0) { close_logger(logf); return 1; }

    int start = -1;
    if (ac == 4 && strcmp(av[2], "-c") == 0)
        start = atoi(av[3]);

    if (0 <= start && start <= 30)
        controller_loop(s, id, start, logf);
    else
        client_loop(s, id, logf);

    close(s);
    close_logger(logf);
    return 0;
}

static int mc_socket(void)
{
    int s = socket(AF_INET, SOCK_DGRAM, 0);
    if (s < 0) return -1;
    int reuse = 1;
    setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof reuse);
#ifdef SO_REUSEPORT
    setsockopt(s, SOL_SOCKET, SO_REUSEPORT, &reuse, sizeof reuse);
#endif
    struct sockaddr_in a = { .sin_family = AF_INET, .sin_addr.s_addr = htonl(INADDR_ANY), .sin_port = htons(MC_PORT) };
    if (bind(s, (void *)&a, sizeof a) < 0) return -1;

    struct ip_mreq m = { .imr_multiaddr.s_addr = inet_addr(MC_IP), .imr_interface.s_addr = htonl(INADDR_ANY) };
    setsockopt(s, IPPROTO_IP, IP_ADD_MEMBERSHIP, &m, sizeof m);
    unsigned char loop = 1; setsockopt(s, IPPROTO_IP, IP_MULTICAST_LOOP, &loop, sizeof loop);
    return s;
}
static void mc_send(int s, const void *p, size_t n)
{
    struct sockaddr_in d = { .sin_family = AF_INET, .sin_addr.s_addr = inet_addr(MC_IP), .sin_port = htons(MC_PORT) };
    sendto(s, p, n, 0, (void *)&d, sizeof d);
}

static long now_ms(void)
{
    struct timespec ts; clock_gettime(CLOCK_REALTIME, &ts);
    return ts.tv_sec * 1000L + ts.tv_nsec / 1000000L;
}

static void controller_loop(int s, const char *id, int interval, FILE *logf)
{
    log_message(logf, id, "Controller up, interval=%d", interval);
    long next_tx = now_ms();
    struct pollfd pfds[1] = { { .fd = STDIN_FILENO, .events = POLLIN } };

    for (;;) {
        long wait = next_tx - now_ms();
        if (wait < 0) wait = 0;
        poll(pfds, 1, (int)wait);

        if (pfds[0].revents & POLLIN) {
            int val = prompt_int("new interval 0‑30 (exit=‑1): ");
            if (val == -1) break;
            if (0 <= val && val <= 30) {
                interval = val;
                log_message(logf, id, "interval -> %d", interval);
            }
        }
        if (now_ms() >= next_tx) {
            SetMsg m = { .h = { MSG_SET, "" }, .interval = interval };
            strncpy(m.h.id, id, MAX_ID);
            mc_send(s, &m, sizeof m);
            next_tx += CMD_PERIOD_MS;
        }
    }
    ByeMsg bye = { .h = { MSG_BYE, "" } }; strncpy(bye.h.id, id, MAX_ID);
    mc_send(s, &bye, sizeof bye);
}

static void client_loop(int s, const char *id, FILE *logf)
{
    int interval = 0;
    long last_set = now_ms();
    long next_hb  = 0;

    struct pollfd pfds[2] = { { .fd = s, .events = POLLIN }, { .fd = STDIN_FILENO, .events = 0 } };

    for (;;) {
        int tout = 1000; /* 1 s spin */
        poll(pfds, 1, tout);
        if (pfds[0].revents & POLLIN) {
            char buf[256]; recvfrom(s, buf, sizeof buf, 0, NULL, NULL);
            Header *h = (Header *)buf;
            if (strcmp(h->id, id) == 0) continue; /* self */
            if (h->tag == MSG_SET) {
                interval = ((SetMsg *)buf)->interval;
                log_message(logf, id, "new interval %d", interval);
                next_hb = now_ms() + interval * 1000L;
                last_set = now_ms();
            } else if (h->tag == MSG_BYE) {
                log_message(logf, id, "controller down");
                interval = 0;
            }
        }
        /* liveness */
        if (now_ms() - last_set > CTRL_LOST_MS) {
            int val = prompt_int("controller lost, takeover interval 0‑30: ");
            if (0 <= val && val <= 30) { controller_loop(s, id, val, logf); return; }
            last_set = now_ms(); /* reset timer if rejected */
        }
        /* heartbeat */
        if (interval && now_ms() >= next_hb) {
            BeatMsg hb = { .h = { MSG_BEAT, "" } }; strncpy(hb.h.id, id, MAX_ID);
            mc_send(s, &hb, sizeof hb);
            next_hb += interval * 1000L;
        }
    }
}

// console helper
static int prompt_int(const char *msg)
{
    printf("%s", msg); fflush(stdout);
    char buf[32]; if (!fgets(buf, sizeof buf, stdin)) return -1;
    int v = atoi(buf); return v;
}

