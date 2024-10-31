#ifndef FILE_H
#define FILE_H

int copy_file(const char *source, const char *destination);
int create_directory_if_missing(const char *path);
bool check_file_exists(const char *path, bool is_dir);
int remove_file(const char *path);
int compare_file_size(const char *path, size_t max_size);

#endif
