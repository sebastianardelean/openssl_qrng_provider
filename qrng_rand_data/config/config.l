%{
#include "config.tab.h"  // Bison-generated header
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

%}

%option nounput noinput noyywrap

%%
#.*                         {} // Ignore comments
[ \t\r\n]+					{}
"KB"               { return KB; }
"MB"               { return MB; }
"GB"               { return GB; }
"pool_file_size"   { return POOL_FILE_SIZE; }
"max_chunk_size"   { return MAX_CHUNK_SIZE; }
"domain_address"   { return DOMAIN_ADDRESS; }
"min_rng_value"    { return MIN_RNG_VALUE; }
"max_rng_value"    { return MAX_RNG_VALUE; }
"="                { return EQUALS; }
[0-9]+             { yylval.ival = atoi(yytext); return INTEGER; }
[a-zA-Z_.]+     { strcpy(yylval.sval, yytext); return STRING; }
%%



