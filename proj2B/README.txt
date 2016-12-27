Files
Makefile
Builds lab2c, runs the test cases, creates the csv file, graph the data. Also cleans the directory and create the tarball.

lab2c.c
Similar to lab2a, we have several arguments as input. We add one more in this case, the --list option. We are able to divide the list into sublists and do the operations. We input the number of sublists we want the list to divide into.

SortedList.h
The header file that declares the functions in the SortedList.c file

SortedList.c
The implementation of the functions declared in the .h file. Includes insert, delete, length and lookup.

lab_2b_list.csv
The data we created from testlist.sh and we use this to for our graphs.

testlist.sh
The script written for the test cases. Makefile calls this script to perform test cases.

profile.gperf
The file we created using gperf tool to analyze performance.

graphs
The graphs we created for analysis from the list.csv file

Testing methodologies
I wrote a script to test the cases. I varied the thread number and iteration number, and added different options to test the cases. I also looked at the graphing requirements to add on to the testing part.

Answers
2.3.1
1)Most cycles are spent on the locks for both add and list. The actual critical section doesnt take long, but it cost time to do context switches.

2)Most of the time are being spent in the spinning part. When one thread acquires the lock and keeps running, the other threads have to spin and wait, and that cost a huge amount of time.

3)For list, scanning through the list takes a lot of time since its a complicated data structure. If there are a number of CPU, then lock takes more time since other cycles are all spinning.

2.3.2
1)From the execution profile we see that the lines where there are spin locks cost the most time. In the file we see line 74 and line 116 cost the most time, which is the part where we have spin wait(the while loop).

2)Once a thread runs on the critical section, the over threads have to wait. If we have a large number of threads, a large number of them have to spin wait for one process to finish, which will make it very expensive.

2.3.3
1)Because a thread spends more time on acquiring the lock then actually performing the critical section. If we have more threads, when a lock is occupied, more threads are waiting for it. It thus costs a lot of time.

2)Because the thread spend less time on the completion time peroperation. It instead spends more time on acquiring the lock.

3)Because as the number of threads increases, we have more threads waiting for a lock. Thus it costs more time for us to get a lock, and it slows down the entire process.

2.3.4
1)The performance gets better as the number of lists increases. The more sublists we have, the better performace we get as a result.

2)The throughput might not continue increasing because there should be a limit. The total number of elements remains the same, so as we increase the number of lists there might be memory contention and the throughput may be low.

3)Yes it is reasonable. Since it's faster to go over shorter list, we dont need that many threads to increase the performance. If we have a longer list, partitioning it into N sublists and use N threads makes it equivalent to running N short lists at the same time, which increases the performance.
