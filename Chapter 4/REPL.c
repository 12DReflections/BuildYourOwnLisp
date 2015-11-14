#include <stdio.h>

int main(int arc, char ** argv) {

	/* Static global variable is only seen within the file it's declared in  
		Static local variable can is saved between invocations
		size(input) = 2048 = 2^10 
	*/
	static char input[2048];
	
	printf("JLISP Version 0.0.0.0.1\n");
	printf("\tEnter Ctrl+c to exit\n\t(or Ctrl+d on LINUX)\n\n");

	while( *input != EOF){

		/* Print and don't append newline */
		fputs("JLisp> ", stdout);

		fgets(input, sizeof(input), stdin);
		printf("No you're a %s", input);

	}

	return 0;
} 