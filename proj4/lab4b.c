#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <mraa/aio.h>
#include <math.h>
#include <time.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>

const int B=4275; 
sig_atomic_t volatile run_flag = 1;

void do_when_interrupted(int sig) {
	if (sig == SIGINT)
	run_flag = 0;
}

int main() {

	/* Create a socket point */
	/////////////////////////////////////////////////
	char buffer[100] = "Port request 904581627";
	char *bufptr = buffer;
	int buflen = sizeof(buffer);
	int port_num;

	struct sockaddr_in serv_addr;
    struct hostent *server;
    int sockfd;
    int rc = sizeof(int);
    int serveraddrlen = sizeof(serv_addr);

    //connect server the first time to get the port number -----------------------
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) {
	    perror("ERROR opening socket");
	    exit(1);
	}
		
	server = gethostbyname("lever.cs.ucla.edu");
	if (server == NULL) {
	    fprintf(stderr,"ERROR, no such host\n");
	    exit(0);
	}
	   
	memset((char *) &serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	memcpy((char *)&serv_addr.sin_addr.s_addr, (char *)server->h_addr, server->h_length);
	serv_addr.sin_port = htons(16000);
	   
	/* Now connect to the server */
	if (connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
	    perror("ERROR connecting");
	    exit(1);
	}

	//send message
	dprintf(sockfd,"Port request 904581627");
	 
	//receive new port number
	rc = read(sockfd, &port_num, sizeof(int)); 
	if(rc < 0)
	{
		perror("Error receiving the port number from the server");
		close(sockfd);
		exit(-1);
	}
	else
	{
		printf("Port number is %d\n", port_num);
	}

	close(sockfd);

	//now reconnect to the server for the new port number---------------------------
	sockfd = socket(AF_INET, SOCK_STREAM, 0);   
	if (sockfd < 0) {
	    perror("ERROR opening socket");
	    exit(1);
	}
		
	server = gethostbyname("lever.cs.ucla.edu");	   
	if (server == NULL) {
	    fprintf(stderr,"ERROR, no such host\n");
	    exit(0);
	}
	   
	memset((char *) &serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	memcpy((char *)&serv_addr.sin_addr.s_addr, (char *)server->h_addr, server->h_length);
	serv_addr.sin_port = htons(port_num);
	   
	/* Now connect to the server */
	if (connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
	    perror("ERROR connecting");
	    exit(1);
	}

	fcntl(sockfd, F_SETFL, O_NONBLOCK);

	//temparature read
	uint16_t value;
	// declare rotary as an analog I/O context
	mraa_aio_context temperature;
	temperature = mraa_aio_init(0);

	//write in a log file
	FILE *f;
	f = fopen("Part2_log", "w");

	int stop_flag = 0;
	int x = 3;
	int scale = 0;

	while(run_flag) {
		int invalid_flag = 0;

		//receive command
		char c[10];
	  	int size = 0;
	  	while((size = read(sockfd,&c,sizeof(c))) > 0)
		{
			if (strncmp(c,"OFF",10) == 0)          
		    {
		    	exit(0);
		    }
		    else if (strncmp(c,"STOP",10) == 0) 
		    {
		    	stop_flag = 1;
		    } 
		    else if (strncmp(c,"START",10) == 0) 
		    {
		    	if(stop_flag == 1)
		    		stop_flag = 0;
		    }
		    else if (strncmp(c,"SCALE=F",7) == 0) 
		    {
		    	scale = 0;
		    }
		   	else if (strncmp(c,"SCALE=C",7) == 0) 
		    {
		    	scale = 1;
		    }
		    else if(strncmp(c,"FREQ=",5) == 0)
		    {
		    	x = atoi(c+5);
		    	if(x>3600 || x<1)
		    		invalid_flag = 1;
		    }
		    else
		    	invalid_flag = 1;
		    if(invalid_flag)
		    {
		    	printf("%s I\n", c);
		    	fprintf(f,"%s I\n", c);
		    	fflush(f);
		    }
		    else
		    {
				printf("%s\n", c);
				fprintf(f,"%s\n", c);
				fflush(f);
		    }
		}	
		memset(c,0,sizeof(c));

	//read the rotary sensor value
		if(!stop_flag)
		{
			value = mraa_aio_read(temperature);
	   		float R = 1023.0/((float)value)-1.0;
	    	R = 100000.0*R;

	    	time_t timer;
		    char buffer[10];
		    struct tm* tm_info;
		    time(&timer);
		    tm_info = localtime(&timer);
		    strftime(buffer, 10, "%H:%M:%S", tm_info);

		    //temparature
	    	float temp=1.0/(log(R/100000.0)/B+1/298.15)-273.15;//convert to temperature via datasheet ;
	    	if(scale == 0)
	    		temp = temp*1.8+32;

			printf("%s ", buffer);
			fprintf(f,"%s ", buffer);
			fflush(f);
			if(scale == 1)
			{
				printf("%0.1f C\n", temp);
				fprintf(f,"%0.1f C\n", temp);
				fflush(f);
			}
			else
			{
				printf("%0.1f F\n", temp);
				fprintf(f,"%0.1f F\n", temp);
				fflush(f);
			}

			printf("904581627 TEMP=%0.1f\n", temp);
			dprintf(sockfd,"904581627 TEMP=%0.1f\n", temp);

			sleep(x);
		}

	}

	mraa_aio_close(temperature);
	fclose(f);

	close(sockfd);
	return 0;
}

