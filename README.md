# BuildYourOwnLisp
Building a LISP Compiler in C

This my attempt and building a LISP Compiler in C named JLISP.
The core structure of JLISP will be derived from Daniel Holden's book 
"Build your own LISP"
http://www.buildyourownlisp.com/

To run in Linux install libedit
sudo apt-get install libedit -dev


To compile:
	On Windows
		gcc -std=c99 -Wall parsing.c mpc.c parsing.h -o parsing

	On Linux and Mac
		cc -std=c99 -Wall parsing.c mpc.c parsing.h -ledit -lm -o parsing
