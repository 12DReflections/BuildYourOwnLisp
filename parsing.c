/* cc -std=c99 -Wall *.c -o parsing */
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

/* Create Enumeration of Possible lval Types */
enum { LVAL_NUM, LVAL_ERR, LVAL_SYM, LVAL_SEXPR, LVAL_QEXPR };

/* Lval struct of either num or error */
typedef struct lval {
  int type;
  long num;
  char* err; /* Error and Symbol types as strings */
  char* sym;
  int count;
  struct lval** cell; /* pointer to a list of LVALS */
  
} lval;


/* Create number LVAL, assign pointer and values, return it */
lval* lval_num(long x) {
  lval* v = malloc(sizeof(lval));
  v->type = LVAL_NUM;
  v->num = x;
  return v;
}

/* Create an LVAL error, malloc, assign variables and error, return */
lval* lval_err(char* m) {
  lval* v = malloc(sizeof(lval));
  v->type = LVAL_ERR;
  v->err = malloc(strlen(m)+1);
  strcpy(v->err, m);
  return v;
}

/* A pointer to a symbol */
lval* lval_sym(char* s){
	lval* v = malloc(sizeof(lval));
	v->type = LVAL_SYM;
	v->sym = malloc(strlen(s) +1);
	strcpy(v->sym, s);
	return v;
}

/* A pointer to an LVAL S-Expression (list) */
lval* lval_sexpr(void){
	lval* v = malloc(sizeof(lval));
	v->type = LVAL_SEXPR;
	v->count = 0;
	v->cell = NULL;
	return v;
}

/* A pointer to a new empty Qexpr for data reads */
lval* lval_qexpr(void) {
	lval* v = malloc(sizeof(lval));
	v->type = LVAL_QEXPR;
	v->count = 0;
	v->cell = NULL;
	return v;
	
}

void lval_del(lval* v){
	
	switch(v->type){
		/* Do nothing special for number type */
		case LVAL_NUM: break;
		/* For Err or Symbol, free the string data */
		case LVAL_ERR: free(v->err); break;
		case LVAL_SYM: free(v->sym); break;
		
		/* If Sexpr or Qexpr then delete all elements inside */
		/* Also free memory allocated to contain the pointers */
		case LVAL_QEXPR:		
		case LVAL_SEXPR:
			for(int i = 0; i < v->count; i++) {
				lval_del(v->cell[i]);
			}
			free(v->cell);
		break;
	}
	
	/* Free the "lval" struct itself */
	free(v);		
}

lval* lval_add(lval* v, lval* x){
	v->count++;
	v->cell = realloc(v->cell, sizeof(lval*) * v->count);
	v->cell[v->count-1] = x;
	return v;
}

lval* lval_pop(lval* v, int i) {
	/* Find the item at "i" */
	lval* x = v->cell[i];
	
	/* Shift memory after the item at "i" over the top */
	memmove(&v->cell[i], &v->cell[i+1],
				sizeof(lval*) * (v->count-i-1));
				
	/* Decrease the count of items in the list */
	v->count--;
	
	/* Reallocate the memory used */
	v->cell = realloc(v->cell, sizeof(lval*) * v->count);
	return x;
}

lval* lval_join(lval* x, lval* y) {
	while(y->count) {
		x = lval_add(x, lval_pop(y, 0));
	}
	
	lval_del(y);
	return x;
}

lval* lval_take(lval* v, int i) {
	lval* x = lval_pop(v, i);
	lval_del(v);
	return x;
}

void lval_print(lval *v); /* preprocess */
void lval_expr_print(lval* v, char open, char close){
	putchar(open);
	for(int i = 0; i< v->count; i++){
		
		/* Print Value contained within */
		lval_print(v->cell[i]);
		
		/* Don't print trailing space if last element. */
		if(i != (v->count-1)){
			putchar(' ');
		}
	}
	putchar(close);
}

/* Print an lval */
void lval_print(lval* v) {
  switch (v->type) {
    /* print if number, error, symbol or list expression */
    case LVAL_NUM: printf("%li", v->num); break;
    case LVAL_ERR: printf("Error: %s", v->err); break;
	case LVAL_SYM: printf("%s", v->sym); break;
	case LVAL_SEXPR: lval_expr_print(v, '(', ')'); break;
	case LVAL_QEXPR: lval_expr_print(v, '{', '}'); break;
  }
}

/* Print an "lval" followed by a new line */
void lval_println(lval* v) { lval_print(v); putchar('\n'); }

#define LASSERT(args, cond, err) \
	if (!(cond)) { lval_del(args); return lval_err(err); }
	
lval* lval_eval(lval* v);

lval* builtin_list(lval* a) {
	a->type = LVAL_QEXPR;
	return a;
}

lval* builtin_head(lval* a) {
  LASSERT(a, a->count == 1,
    "Function 'head' passed too many arguments.");
  LASSERT(a, a->cell[0]->type == LVAL_QEXPR,
    "Function 'head' passed incorrect type.");
  LASSERT(a, a->cell[0]->count != 0,
    "Function 'head' passed {}.");
	
	lval* v = lval_take(a,0);
	while (v->count > 1) { lval_del(lval_pop(v,1));}
	return v;
}

lval* builtin_tail(lval* a) {
  LASSERT(a, a->count == 1,
    "Function 'tail' passed too many arguments.");
  LASSERT(a, a->cell[0]->type == LVAL_QEXPR,
    "Function 'tail' passed incorrect type.");
  LASSERT(a, a->cell[0]->count != 0,
    "Function 'tail' passed {}.");

  lval* v = lval_take(a, 0);  
  lval_del(lval_pop(v, 0));
  return v;
}

lval* builtin_eval(lval* a) {
  LASSERT(a, a->count == 1,
    "Function 'eval' passed too many arguments.");
  LASSERT(a, a->cell[0]->type == LVAL_QEXPR,
    "Function 'eval' passed incorrect type.");
  
  lval* x = lval_take(a, 0);
  x->type = LVAL_SEXPR;
  return lval_eval(x);
}

lval* builtin_join(lval* a) {

  for (int i = 0; i < a->count; i++) {
    LASSERT(a, a->cell[i]->type == LVAL_QEXPR,
      "Function 'join' passed incorrect type.");
  }
  
  lval* x = lval_pop(a, 0);
  
  while (a->count) {
    x = lval_join(x, lval_pop(a, 0));
  }
  
  lval_del(a);
  return x;
}


/* Use operator strings to see which operation to performs */
lval* builtin_op(lval* a, char* op) {

	for(int i = 0; i < a->count; i++){
		if(a->cell[i]->type != LVAL_NUM){
			lval_del(a);
			return lval_err("Cannot operate on non-number");
		}
	}
	
	/* Pop the first element */
	lval* x = lval_pop(a, 0);
	
	/* If no arguments and subtraction then perform unary negation */
	if((strcmp(op, "-") == 0) && a->count == 0){
		x->num = -x->num;
	}

	/* While there are still elements remaining */
	while(a->count > 0) {
		lval* y = lval_pop(a, 0);
	
		if(strcmp(op, "+") == 0 ) { lval_num(x->num += y->num); }
		if(strcmp(op, "-") == 0 ) { lval_num(x->num -= y->num); }
		if(strcmp(op, "*") == 0 ) { lval_num(x->num *= y->num); }
		if(strcmp(op, "/") == 0 ) { 
			if(y->num == 0){
				lval_del(x); lval_del(y);
				x = lval_err("Division by Zero");
			}
			else x->num /= y->num;
		}
		lval_del(y);
	}
	
	lval_del(a); 
	return x;
}

lval* builtin(lval* a, char* func) {
  if (strcmp("list", func) == 0) { return builtin_list(a); }
  if (strcmp("head", func) == 0) { return builtin_head(a); }
  if (strcmp("tail", func) == 0) { return builtin_tail(a); }
  if (strcmp("join", func) == 0) { return builtin_join(a); }
  if (strcmp("eval", func) == 0) { return builtin_eval(a); }
  if (strstr("+-/*", func)) { return builtin_op(a, func); }
  lval_del(a);
  return lval_err("Unknown Function");
}

lval* lval_eval(lval* v);
lval* lval_eval_sexpr(lval*v){
	/* Evaluate Children */
	for(int i = 0; i < v->count; i++){
		v->cell[i] = lval_eval(v->cell[i]);
	}
	
	/* Error Checking */
	for(int i = 0; i < v->count; i++){
		if( v->cell[i]->type == LVAL_ERR) { return lval_take(v, i); }
	}
	
	/* Empty Expression */
	if(v->count == 0 ) { return v;}
	
	/* Single Expression */
	if(v->count == 1) 	{ return lval_take(v, 0); }
	
	/* Ensure first element is a Symbol */
	lval* f = lval_pop(v,0);
	if(f->type != LVAL_SYM) {
		lval_del(f); lval_del(v);
		return lval_err("S-expression does not start with a symbol");
	}
	
	/* Call built in with operator */
	lval* result = builtin(v, f->sym);
	lval_del(f);
	return result;
}

lval* lval_eval(lval* v) {
	/* Evaluate S-Expression */
	if(v->type == LVAL_SEXPR) {return lval_eval_sexpr(v); }
	/* All other lval types remain the same */
	return v;
}

lval* lval_read_num(mpc_ast_t* t) {
	errno = 0;
	long x = strtol(t->contents, NULL, 10);
	return errno != ERANGE ? lval_num(x) : lval_err("invalid number\n");
}

lval* lval_read(mpc_ast_t* t){
	
	/* If Symbol Or Number return conversion to that type  */
	if(strstr(t-> tag, "number")) { return lval_read_num(t); }
	if(strstr(t-> tag, "symbol")) { return lval_sym(t->contents); }
	
	/* If root (>) or sexpr  the create empty list */
	lval* x = NULL;
	if(strcmp(t->tag, ">") == 0) { x = lval_sexpr(); }
	if(strstr(t->tag, "sexpr"))  { x = lval_sexpr(); }
	if(strstr(t->tag, "qexpr"))  { x = lval_qexpr(); }

	
	/* Fill this list with any valid expression contained within */
	for(int i = 0; i < t->children_num; i++){
		if( strcmp( t->children[i]->contents, "(") == 0) { continue; }
		if( strcmp( t->children[i]->contents, ")") == 0) { continue; }
		if( strcmp( t->children[i]->contents, "{") == 0) { continue; }
		if( strcmp( t->children[i]->contents, "}") == 0) { continue; }
		if( strcmp( t->children[i]->tag,  "regex") == 0) { continue; }
		x = lval_add(x, lval_read(t->children[i]));
	}
	return x;
}


int main(int argc, char ** argv) {

	  /* Create Some Parsers */
  mpc_parser_t* Number   = mpc_new("number");
  mpc_parser_t* Symbol	 = mpc_new("symbol");
  mpc_parser_t* Sexpr 	 = mpc_new("sexpr");
  mpc_parser_t* Qexpr  = mpc_new("qexpr");
  mpc_parser_t* Expr     = mpc_new("expr");
  mpc_parser_t* Lispy    = mpc_new("lispy");
  
  /* Define them with the following Language */
  mpca_lang(MPCA_LANG_DEFAULT,
    "                                                    \
      number : /-?[0-9]+/ ;                              \
      symbol : \"list\" | \"head\" | \"tail\" | \"eval\" \
             | \"join\" | '+' | '-' | '*' | '/' ;        \
      sexpr  : '(' <expr>* ')' ;                         \
      qexpr  : '{' <expr>* '}' ;                         \
      expr   : <number> | <symbol> | <sexpr> | <qexpr> ; \
      lispy  : /^/ <expr>* /$/ ;                         \
    ",
    Number, Symbol, Sexpr, Qexpr, Expr, Lispy);


	printf("JLISP Version 0.0.0.0.1\n");
	printf("\tEnter Ctrl+c to exit\n\n");

	while( 1 ){

	 char* input = readline("JLISP> ");
     add_history(input);
    
    /* Attempt to parse the user input */
    mpc_result_t r;
    if (mpc_parse("<stdin>", input, Lispy, &r)) {
      lval* x = lval_eval(lval_read(r.output)); 
      lval_println(x);
	  lval_del(x);
	  mpc_ast_delete(r.output);
	  
    } else {
      /* Otherwise print and delete the Error */
      mpc_err_print(r.error);
      mpc_err_delete(r.error);
    }
    
    free(input);
  }
  
  /* Undefine and delete parser */
  mpc_cleanup(6, Number, Symbol, Sexpr, Qexpr, Expr, Lispy);
  
  return 0;
}