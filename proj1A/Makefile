SHELL = /bin/bash

default: lab1a.c
	gcc -g -pthread lab1a.c -o lab1a

clean:
	@rm -rf lab1a *.o *.tar.gz *~

dist: Makefile lab1a.c README
	@tar -cvzf lab1a-904581627.tar.gz README Makefile lab1a.c
