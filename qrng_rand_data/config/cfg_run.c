#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lex.yy.h"
#include "config.tab.h"

void yyerror(config_t *parm, const char *s) {
    fprintf(stderr, "Error: %s\n", s);
}

int main(int argc, char **argv) {
    int ret = 0;
	
	char * buffer = 0;
	long length;
    config_t config;
    FILE *f = fopen(argv[1], "rb");

	if (f)
	{
	  fseek (f, 0, SEEK_END);
	  length = ftell (f);
	  fseek (f, 0, SEEK_SET);
	  buffer = malloc (length);
	  if (buffer)
	  {
		fread (buffer, 1, length, f);
	  }
	  fclose (f);
	}
	if (buffer) {
		printf("%s\n\n", buffer);
		// Initialize scanner with input string
		YY_BUFFER_STATE buffer_state = yy_scan_string(buffer);

		// Parse with parseResults as parameter
		ret = yyparse(&config);

		// Clean up
		yylex_destroy();

		// Print parsed configuration
		printf("Pool File Size: %ld bytes\n", config.pool_file_size);
		printf("Max Chunk Size: %ld bytes\n", config.max_chunk_size);
		printf("Domain Address: %s\n", config.domain_address);
		printf("Min RNG Value: %d\n", config.min_rng_value);
		printf("Max RNG Value: %d\n", config.max_rng_value);
	}
    

    return ret;
}
