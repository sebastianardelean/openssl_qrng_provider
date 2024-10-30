#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>

#include "utils.h"
#include "config.tab.h"
#include "cfg_read.h"


#define DOMAIN_ADDRESS_MAX_SIZE 256


static  config_t config;

static const char cfg_file_path[]= "/usr/lib/qrng/qrng.cnf";
static const char cfg_dir_path[] = "/usr/lib/qrng";

static const char cfg_file_default[]="qrng.cnf";




void yyerror(config_t *parm, const char *s) {
    (void)parm;
    fprintf(stderr, "Error: %s\n", s);
}



void cfg_read_init(void)
{
    create_directory_if_missing(cfg_dir_path);
    if (check_file_exists(cfg_file_path, false) == false) {
        //create default configuration
        if(copy_file(cfg_file_default, cfg_file_path) == -1) {
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



