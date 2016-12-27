default: lab2_add.c lab2_list.o SortedList.o
	gcc -g -Wall -pthread lab2_add.c -o lab2_add
	gcc -g -Wall -pthread lab2_list.o SortedList.o -o lab2_list
	-./testadd.sh
	-./testlist.sh 

lab2_list.o: lab2_list.c
	gcc -g -Wall -pthread -c lab2_list.c -o lab2_list.o

SortedList.o:SortedList.c
	gcc -g -Wall -pthread -c SortedList.c -o SortedList.o

clean:
	rm -rf lab2_add lab2_list *.o *.tar.gz *~ *.csv *.png

dist:
	tar -cvzf lab2a-904581627.tar.gz README.txt Makefile lab2_add.c lab2_list.c SortedList.c SortedList.h *.png *.sh *.csv
