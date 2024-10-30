#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>

#include "config.tab.h"
#include "cfg_read.h"

#define BUFFER_SIZE 4096
#define DOMAIN_ADDRESS_MAX_SIZE 256


static  config_t config;

static const char cfg_file_path[]= "/usr/lib/qrng/qrng.cnf";
static const char cfg_dir_path[] = "/usr/lib/qrng";

static const char cfg_file_default[]="qrng.cnf";

static int cfg_read_copy_file(const char *source, const char *destination);
static int cfg_read_create_directory_if_missing(const char *path);
static bool cfg_read_check_file_exists(const char *path, bool is_dir);



void yyerror(config_t *parm, const char *s) {
    (void)parm;
    fprintf(stderr, "Error: %s\n", s);
}



void cfg_read_init(void)
{
    cfg_read_create_directory_if_missing(cfg_dir_path);
    if (cfg_read_check_file_exists(cfg_file_path, false) == false) {
        //create default configuration
        if(cfg_read_copy_file(cfg_file_default, cfg_file_path) == -1) {
            //TODO: we have an error
        }
    }
    memset(&config, 0, sizeof(config_t));
    
}

void cfg_read_run(void)
{
    FILE *fcfg = fopen(cfg_file_path, "rb");
    char *buffer = 0;

    size_t cfg_file_size = 0;

    if (fcfg) {
        fseek(fcfg,0, SEEK_END);
        cfg_file_size = ftell (fcfg);
        fseek (fcfg, 0, SEEK_SET);
        buffer = malloc(cfg_file_size);
        if (buffer)
        {
            fread (buffer, 1, cfg_file_size, fcfg);
        }
        else {
            fprintf(stderr, "Could not allocate memory for the config file content");
            fclose(fcfg);
        }
    }
    if (buffer) {
        (void)yy_scan_string(buffer);

        (void)yyparse(&config);
        yylex_destroy();
    }
}

void cfg_read_domain_address(char *domain_address)
{
    strncpy(domain_address, config.domain_address,DOMAIN_ADDRESS_MAX_SIZE);
}

void cfg_read_pool_size(size_t *pool_size)
{
    *pool_size = config.pool_file_size;
}

void cfg_read_chunk_size(size_t *chunk_size)
{
    *chunk_size = config.max_chunk_size;
}

void cfg_read_min_rng_value(int32_t *min_value)
{
    *min_value = config.min_rng_value;
}

void cfg_read_max_rng_value(int32_t *max_value)
{
    *max_value = config.max_rng_value;
}



int cfg_read_copy_file(const char *source, const char *destination) {
    int src_fd = open(source, O_RDONLY);
    if (src_fd < 0) {
        fprintf(stderr, "Error opening source file %s", strerror(errno));
        return -1;
    }

    int dest_fd = open(destination, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    if (dest_fd < 0) {
        fprintf(stderr, "Error opening destination file %s", strerror(errno));
        close(src_fd);
        return -1;
    }

    char buffer[BUFFER_SIZE];
    ssize_t bytes_read, bytes_written;

    // Copying loop
    while ((bytes_read = read(src_fd, buffer, BUFFER_SIZE)) > 0) {
        bytes_written = write(dest_fd, buffer, bytes_read);
        if (bytes_written < 0) {
            fprintf(stderr, "Error writing %s", strerror(errno));
            close(src_fd);
            close(dest_fd);
            return -1;
        }
    }

    if (bytes_read < 0) {
        fprintf(stderr, "Error reading %s", strerror(errno));
    }

    // Close the file descriptors
    close(src_fd);
    close(dest_fd);
    
    return (bytes_read < 0) ? -1 : 0;  // Return 0 on success, -1 on failure
}




int cfg_read_create_directory_if_missing(const char *path)
{
    if (cfg_read_check_file_exists(path, true) == false) {
        if(mkdir(path, S_IRUSR|S_IWUSR|S_IXUSR) ==-1) {
            fprintf(stderr, "Cannot create directory %s - Error %s\n",path,strerror(errno));
            return -1;
        }
    }  
    return 0;
}


bool cfg_read_check_file_exists(const char *path, bool is_dir)
{
    struct stat sb;
    if (is_dir == true) {
        if (stat(path, &sb) == 0 && S_ISDIR(sb.st_mode)) {
            return true;
        }
    }
    else {
        if (stat(path, &sb) == 0 && S_ISREG(sb.st_mode)) {
            return true;
        }
    }

    return false;
}


