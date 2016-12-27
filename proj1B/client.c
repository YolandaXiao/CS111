
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

//-------------------------------------

static struct option long_options[]=
{
   	{"log",required_argument,NULL,'l'},
   	{"encrypt",no_argument,NULL,'e'},
   	{"port",required_argument,NULL,'p'}
};

static int port_flag    = 0;
static int port_number  = 0;
static int encrypt_flag = 0;
static int log_flag = 0;
static char* log;
static int log_fd = 0;

static MCRYPT td;
static int sockfd;

int encrypt_message = 1; 
int decrypt_message = 0;
char key[100];

//--------------------------------
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

// Noncanonical Mode Example-------------------------

struct termios saved_attributes;

void
reset_input_mode (void)
{
  tcsetattr (STDIN_FILENO, TCSANOW, &saved_attributes);
}

void
set_input_mode (void)
{
  struct termios tattr;
  char *name;

  /* Make sure stdin is a terminal. */
  if (!isatty (STDIN_FILENO))
    {
      fprintf (stderr, "Not a terminal.\n");
      exit (EXIT_FAILURE);
    }

  /* Save the terminal attributes so we can restore them later. */
  tcgetattr (STDIN_FILENO, &saved_attributes);
  atexit (reset_input_mode);

  /* Set the funny terminal modes. */
  tcgetattr (STDIN_FILENO, &tattr);
  tattr.c_lflag &= ~(ICANON|ECHO); /* Clear ICANON and ECHO. */
  tattr.c_cc[VMIN] = 1;
  tattr.c_cc[VTIME] = 0;
  tcsetattr (STDIN_FILENO, TCSAFLUSH, &tattr);
}

//handlers-----------------------------------------------
void exit_handler(int num)
{
	exit(num);
}

//thread function-----------------------------------
//read from socket
void* my_thread(void* ptr)
{
	char buffer;
	int size = 0;
	while((size = read(sockfd, &buffer, 1))>0)
	{
		if(log_flag)
		{
			dprintf(log_fd, "RECEIVED %d bytes: %c\n", size, buffer);
		}
		if(encrypt_flag)
		{
			msg_crypt(&buffer, key, size, decrypt_message);
		}
		write(STDOUT_FILENO, &buffer, 1);
	}
	exit_handler(1);
	pthread_exit(NULL);
}



/////////////////////////////
//main----------------------
int main(int argc, char *argv[]){

	//int portno, n;
    struct sockaddr_in serv_addr;
    struct hostent *server;
	   
	char buffer[256];

    while(1)
    {
    	int ret = 0;
    	ret = getopt_long(argc,argv,"p:el:",long_options,NULL);
    	if(ret == -1)
    	{
        	break;
        }
    	switch(ret)
	    {
	    	case 'l':
	    		log_flag = 1;
	    		log = optarg;
	    		break;
	    	case 'p':
	    		port_flag = 1;
        		port_number = atoi(optarg);
	    		//portno = atoi(optarg);
	    		break;
	    	case 'e':
	    		encrypt_flag = 1;
	    		break;
	    	default:
	    		abort();
	    }
    }

    if(log_flag)
    {
    	if((log_fd = creat(log,0666))<0)
    	{
    		fprintf(stderr, "Error: cannot create the log file\n");
    		exit(1);
    	}
    }
    if (!port_flag)
  	{	
    	fprintf(stderr, "No port specified to connect. Exiting client...\n");
    	exit(1);
  	}

    //------------------------------

    set_input_mode ();

    //------------------------------
    //setup connection
	   
	/* Create a socket point */
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	   
	if (sockfd < 0) {
	    perror("ERROR opening socket");
	    exit(1);
	}
		
	//server = gethostbyname(argv[1]);
	server = gethostbyname("localhost");
	//only works if run both on the same server. ex. lnxsrv09
	   
	if (server == NULL) {
	    fprintf(stderr,"ERROR, no such host\n");
	    exit(0);
	}
	   
	//use memset instead of bzero -> for serv_addr
	//use memcpy instead of bcopy
	//portno should be an option
	//memset(void *s, int c, size_t n);
	memset((char *) &serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	memcpy((char *)&serv_addr.sin_addr.s_addr, (char *)server->h_addr, server->h_length);
	serv_addr.sin_port = htons(port_number);
	   
	/* Now connect to the server */
	if (connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
	    perror("ERROR connecting");
	    exit(1);
	}


	//---------------------------------------
	//read from socket (decrypt message)
	pthread_t thread_1_id;
	if(pthread_create(&thread_1_id,NULL,my_thread,NULL))
	{
		fprintf(stderr, "Error: cannot create new thread.\n");
		exit(1);
	}

	//read from terminal (encrypt message)
  	char c;
  	int size = 0;
  	while((size = read(STDIN_FILENO,&c,1)) != -1)
	{
		if (c == '\004')          /* C-d */
	    {
	    	close(sockfd); //close socket
	    	break;
	    }
	    else if (c == '\r' || c == '\n') 
	    {
	    	write(STDOUT_FILENO,"\r\n",2);
	    	c = '\n';
	    	if(encrypt_flag)
	    	{
	    		msg_crypt(&c, key, 1, encrypt_message);
	    	}
	    	write(sockfd,&c,1);
	    }
	    else
	    {
	    	write(STDOUT_FILENO,&c,1);
	    	if(encrypt_flag)
	    	{
	    		msg_crypt(&c, key, 1, encrypt_message);
	    	}
	    	write(sockfd,&c,1);
	    }
	    if(log_flag)
	    {
	    	dprintf(log_fd, "SENT %d bytes: %c\n", size, c);
	    }
	}

	close(sockfd);
    exit(0);
}

