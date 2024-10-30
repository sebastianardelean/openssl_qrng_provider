#include <stdio.h>
#include <time.h>
#include <stdarg.h>
#include <stdbool.h>

#include <string.h>
#include "utils.h"
#include "log.h"


#define MAX_LOG_SIZE 1024 * 1024  // 1 MB
#define BACKUP_LOG_FILE_FORMAT "/var/log/qrng.log.%Y-%m-%d_%H-%M-%S"

static const char log_filepath[] = "/var/log/qrng.log";



static bool log_is_initialized = false;

static FILE *logfp = NULL;

static void log_rotate(void);


void init() {
    if (log_is_initialized == false) {
        log_is_initialized = true;
        /* Create the new file path*/
        logfp = fopen(log_filepath, "a");
            
        }
    }
    else {
        /* do nothing */
    }
}

void close() {
    if (logfp != NULL) {
        fclose(logfp);
    }
    log_is_initialized = false;
}

void log(const char *level, const char *format, ...) {
   
    time_t now = time(NULL);
    struct tm *t = localtime(&now);

    if (compare_file_size(log_filepath, MAX_LOG_SIZE)) {
        close();
        log_rotate();
        init();
    }

    // Print timestamp
    fprintf(logfp, "[%02d-%02d-%04d %02d:%02d:%02d] [%s] ",
            t->tm_mday, t->tm_mon + 1, t->tm_year + 1900,
            t->tm_hour, t->tm_min, t->tm_sec, level);

    // Print the message with variable arguments
    va_list args;
    va_start(args, format);
    vfprintf(logfp, format, args);
    va_end(args);

    fprintf(logfp, "\n");

}



void log_rotate(void)
{
    char backup_filename[256];
    time_t now = time(NULL);
    struct tm *t = localtime(&now);

    strftime(backup_filename, sizeof(backup_filename), BACKUP_LOG_FILE_FORMAT, t);

    rename(log_filepath, backup_filename);

}


