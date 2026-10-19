#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "logger.h"

#define PORT 15432

// This program uses multicast heatbeat logger
int main(int argc, char *argv[])
{
	char user_id[64];
	char *user1 = "user_1", *user2 = "user_2", *user3 = "user_3";
	FILE* logfile;
    struct sockaddr_in addr;
	socklen_t addrlen;
	char ip[32];
	int port, result;
    char msgbuf[256]; //message buffer

	char *usage = "Usage: %s <user_id>, with maximum user_id length 32 characters\n";
	char *send_interval = "Sending SET_INTERVAL %d\n";
	char *send_heartbeat = "Sending Heartbeat\n";
	char *send_controller_down = "Sending CONTROLLER_DOWN\n";
	char *send_bye = "Sending BYE\n"; 

	char *recv_interval = "Received command: SET_INTERVAL %d from controller (%s:%d)\n";
	char *recv_heartbeat = "Received Heartbeat from %s (%s:%d)\n";
	char *recv_controller_down = "Received signal CONTROLLER_DOWN from controller (%s:%d)\n";
	char *recv_bye = "Received BYE from %s (%s:%d)\n";

	// simulate log event intervals
	struct timespec req, rem;
	req.tv_sec = 0;           // Sleep for 0 second
	req.tv_nsec = 500000000L; // Sleep for 500 milliseconds (0.5 seconds)

	// check user_id
	if (argc < 2 || (strlen(argv[1]) > 32)) {
		fprintf(stderr, usage, argv[0]);
		exit(EXIT_FAILURE);
	}

	strcpy(user_id, argv[1]);

    /* set up IP address */
    memset(&addr,0,sizeof(addr));
    addr.sin_family=AF_INET;
	inet_pton(AF_INET,"127.0.0.1",&(addr.sin_addr.s_addr));
    addr.sin_port=htons(PORT);

	// extract IP and port for logging
	if (inet_ntop(AF_INET, &(addr.sin_addr.s_addr), ip, sizeof(ip)) == NULL) {
		perror("inet_ntop()");
		exit(EXIT_FAILURE);
	}
	port = ntohs(addr.sin_port);

	// open log file
	logfile = init_logger(user_id);
	if (logfile == NULL) {
		perror("Open file");
		exit(EXIT_FAILURE);
	}

	// log send command
	log_message(logfile, user_id, send_interval, 5);
	result = nanosleep(&req, &rem);
	if (result < 0) {
		perror("nanosleep");
	}

	// log send command
	log_message(logfile, user_id, send_interval, 0);
	result = nanosleep(&req, &rem);
	if (result < 0) {
		perror("nanosleep");
	}

	// log send heartbeat
	log_message(logfile, user_id, send_heartbeat);
	result = nanosleep(&req, &rem);
	if (result < 0) {
		perror("nanosleep");
	}

	// log send CONTROLLER_DOWN
	log_message(logfile, user_id, send_controller_down);
	result = nanosleep(&req, &rem);
	if (result < 0) {
		perror("nanosleep");
	}

	// log send BYE 
	log_message(logfile, user_id, send_bye);
	result = nanosleep(&req, &rem);
	if (result < 0) {
		perror("nanosleep");
	}

	// log receive command
	log_message(logfile, user_id, recv_interval, 5, ip, port);
	result = nanosleep(&req, &rem);
	if (result < 0) {
		perror("nanosleep");
	}

	// log receive heartbeat
	log_message(logfile, user_id, recv_heartbeat, user1, ip, 12350);
	result = nanosleep(&req, &rem);
	if (result < 0) {
		perror("nanosleep");
	}

	// log receive heartbeat
	log_message(logfile, user_id, recv_heartbeat, user2, ip, 12301);
	result = nanosleep(&req, &rem);
	if (result < 0) {
		perror("nanosleep");
	}

	// log receive heartbeat
	log_message(logfile, user_id, recv_heartbeat, user3, ip, 10234);
	result = nanosleep(&req, &rem);
	if (result < 0) {
		perror("nanosleep");
	}

	// log receive CONTROLLER_DONW 
	log_message(logfile, user_id, recv_controller_down, ip, port);
	result = nanosleep(&req, &rem);
	if (result < 0) {
		perror("nanosleep");
	}

	// log receive BYE 
	log_message(logfile, user_id, recv_bye, user2, ip, port);
	result = nanosleep(&req, &rem);
	if (result < 0) {
		perror("nanosleep");
	}

	// close log file
	close_logger(logfile);
}
