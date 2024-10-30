%code requires {
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

typedef struct _config_t {
	size_t pool_file_size;
	size_t max_chunk_size;
	char domain_address[256];
	int32_t min_rng_value;
	int32_t max_rng_value;
}config_t;


extern int yylex(void);
extern void yyerror(config_t *parm,const char *s);


}

%parse-param {config_t *parm}

%union {
    int ival;
    char sval[256];
}



%token <ival> INTEGER
%token <sval> STRING
%token KB MB GB
%token POOL_FILE_SIZE MAX_CHUNK_SIZE DOMAIN_ADDRESS MIN_RNG_VALUE MAX_RNG_VALUE
%token EQUALS
%%

config: 
	| config config_line
	
	;

config_line:
    POOL_FILE_SIZE EQUALS INTEGER GB { ((config_t *)parm)->pool_file_size = $3 * 1024 * 1024 * 1024;}
	| POOL_FILE_SIZE EQUALS INTEGER MB { ((config_t *)parm)->pool_file_size = $3 * 1024 * 1024;}
	| POOL_FILE_SIZE EQUALS INTEGER KB { ((config_t *)parm)->pool_file_size = $3 * 1024;}
    | MAX_CHUNK_SIZE EQUALS INTEGER KB { ((config_t *)parm)->max_chunk_size = $3 * 1024;}
    | DOMAIN_ADDRESS EQUALS STRING { strcpy(((config_t *)parm)->domain_address, $3); }
    | MIN_RNG_VALUE EQUALS INTEGER { ((config_t *)parm)->min_rng_value = $3; }
    | MAX_RNG_VALUE EQUALS INTEGER { ((config_t *)parm)->max_rng_value = $3; }
    ;
%%


