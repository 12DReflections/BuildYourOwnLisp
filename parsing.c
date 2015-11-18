#include "parsing.h"
#include "mpc.h"

/* If compiling on windows */
#ifdef _WIN32

char * readline( char *prompt );

static char buffer[2048]; //2^10
/* Static global variable is only seen within the file it's declared in  
  Static local variable can is saved between invocations
*/

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
#include <editline/readline.h> /* Linux: sudo apt-get install libedit -dev */
#include <editline/history.h>
#endif

/* Use operator strings to see which operation to performs */
long eval_op(long x, char* op, long y) {
  if(strcmp(op, "+") == 0 ) { return x + y; }
  if(strcmp(op, "-") == 0 ) { return x - y; }
  if(strcmp(op, "*") == 0 ) { return x * y; }
  if(strcmp(op, "/") == 0 ) { return x / y; }
  
  return 0;
}

long eval(mpc_ast_t* t) {
  
  /* If tagged as number return directly */
  if( strstr(t->tag, "number")){
    return atoi(t->contents);
  }

  /* The operator is always the second child. */
  char *op = t->children[1]->contents;

  /* We store the third child in 'x' */
  long x = eval(t->children[2]); /*recursive step */

  /* Iterate the remaining children and combining. */
  int i = 3;
  while( strstr( t->children[i]->tag, "expr" ) ) {
    x = eval_op(x, op, eval(t->children[i])); //evaluate the operation at the trees child [i]
    i++;
  }

  return x;


}


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

	 char* input = readline("JLISP> ");
    add_history(input);
    
    /* Attempt to parse the user input */
    mpc_result_t r;
    if (mpc_parse("<stdin>", input, Lispy, &r)) {
      
      long result = eval(r.output);
      printf( "%li\n\n" , result );
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