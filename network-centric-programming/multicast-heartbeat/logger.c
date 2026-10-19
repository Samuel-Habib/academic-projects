#include "logger.h"
#include <stdarg.h>
#include <string.h>

FILE *init_logger(const char *user_id) {
	char filename[64];
	snprintf(filename, sizeof(filename), "heartbeat_app_%s.log", user_id);
	FILE *log_file = fopen(filename, "a");
	if (!log_file) {
		perror("Error opening log file");
		return NULL;
	}
	return log_file;
}

void log_message(FILE *log_file, const char *user_id, const char *format, ...) {
	if (log_file) {
		time_t timer;
		char timestamp[26];
		struct tm* tm_info;

		time(&timer);
		tm_info = localtime(&timer);
		strftime(timestamp, 26, "[%Y-%m-%d %H:%M:%S.", tm_info);
		struct timespec ts;
		timespec_get(&ts, TIME_UTC);
		long milliseconds = (ts.tv_nsec / 1000000);
		char timestamp_ms[64];
		snprintf(timestamp_ms, sizeof(timestamp_ms), "%s%03ld] %s: ", timestamp, milliseconds, user_id);

		char message[MAX_LOG_MESSAGE_LENGTH];
		va_list args;
		va_start(args, format);
		vsnprintf(message, MAX_LOG_MESSAGE_LENGTH - strlen(timestamp_ms) - 2, format, args); // -2 for newline and null terminator
		va_end(args);
		fprintf(log_file, "%s%s\n", timestamp_ms, message);
		fflush(log_file); // Important to flush to ensure logs are written promptly
	}
}

void close_logger(FILE *log_file) {
	if (log_file) {
		fclose(log_file);
	}
}
