#ifndef LOG_H 
#define LOG_H


void log_init(void);

void log_close(void);

void log(const char *level, const char *format, ...);


#endif
