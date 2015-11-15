#include <stdio.h>
#include <stdlib.h>

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

	
	printf("JLISP Version 0.0.0.0.1\n");
	printf("\tEnter Ctrl+c to exit\n\n");

	while( 1 ){

		char *input = readline( "JLISP> ");
		add_history( input );
		printf( "No you're a %s\n", input );
		free( input );
	}

	return 0;
} 