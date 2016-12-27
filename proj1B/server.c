#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <getopt.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <assert.h>
#include <stdbool.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <netdb.h>
#include <netinet/in.h>
#include <string.h>
#include <mcrypt.h>

static int to_child_pipe[2];
static int from_child_pipe[2];
pid_t child_pid = -1;

static struct option long_options[]=
{
   	{"encrypt",no_argument,NULL,'e'},
   	{"port",required_argument,NULL,'p'}
};

static int port_flag    = 0;
static int port_number  = 0;
static int encrypt_flag = 0;
static int connected = 0;

MCRYPT td;
int sockfd;
int newsockfd;

int encrypt_message = 1; 
int decrypt_message = 0;
char key[100];

void exit_handler(int num)
{
	exit(num);
}

//----------------------------
int msg_crypt(char *message,char *ukey,int length,int action)
{
  /*Needed variables*/
  MCRYPT td;
  int i,size;
  char *key;
  char *IV; /*Initialization Vector*/
  int keysize = 16; /*256 bits*/

  key = calloc(1,keysize);
  /*Open the mcrypt library for work*/
  td = mcrypt_module_open("twofish",NULL,"cfb",NULL);
  if(td == MCRYPT_FAILED)
  {
    return MCRYPT_FAILED;
  }
  size = mcrypt_enc_get_iv_size(td);
  IV = malloc(size);
  for(i=0;i<size;i++)
  {
    IV[i] = 1;
  }
  memmove(key,ukey,strlen(ukey));
  i = mcrypt_generic_init(td, key, keysize, IV);
  if(i < 0)
  {
    mcrypt_perror(i);
    return MCRYPT_FAILED;
  }
  if(action == encrypt_message)
  {
    i = mcrypt_generic(td,message,length);
    if(i != 0)
    {
      printf("Error encrypting\n");
    }
  }
  if(action == decrypt_message)
  {
    mdecrypt_generic(td,message,length);
  }
  mcrypt_generic_end(td);
  return length;
}

//thread function-----------------------------------
void* my_thread(void* ptr)
{
	//write to STDOUT from the pipe
	char buffer;
	while(read(from_child_pipe[0], &buffer, 1)>0)//read from_child_pipe[0]
	{
		if(encrypt_flag)
		{
			msg_crypt(&buffer, key, 1, encrypt_message);
		}
		write(STDOUT_FILENO, &buffer, 1);
	}
	exit_handler(1);
	pthread_exit(NULL);
}

//handlers----------------------------------------------

//sigpipe sighup -> restore terminal modes
void sigpipe_handler(int signum)
{
	close(sockfd);
  	close(newsockfd);
  	kill(child_pid, SIGKILL);
  	exit_handler(2);
}

//main----------------------
int main(int argc, char *argv[]){

	int newsockfd, clilen;
   	char buffer[256];
   	struct sockaddr_in serv_addr, cli_addr;
   	int  n;

   	int ret = 0;
    while(1)
    {
    	ret = getopt_long(argc,argv,"",long_options,NULL);
    	if(ret == -1)
    	{
        	break;
        }
    	switch(ret)
	    {
	    	case 'p':
	    		port_flag = 1;
        		port_number = atoi(optarg);
	    		break;
	    	case 'e':
	    		encrypt_flag = 1;
	    		break;
	    	default:
	    		abort();
	    }
    }

    if (signal(SIGPIPE, sigpipe_handler) == SIG_ERR)
  	{
    	fprintf(stderr, "Error: cannot create the SIGPIPE handler.\n");
    	exit(1);
  	}
  
  	if (!port_flag)
 	{
    	fprintf(stderr, "Error: do not have port number to set up the server\n");
    	exit(1);
  	}

    //setup server------------------
   
   	/* First call to socket() function */
   	sockfd = socket(AF_INET, SOCK_STREAM, 0);
   
   	if (sockfd < 0) {
      	perror("ERROR opening socket");
      	exit(1);
   	}
   
   	/* Initialize socket structure */
   	memset((char *) &serv_addr, 0, sizeof(serv_addr));
   	//bzero((char *) &serv_addr, sizeof(serv_addr));
   
   	serv_addr.sin_family = AF_INET;
   	serv_addr.sin_addr.s_addr = INADDR_ANY;
   	serv_addr.sin_port = htons(port_number);
   
   	/* Now bind the host address using bind() call.*/
   	if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
      	perror("ERROR on binding");
      	exit(1);
   	}
   	else
   		connected = 1;
      
   	/* Now start listening for the clients, here process will
      * go in sleep mode and will wait for the incoming connection
   	*/
   
   	listen(sockfd,5);
   	clilen = sizeof(cli_addr);
   
   	/* Accept actual connection from the client */
   	newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen);
   
   	if (newsockfd < 0) {
      	perror("ERROR on accept");
      	exit(1);
   	}



    //if connected, read from socket----------------------
    if(connected)
    {
	  	//fork process -------------
		if(pipe(to_child_pipe) == -1)
		{
			fprintf(stderr,"pipe() failed:\n");
			exit(1);
		}
		if(pipe(from_child_pipe) == -1)
		{
			fprintf(stderr,"pipe() failed:\n");
			exit(1);
		}
		child_pid = fork();

		//parent process
		if(child_pid > 0)
		{
			//close pipe first
			close(to_child_pipe[0]);//input
			close(from_child_pipe[1]);//output

			dup2(newsockfd, STDIN_FILENO);
		    dup2(newsockfd, STDOUT_FILENO);
		    dup2(newsockfd, STDERR_FILENO);
		    close(newsockfd);

			//create a thread to read from STDIN to the pipe 
			pthread_t thread_1_id;
			if(pthread_create(&thread_1_id,NULL,my_thread,NULL))
			{
				fprintf(stderr, "Error: cannot create new thread.\n");
				exit(1);
			}

			//read input from the socket, echo it to stdout
			char buffer;
			int size;
			while((size = read(STDIN_FILENO, &buffer, 1))>-1)
			{
				if (size == 0)
			  	{
			  		fprintf(stderr, "Error: cannot read from the socket.\n");
					exit(1);
			  	}
				if(encrypt_flag)
				{
					msg_crypt(&buffer, key, 1, decrypt_message);
				}
				write(to_child_pipe[1],&buffer,1);
			}
		}
		//child process
		else if(child_pid == 0)
		{
			//copies descripters to stdin and stdout
			close(to_child_pipe[1]);
			close(from_child_pipe[0]);
			dup2(to_child_pipe[0],STDIN_FILENO);
			dup2(from_child_pipe[1],STDOUT_FILENO);
			close(to_child_pipe[0]);
			close(from_child_pipe[1]);

			char *execvp_argv[2];
			char execvp_filename[] = "/bin/bash";
			execvp_argv[0] = execvp_filename;
			execvp_argv[1] = NULL;
			if(execvp(execvp_filename,execvp_argv) == -1)
			{
				fprintf(stderr, "execvp() failed\n");
				exit(1);
			}
		}
		//fork failed
		else
		{
			fprintf(stderr, "fork() failed\n");
			exit(1);
		}

		close(newsockfd);
    	close(sockfd);
		exit_handler(0);
    } 

    exit(0);
}

