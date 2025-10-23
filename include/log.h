#ifndef LOG_H
#define LOG_H

#include <stdio.h>
#include <time.h>
#include <stdarg.h>

#define LOG_PATH "./logs"

void log_init();
void log_message(const char *level, const char *format, ...);
void log_close();

#endif
