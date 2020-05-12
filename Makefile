# PSIS 2019-2020
# Makefile for pacman
#-----------------------------------------------------------------------
#compile commands
server:
	gcc server.c UI_library.c -o $@ -lpthread -lSDL2 -lSDL2_image -Wall

client:
	gcc client.c UI_library.c -o $@ -lpthread -lSDL2 -lSDL2_image -Wall

FILES = $(shell ls *.txt)
# test commands
ts:
	for F in ${FILES}; do  ./server $${F} ; done;

tc:
	./client $A $B

#clear executable
cleans::
	rm -f *.o core a.out server gv *~

cleanc::
	rm -f *.o core a.out client gv *~
