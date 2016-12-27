default: lab2_list.o SortedList.o
	gcc -g -Wall -pthread lab2_list.o SortedList.o -o lab2_list
	gcc -o $@ -pthread -L /u/cs/ugrad/yinxue/lib -lprofiler lab2_list.o SortedList.o
	-./testlist.sh

lab2_list.o: lab2_list.c
	gcc -g -Wall -pthread -c lab2_list.c -o lab2_list.o

SortedList.o:SortedList.c
	gcc -g -Wall -pthread -c SortedList.c -o SortedList.o

profile.gperf: lab2_list Makefile
	-rm -f ./raw.gperf
	LD_PRELOAD=/u/cs/ugrad/yinxue/lib/libprofiler.so.0 CPUPROFILE=./raw.gperf ./lab2_list --threads=12 --iterations=1000 --sync=s
	pprof --text ./lab2_list ./raw.gperf >> profile.gperf
	pprof --list=my_thread ./lab2_list ./raw.gperf >> profile.gperf
	rm -f ./raw.gperf        

clean:
	rm -rf lab2_list *.o *.tar.gz *~ lab_2b_list.csv *.png profile.gperf

dist:
	tar -cvzf lab2b-904581627.tar.gz README.txt Makefile lab2_list.c SortedList.c SortedList.h *.png *.sh *.csv profile.gperf
