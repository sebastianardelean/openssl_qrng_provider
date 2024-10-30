#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <qrng.h>
#include "cfg_read.h"



#define FILE_PATH "/tmp/datafile.bin"


static int32_t rng_min_value = 0;
static int32_t rng_max_value = 255;


static void write_random_data(int fd, size_t batch_size)
{
    int32_t *data = malloc(batch_size * sizeof(int32_t));
    if (data) {
        if (qrng_random_int32(rng_min_value, rng_max_value, batch_size, data) == 0) {
            //print the value to stdout
            write(fd, data, batch_size);
        }
    }
    else {
        perror("Error on alloc");
        return;
    }


    free(data);



}


int main(int argc, char **argv)
{
    (void)argc;
    (void)argv;
    int retval = 0;

    int nochdir = 0;
    int noclose = 0;


    size_t pool_size = 0;
    size_t chunk_size = 0;
 
    char domain[256] = {0};
    
    //create the good demon

    if (daemon(nochdir, noclose)) {
        perror("No daemon was conjured! :(");
        exit(EXIT_FAILURE);
    }

    int fd = open(FILE_PATH, O_WRONLY | O_CREAT | O_APPEND, 0666);
    if (fd == -1) {
        perror("Failed to open file");
        exit(EXIT_FAILURE);
    }

    // init read configurations
    cfg_read_init();
    cfg_read_run();

    // read configurations
    cfg_read_pool_size(&pool_size);
    cfg_read_chunk_size(&chunk_size);
    cfg_read_domain_address(domain);
    cfg_read_min_rng_value(&rng_min_value);
    cfg_read_max_rng_value(&rng_max_value);

    
    retval = qrng_open(domain);
    if (retval) {
        perror("QRNG init failed");
        exit(EXIT_FAILURE);
    }

    
    while(1) {
        struct stat st;
        // Lock the file for writing
        flock(fd, LOCK_EX);

        // Get current file size
        if (stat(FILE_PATH, &st) == -1) {
            perror("Failed to get file stats");
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
   

    close(fd);
    qrng_close();
    

    exit(EXIT_SUCCESS);
}

