Included Files

Makefile
Builds lab2_add and lab2_list, runs the test cases, creates the csv file, and graph the data. Also cleans the directory, and create the tarball.

lab2_add.c
The code aims to test threads by adding them to a add/substract function. it takes four arguments: --iterations, --threads, --yield and --sync. Iteration means how manay times the single computation is performed. Thread means how many threads are running in parallel. Yield means when typing it then just move to the end of the run queue. Sync provides options such as m, s and c to use different locks.

lab2_list.c
The code aims to test threads through implementing a doubly-linked list. Similar to lab2_add.c, it takes four arguments: --iterations, --threads, --yield and --sync. For iterations and threads its the number entered or iteration number and number of threads. For yield we enter three options, i, l and d. For sync we enter m or s for different types of locks.

SortedList.h
The header file that declares the functions in the SortedList.c file

SortedList.c
The implementation of the functions declared in the .h file. Includes insert, delete, length and lookup.

png files
The png files created for visualization that compares the performance through different cases.

testlist.sh,testadd.sh
The script written for the test cases. Makefile calls these two scripts too perform test cases.

Testing methodology
I wrote a script to test the cases. I varied the thread number and iteration number, and added different options to test the cases. I also looked at the graphing requirements to add on to the testing part.

Answers
2.1.1 - causing conflicts:
      1)Why does it take many iterations before errors are seen?
Because there needs to be enough instances for the program to perform a context switch for the shared counter. We need a large number to increase the likelyhood to result in this behaviour.
	2)Why does a significantly smaller number of iterations so seldom fail?
Because for a smaller number of iterations the threads have less opportunity to update the shared counter.

2.1.2 - cost of yielding:
      It's slower because it has to call pthread_yield function. It moves the running thread to the end, and tells the next thread to run. The extra time is going to context switch for yielding.

2.1.3 - measurement errors:
      The average time drops because the time is distributed over more iterations. Since the iteration number increases, the denominator is bigger, so the average number decreases. We can remove the overhead expended at the beginning of the program. Since it allocates memory from the total time of the program before calculating the average cost per operation.

2.1.4 - costs of serialization:
      1)Because each thread has less opportunity to be blocked, each option spend similarly little amount of time being blocked.
      2)Because each thread has more opportunity to be blocked, so they spend time not executing. Thus they get to slow down.
      3)Because the running thread will continue spinning until it gets released. It doesnt go to sleep and let other threads to run. Thus it's so expensive.

2.2.1 -scalability of Mutex
      The variation in time per protected operation vs the number of threads is because for part 1 we are changing the value of a variable, but for part 2 we are changing the values of a data structure. Thus part 2 has more operations per step(ex. insert, addition). For part2, there are more complicated operations that leads to a greater possibility of interrupted by a context switch. Thus the threads spend more time being blocked.

2.2.2 - scalability of spin locks
      The variation in time per protected operation vs the number of threads for Mutex vs Spin locks differs because for spin it takes a long time to wait for the other threads, compared to the time spent on mutex. Since the operations in part2 includes a linked list, which is more complicated, it takes more time to process per operation, thus it spends more time on spin locks
