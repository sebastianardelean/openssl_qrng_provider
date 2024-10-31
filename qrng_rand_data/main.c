#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <syslog.h>
#include <dlfcn.h>



#include "cfg_read.h"



#define FILE_PATH "/tmp/datafile.bin"

#define LOG_NAME "qrng"

static int32_t rng_min_value = 0;
static int32_t rng_max_value = 255;

static int (*qrng_open)(const char *);
static int (*qrng_rand_int32)(int32_t, int32_t, size_t, int32_t *);
static void (*qrng_close)(void);




static void write_random_data(int fd, size_t batch_size)
{
    int32_t *data = malloc(batch_size * sizeof(int32_t));
    if (data) {
        if ((*qrng_rand_int32)(rng_min_value, rng_max_value, batch_size, data) == 0) {
            //print the value to stdout
            write(fd, data, batch_size);
        }
    }
    else {
        syslog(LOG_ERR, "Error on alloc");
        return;
    }


    free(data);



}


int main(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    void *handle_qrng_provider = NULL;
    
    int retval = 0;

    int nochdir = 0;
    int noclose = 0;


    size_t pool_size = 0;
    size_t chunk_size = 0;
 
    char rng_module[256] = {0};
    char *dlerrorstr = NULL;
    openlog(LOG_NAME, LOG_PID | LOG_NOWAIT, LOG_USER);
    

/*
    if (daemon(nochdir, noclose)) {
        syslog(LOG_CRIT, "No daemon was conjured! :(");
        exit(EXIT_FAILURE);
    }
*/
    syslog(LOG_INFO, "Daemon conjured successfully!");
    int fd = open(FILE_PATH, O_WRONLY | O_CREAT | O_APPEND, 0666);
    if (fd == -1) {
        syslog(LOG_CRIT, "The daemon has no power. It couldn't open the file descriptor.");
        closelog();
        exit(EXIT_FAILURE);
    }

    syslog(LOG_INFO, "Created pool fd successfully!");
    
    // init read configurations
    if (cfg_read_init() == -1) {
        syslog(LOG_CRIT, "The daemon doesn't know his powers. The configurations cannot be read!");
        closelog();
        exit(EXIT_FAILURE);
    }
    
    if (cfg_read_run() == -1) {
        syslog(LOG_CRIT, "The daemon doesn't know how to read. The configurations are malformed!");
        closelog();
        exit(EXIT_FAILURE);
    }

    // read configurations
    cfg_read_pool_size(&pool_size);
    cfg_read_chunk_size(&chunk_size);
    cfg_read_rng_module(rng_module);
    cfg_read_min_rng_value(&rng_min_value);
    cfg_read_max_rng_value(&rng_max_value);

    syslog(LOG_INFO, "Finished reading configurations!");

    handle_qrng_provider = dlopen("/usr/local/lib/libqrng.so", RTLD_NOW | RTLD_GLOBAL);
    if (handle_qrng_provider == NULL) {
        syslog(LOG_CRIT, "Poor daemon cannot load the qrng provider: %s", dlerror());
        close(fd);
        closelog();
        exit(EXIT_FAILURE);
    }

    dlerror(); /* Clear any existing error */

    *(void **)(&qrng_open) = dlsym(handle_qrng_provider, "qrng_open");
    
    if ((dlerrorstr = dlerror()) != NULL) {
        syslog(LOG_CRIT,"Poor daemon has problems with dlsym: %s", dlerrorstr);
        close(fd);
        closelog();
        dlclose(handle_qrng_provider);
        exit(EXIT_FAILURE);
    }

    *(void **)(&qrng_rand_int32) = dlsym(handle_qrng_provider, "qrng_random_int32");
    if ((dlerrorstr = dlerror()) != NULL) {
        syslog(LOG_CRIT,"Poor daemon has problems with dlsym: %s", dlerrorstr);
        close(fd);
        closelog();
        dlclose(handle_qrng_provider);
        exit(EXIT_FAILURE);
    }
     
    *(void **)(&qrng_close) = dlsym(handle_qrng_provider, "qrng_close");
    if ((dlerrorstr = dlerror()) != NULL) {
        syslog(LOG_CRIT,"Poor daemon has problems with dlsym: %s", dlerrorstr);
        close(fd);
        closelog();
        dlclose(handle_qrng_provider);
        exit(EXIT_FAILURE);
    }  
    syslog(LOG_INFO, "Finished initializing handlers!");
    
    retval = (*qrng_open)("random.cs.upt.ro");//TODO: Hardcoded for now
    if (retval) {
        syslog(LOG_CRIT,"QRNG init failed");
        close(fd);
        closelog();
        dlclose(handle_qrng_provider);
        exit(EXIT_FAILURE);
    }

    syslog(LOG_INFO, "Daemon connected to QRNG!");
    
    while(1) {
        struct stat st;
        // Lock the file for writing
        flock(fd, LOCK_EX);

        // Get current file size
        if (stat(FILE_PATH, &st) == -1) {
            
            syslog(LOG_WARNING, "Failed to get file stats");
            break;
        }

        // If file size is less than 1GB, write random data
        size_t batch_size = pool_size - st.st_size;
        if (batch_size != 0) {
            if (batch_size > chunk_size) {
                write_random_data(fd, chunk_size);
            }else {
         
                write_random_data(fd,batch_size);
            }
        }

        // Unlock the file
        flock(fd, LOCK_UN);

        // Sleep for a short time before checking the file size again
        usleep(1000);
    }
   
    closelog();
    close(fd);
    (*qrng_close)();
    dlclose(handle_qrng_provider);

    exit(EXIT_SUCCESS);
}

