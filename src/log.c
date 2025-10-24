#include "log.h"
#include <stdarg.h>
#include <stdio.h>
#include <time.h>
#include <string.h>

static FILE *log_fp = NULL;

void log_init(const char *path) {
    if (!path) return;
    log_fp = fopen(path, "a");
    if (!log_fp) {
        fprintf(stderr, "[LOG] Could not open log file: %s\n", path);
    }
}

void log_close(void) {
    if (log_fp) {
        fclose(log_fp);
        log_fp = NULL;
    }
}

static const char *level_color(const char *level) {
    if (strcmp(level, "ACTIVE") == 0) return "\x1b[32m";   // green
    if (strcmp(level, "DEAD") == 0) return "\x1b[31m";     // red
    if (strcmp(level, "TIMEOUT") == 0) return "\x1b[33m";  // yellow
    if (strcmp(level, "ERROR") == 0) return "\x1b[31m";
    if (strcmp(level, "SUCCESS") == 0) return "\x1b[32m";
    return "\x1b[0m";
}

void log_message(const char *level, const char *fmt, ...) {
    time_t t = time(NULL);
    struct tm tm;
    gmtime_r(&t, &tm);
    char ts[64];
    strftime(ts, sizeof(ts), "%Y-%m-%d %H:%M:%S", &tm);

    va_list ap;
    va_start(ap, fmt);

    char msg[2048];
    vsnprintf(msg, sizeof(msg), fmt, ap);
    va_end(ap);

    const char *color = level_color(level);
    const char *reset = "\x1b[0m";

    fprintf(stdout, "[%s] %s%-7s%s - %s\n", ts, color, level, reset, msg);
    fflush(stdout);

    if (log_fp) {
        fprintf(log_fp, "[%s] %-7s - %s\n", ts, level, msg);
        fflush(log_fp);
    }
}
