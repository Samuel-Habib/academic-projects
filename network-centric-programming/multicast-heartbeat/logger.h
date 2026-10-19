#ifndef LOGGER_H
#define LOGGER_H

#include <stdio.h>
#include <time.h>

// Define a maximum message length for log entries
#define MAX_LOG_MESSAGE_LENGTH 256

// Function to initialize the logger (e.g., open the log file)
FILE *init_logger(const char *user_id);

// Function to write a log message
void log_message(FILE *log_file, const char *user_id, const char *format, ...);

// Function to close the log file
void close_logger(FILE *log_file);

#endif // LOGGER_H
