########################################################################
# JLISP MAKEFILE
# Made by Julian Wise
########################################################################
CC = gcc
CFLAGS = -std=c99 -Wall
USER = Julian

all: parsing

parsing: parsing.o mpc.o
	$(CC) parsing.o mpc.o -o parsing

#targets #dependancy
parsing.o: parsing.c
	$(CC) $(CFLAGS) parsing.c

mpc.o: mpc.c
	$(CC) $(CFLAGS) mpc.c

clean: 
	rm -rf *o ppd

SOURCES=parsing.c mpc.c 
HEADERS=mpc.h
README=ppd_readme
MAKEFILE=Makefile

########################################################################
# Archiving the code for portability
######################################################################
archive:
	zip $(USER)-a2 $(SOURCES) $(HEADERS) $(README) $(MAKEFILE)
