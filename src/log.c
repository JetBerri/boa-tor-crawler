#include "log.h"
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

static FILE *log_file = NULL;

void log_init() {
    struct stat st = {0};
    if (stat(LOG_PATH, &st) == -1) {
        mkdir(LOG_PATH, 0755);
    }

    time_t t = time(NULL);
    struct tm *tm_info = localtime(&t);
    char filename[256];
    strftime(filename, sizeof(filename), LOG_PATH "/crawler_%Y-%m-%d.log", tm_info);

    log_file = fopen(filename, "a");
    if (!log_file) {
        fprintf(stderr, "ERROR: Could not open log file %s\n", filename);
        exit(EXIT_FAILURE);
    }
}

void log_message(const char *level, const char *format, ...) {
    if (!log_file) log_init();

    time_t now = time(NULL);
    struct tm *tm_info = localtime(&now);

    char timestamp[64];
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", tm_info);

    fprintf(log_file, "[%s] %-7s - ", timestamp, level);

    va_list args;
    va_start(args, format);
    vfprintf(log_file, format, args);
    va_end(args);

    fprintf(log_file, "\n");
    fflush(log_file);
}

void log_close() {
    if (log_file) {
        fclose(log_file);
        log_file = NULL;
    }
}
