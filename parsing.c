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

/* Create Enumeration of Possible Error Types */
enum { LERR_DIV_ZERO, LERR_BAD_OP, LERR_BAD_NUM };

/* Create Enumeration of Possible lval Types */
enum { LVAL_NUM, LVAL_ERR };

/* Lval struct of either num or error */
typedef struct {
  int type;
  long num;
  int err;
} lval;


/* lval !Error Struct */
lval lval_num(long x) {
  lval v;
  v.type = LVAL_NUM;
  v.num = x;
  return v;
}

/* lval Error Struct */
lval lval_err(int x) {
  lval v;
  v.type = LVAL_ERR;
  v.err = x;
  return v;
}

/* Print an lval */
void lval_print(lval v) {
  switch (v.type) {
    /* print if number */
    case LVAL_NUM: printf("%li", v.num); break;

    /* if type is error */
    case LVAL_ERR:
      /* Check and print error type */
      if(v.err == LERR_DIV_ZERO) {
        printf("Error: Divide by Zero");
      }
      if(v.err == LERR_BAD_NUM) {
        printf("Invalid operator");
      }
      if(v.err == LERR_BAD_NUM) {
        printf("Error: Invalid Number");
      }
    break;
  }
}

/* Print an "lval" followed by a new line */
void lval_println(lval v) { lval_print(v); putchar('\n'); }



/* Use operator strings to see which operation to performs */
lval eval_op(lval x, char* op, lval y) {

  /* If either value is an error return it */
  if(x.type == LVAL_ERR) {return x; }
  if(y.type == LVAL_ERR) {return y; } 

  /* Else perform its operation */
  if(strcmp(op, "+") == 0 ) { return lval_num(x.num + y.num); }
  if(strcmp(op, "-") == 0 ) { return lval_num(x.num - y.num); }
  if(strcmp(op, "*") == 0 ) { return lval_num(x.num * y.num); }
  if(strcmp(op, "/") == 0 ) { 
    /* if second number is zero return error, else return result */
    return y.num == 0 ? lval_err(LERR_DIV_ZERO) 
      : lval_num(x.num / y.num ); 
  }
  
  return lval_err(LERR_BAD_OP);
}

lval eval(mpc_ast_t* t) {
  
  /* Test for error in evaluation, return if none, else return error */
  if( strstr(t->tag, "number")){
    errno = 0;
    long x = strtol(t->contents, NULL, 10);
    return errno != ERANGE ? lval_num(x) : lval_err(LERR_BAD_NUM);
  }

  /* The operator is always the second child. */
  char *op = t->children[1]->contents;

  /* We store the third child in 'x' */
  lval x = eval(t->children[2]); /*recursive step */

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
      
      lval result = eval(r.output);
      lval_println(result);
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