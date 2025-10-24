#ifndef LOG_H
#define LOG_H

void log_init(const char *path);
void log_close(void);
void log_message(const char *level, const char *fmt, ...);

#endif // LOG_H
