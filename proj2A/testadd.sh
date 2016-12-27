#!/bin/bash

#question1
# range of threads and iterations to see what it takes to cause a failure
./lab2_add --threads=2 --iterations=10 >> lab2_add.csv
./lab2_add --threads=2 --iterations=100 >> lab2_add.csv
./lab2_add --threads=2 --iterations=1000 >> lab2_add.csv
./lab2_add --threads=2 --iterations=10000 >> lab2_add.csv
./lab2_add --threads=2 --iterations=100000 >> lab2_add.csv
./lab2_add --threads=4 --iterations=10 >> lab2_add.csv
./lab2_add --threads=4 --iterations=100 >> lab2_add.csv
./lab2_add --threads=4 --iterations=1000 >> lab2_add.csv
./lab2_add --threads=4 --iterations=10000 >> lab2_add.csv
./lab2_add --threads=4 --iterations=100000 >> lab2_add.csv
./lab2_add --threads=8 --iterations=10 >> lab2_add.csv
./lab2_add --threads=8 --iterations=100 >> lab2_add.csv
./lab2_add --threads=8 --iterations=1000 >> lab2_add.csv
./lab2_add --threads=8 --iterations=10000 >> lab2_add.csv
./lab2_add --threads=8 --iterations=100000 >> lab2_add.csv
./lab2_add --threads=12 --iterations=10 >> lab2_add.csv
./lab2_add --threads=12 --iterations=100 >> lab2_add.csv
./lab2_add --threads=12 --iterations=1000 >> lab2_add.csv
./lab2_add --threads=12 --iterations=10000 >> lab2_add.csv
./lab2_add --threads=12 --iterations=100000 >> lab2_add.csv
# range of threads and iterations to cause failure with yield
./lab2_add --threads=2 --iterations=10 --yield >> lab2_add.csv
./lab2_add --threads=4 --iterations=10 --yield >> lab2_add.csv
./lab2_add --threads=8 --iterations=10 --yield >> lab2_add.csv
./lab2_add --threads=12 --iterations=10 --yield >> lab2_add.csv
./lab2_add --threads=2 --iterations=100 --yield >> lab2_add.csv
./lab2_add --threads=4 --iterations=100 --yield >> lab2_add.csv
./lab2_add --threads=8 --iterations=100 --yield >> lab2_add.csv
./lab2_add --threads=12 --iterations=100 --yield >> lab2_add.csv
./lab2_add --threads=2 --iterations=1000 --yield >> lab2_add.csv
./lab2_add --threads=4 --iterations=1000 --yield >> lab2_add.csv
./lab2_add --threads=8 --iterations=1000 --yield >> lab2_add.csv
./lab2_add --threads=12 --iterations=1000 --yield >> lab2_add.csv
./lab2_add --threads=2 --iterations=10000 --yield >> lab2_add.csv
./lab2_add --threads=4 --iterations=10000 --yield >> lab2_add.csv
./lab2_add --threads=8 --iterations=10000 --yield >> lab2_add.csv
./lab2_add --threads=12 --iterations=10000 --yield >> lab2_add.csv
./lab2_add --threads=2 --iterations=100000 --yield >> lab2_add.csv
./lab2_add --threads=4 --iterations=100000 --yield >> lab2_add.csv
./lab2_add --threads=8 --iterations=100000 --yield >> lab2_add.csv
./lab2_add --threads=12 --iterations=100000 --yield >> lab2_add.csv

#question2
# look at cost per operation vs number of iterations
./lab2_add --threads=1 --iterations=100 >> lab2_add.csv
./lab2_add --threads=1 --iterations=1000 >> lab2_add.csv
./lab2_add --threads=1 --iterations=10000 >> lab2_add.csv
./lab2_add --threads=1 --iterations=100000 >> lab2_add.csv
./lab2_add --threads=1 --iterations=1000000 >> lab2_add.csv
./lab2_add --threads=2 --iterations=100 >> lab2_add.csv
./lab2_add --threads=2 --iterations=1000 >> lab2_add.csv
./lab2_add --threads=2 --iterations=10000 >> lab2_add.csv
./lab2_add --threads=2 --iterations=100000 >> lab2_add.csv
./lab2_add --threads=2 --iterations=1000000 >> lab2_add.csv
./lab2_add --threads=4 --iterations=100 >> lab2_add.csv
./lab2_add --threads=4 --iterations=1000 >> lab2_add.csv
./lab2_add --threads=4 --iterations=10000 >> lab2_add.csv
./lab2_add --threads=4 --iterations=100000 >> lab2_add.csv
./lab2_add --threads=4 --iterations=1000000 >> lab2_add.csv

./lab2_add --threads=1 --iterations=100 --yield >> lab2_add.csv
./lab2_add --threads=1 --iterations=1000 --yield >> lab2_add.csv
./lab2_add --threads=1 --iterations=10000 --yield >> lab2_add.csv
./lab2_add --threads=1 --iterations=100000 --yield >> lab2_add.csv
./lab2_add --threads=1 --iterations=1000000 --yield >> lab2_add.csv
./lab2_add --threads=2 --iterations=100 --yield >> lab2_add.csv
./lab2_add --threads=2 --iterations=1000 --yield >> lab2_add.csv
./lab2_add --threads=2 --iterations=10000 --yield >> lab2_add.csv
./lab2_add --threads=2 --iterations=100000 --yield >> lab2_add.csv
./lab2_add --threads=2 --iterations=1000000 --yield >> lab2_add.csv
./lab2_add --threads=4 --iterations=100 --yield >> lab2_add.csv
./lab2_add --threads=4 --iterations=1000 --yield >> lab2_add.csv
./lab2_add --threads=4 --iterations=10000 --yield >> lab2_add.csv
./lab2_add --threads=4 --iterations=100000 --yield >> lab2_add.csv
./lab2_add --threads=4 --iterations=1000000 --yield >> lab2_add.csv

#3  Average time per (single threaded) operation vs. the number of iterations.
./lab2_add --threads=1 --iterations=100  >> lab2_add.csv
./lab2_add --threads=1 --iterations=1000  >> lab2_add.csv
./lab2_add --threads=1 --iterations=10000  >> lab2_add.csv
./lab2_add --threads=1 --iterations=100000  >> lab2_add.csv

#question4: threads and iterations that can run successfully with yields under each of the three synchronization methods.
# demonstrate the efficacy of each of the protection mechanisms
./lab2_add --threads=2 --iterations=10000 --yield --sync=m  >> lab2_add.csv
./lab2_add --threads=4 --iterations=10000 --yield --sync=m  >> lab2_add.csv
./lab2_add --threads=8 --iterations=10000 --yield --sync=m  >> lab2_add.csv
./lab2_add --threads=12 --iterations=10000 --yield --sync=m  >> lab2_add.csv
./lab2_add --threads=2 --iterations=10000 --yield --sync=c  >> lab2_add.csv
./lab2_add --threads=4 --iterations=10000 --yield --sync=c  >> lab2_add.csv
./lab2_add --threads=8 --iterations=10000 --yield --sync=c  >> lab2_add.csv
./lab2_add --threads=12 --iterations=10000 --yield --sync=c  >> lab2_add.csv
./lab2_add --threads=2 --iterations=10000 --yield --sync=s  >> lab2_add.csv
./lab2_add --threads=4 --iterations=10000 --yield --sync=s  >> lab2_add.csv
./lab2_add --threads=8 --iterations=1000 --yield --sync=s  >> lab2_add.csv
./lab2_add --threads=12 --iterations=1000 --yield --sync=s  >> lab2_add.csv

#5 Average time per (multi-threaded) operation vs. the number of threads, for all four versions of the add function.
# generate time per operation numbers
# ./lab2_add --threads=1 --iterations=10000  >> lab2_add.csv
# ./lab2_add --threads=2 --iterations=10000  >> lab2_add.csv
# ./lab2_add --threads=4 --iterations=10000  >> lab2_add.csv
# ./lab2_add --threads=8 --iterations=10000  >> lab2_add.csv
# ./lab2_add --threads=12 --iterations=10000 >> lab2_add.csv
./lab2_add --threads=1 --iterations=10000 --sync=m  >> lab2_add.csv
./lab2_add --threads=2 --iterations=10000 --sync=m  >> lab2_add.csv
./lab2_add --threads=4 --iterations=10000 --sync=m  >> lab2_add.csv
./lab2_add --threads=8 --iterations=10000 --sync=m  >> lab2_add.csv
./lab2_add --threads=12 --iterations=10000 --sync=m  >> lab2_add.csv
./lab2_add --threads=1 --iterations=10000 --sync=c  >> lab2_add.csv
./lab2_add --threads=2 --iterations=10000 --sync=c  >> lab2_add.csv
./lab2_add --threads=4 --iterations=10000 --sync=c  >> lab2_add.csv
./lab2_add --threads=8 --iterations=10000 --sync=c  >> lab2_add.csv
./lab2_add --threads=12 --iterations=10000 --sync=c  >> lab2_add.csv
./lab2_add --threads=1 --iterations=10000 --sync=s  >> lab2_add.csv
./lab2_add --threads=2 --iterations=10000 --sync=s  >> lab2_add.csv
./lab2_add --threads=4 --iterations=10000 --sync=s  >> lab2_add.csv
./lab2_add --threads=8 --iterations=10000 --sync=s  >> lab2_add.csv
./lab2_add --threads=12 --iterations=10000 --sync=s  >> lab2_add.csv

/u/cs/grad/zhou/iloveos/gnuplot lab2_add.gp lab2_add.csv
