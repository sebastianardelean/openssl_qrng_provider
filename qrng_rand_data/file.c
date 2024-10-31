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

#include "file.h"

#define BUFFER_SIZE 4096


int copy_file(const char *source, const char *destination) {
    int src_fd = open(source, O_RDONLY);
    if (src_fd < 0) {
        return -2;
    }

    int dest_fd = open(destination, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    if (dest_fd < 0) {
        close(src_fd);
        return -3;
    }

    char buffer[BUFFER_SIZE] = {0};
    ssize_t bytes_read = 0, bytes_written = 0;

    // Copying loop
    while ((bytes_read = read(src_fd, buffer, BUFFER_SIZE)) > 0) {
        bytes_written = write(dest_fd, buffer, bytes_read);
        if (bytes_written < 0) {
            close(src_fd);
            close(dest_fd);
            return -4;
        }
    }

    // Close the file descriptors
    close(src_fd);
    close(dest_fd);
    
    return (bytes_read < 0) ? -1 : 0;  // Return 0 on success, -1 on failure
}




int create_directory_if_missing(const char *path)
{
    if (check_file_exists(path, true) == false) {
        if(mkdir(path, S_IRUSR|S_IWUSR|S_IXUSR) ==-1) {
            return -1;
        }
    }  
    return 0;
}


bool check_file_exists(const char *path, bool is_dir)
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

int remove_file(const char *path)
{
    if (remove(path) == 0) {
        return 0; 
    } else {
        return -1;
    }
}

int compare_file_size(const char *path, size_t max_size)
{
    struct stat st;
    if (stat(path, &st) != 0) {
        return 0;
    }
    return st.st_size >= max_size;
}
