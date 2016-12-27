default:
	gcc -pthread -lmcrypt client.c -o client
	gcc -pthread -lmcrypt server.c -o server

clean:
	rm -rf client server *.txt *~ *.tar.gz

dist:
	tar -cvzf lab1b-904581627.tar.gz client.c server.c README my.key Makefile 
