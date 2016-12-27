#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <mraa/aio.h>
#include <math.h>

const int B=4275; 
sig_atomic_t volatile run_flag = 1;

void do_when_interrupted(int sig) {
	if (sig == SIGINT)
	run_flag = 0;
}

int main() {
	uint16_t value;
	// declare rotary as an analog I/O context
	mraa_aio_context temperature;
	temperature = mraa_aio_init(0);

	//write in a log file
	FILE *f;
	f = fopen("Part1_log", "w");

	while(run_flag) {
	// read the rotary sensor value
		value = mraa_aio_read(temperature);
   		float R = 1023.0/((float)value)-1.0;
    	R = 100000.0*R;

    	//time
    	time_t timer;
	    char buffer[10];
	    struct tm* tm_info;
	    time(&timer);
	    tm_info = localtime(&timer);
	    strftime(buffer, 10, "%H:%M:%S", tm_info);

	    //temparature
    	float temperature=1.0/(log(R/100000.0)/B+1/298.15)-273.15;//convert to temperature via datasheet ;
    	temperature = temperature*1.8+32;

		printf("%s ", buffer);
		printf("%0.1f\n", temperature);

		//print to log
		fprintf(f, "%s ", buffer);
		fprintf(f, "%0.1f\n", temperature);
		fflush(f);

		sleep(1);
	}

	mraa_aio_close(temperature);
	fclose(f);

	return 0;
}

