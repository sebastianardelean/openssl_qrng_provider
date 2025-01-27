#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>

#include "file.h"
#include "config.tab.h"
#include "cfg_read.h"


#define RNG_MODULE_NAME_MAX_SIZE 256


static  config_t config;

static const char cfg_file_path[]= "/usr/lib/qrng/qrngd.cnf";







void yyerror(config_t *parm, const char *s) {
    (void)parm;
    fprintf(stderr, "Error: %s\n", s);
}



int cfg_read_init(void)
{

    if (check_file_exists(cfg_file_path, false) == false) {
        return -1;

    } else {
        memset(&config, 0, sizeof(config_t));
    }

    return 0;
    
}

int cfg_read_run(void)
{
    FILE *fcfg = fopen(cfg_file_path, "rb");
    char *buffer = 0;

    size_t cfg_file_size = 0;

    if (fcfg) {
        fseek(fcfg,0, SEEK_END);
        cfg_file_size = ftell (fcfg);
        fseek (fcfg, 0, SEEK_SET);

        buffer = malloc(cfg_file_size);

        if (buffer) {
            fread (buffer, 1, cfg_file_size, fcfg);
            (void)yy_scan_string(buffer);
            (void)yyparse(&config);
            
        }
        else {
            fclose(fcfg);
            return -1;
        }
    }
    else {
        return -1;
    }
    
    yylex_destroy();
    fclose(fcfg);
    free(buffer);
    return 0;
}

void cfg_read_rng_module(char *rng_module)
{
    strncpy(rng_module, config.rng_module,RNG_MODULE_NAME_MAX_SIZE);
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



