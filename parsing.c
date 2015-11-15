#include <stdio.h>
#include <stdlib.h>
#include "mpc.h"

/* If compiling on windows */
#ifdef _WIN32
#include <string.h> 
char * readline( char *prompt );
/* Static global variable is only seen within the file it's declared in  
	Static local variable can is saved between invocations
	size(input) = 2048 = 2^10 
*/
static char buffer[2048];

/* Fake readline function */
char * readline( char *prompt ) {
	fputs( prompt, stdout ); 
	fgets( buffer, sizeof(buffer), stdin );
	char *cpy = malloc( strlen(buffer)+1 );
	strcpy( cpy, buffer );
	cpy[strlen( cpy )-1] = '\0';
	return cpy;
}

/* Fake add history function */
void add_history( char* unused ){};

/* Otherwise include editline headers */
#else
/* to use the editline library 
	Linux: sudo apt-get install libedit -dev */
#include <editline/readline.h>
#include <editline/history.h>
#endif


int main(int argc, char ** argv) {

	  /* Create Some Parsers */
  mpc_parser_t* Number   = mpc_new("number");
  mpc_parser_t* Operator = mpc_new("operator");
  mpc_parser_t* Expr     = mpc_new("expr");
  mpc_parser_t* Lispy    = mpc_new("lispy");
  
  /* Define them with the following Language */
  mpca_lang(MPCA_LANG_DEFAULT,
    "                                                     \
      number   : /-?[0-9]+/ ;                             \
      operator : '+' | '-' | '*' | '/' ;                  \
      expr     : <number> | '(' <operator> <expr>+ ')' ;  \
      lispy    : /^/ <operator> <expr>+ /$/ ;             \
    ",
    Number, Operator, Expr, Lispy);


	printf("JLISP Version 0.0.0.0.1\n");
	printf("\tEnter Ctrl+c to exit\n\n");

	while( 1 ){

	 char* input = readline("lispy> ");
    add_history(input);
    
    /* Attempt to parse the user input */
    mpc_result_t r;
    if (mpc_parse("<stdin>", input, Lispy, &r)) {
      /* On success print and delete the AST */
      mpc_ast_print(r.output);
      mpc_ast_delete(r.output);
    } else {
      /* Otherwise print and delete the Error */
      mpc_err_print(r.error);
      mpc_err_delete(r.error);
    }
    
    free(input);
  }
  
  /* Undefine and delete our parsers */
  mpc_cleanup(4, Number, Operator, Expr, Lispy);
  
  return 0;
}