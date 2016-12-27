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


static int to_child_pipe[2];
static int from_child_pipe[2];
pid_t child_pid = -1;
int signal_number;

static struct option long_options[]=
{
   	{"shell",no_argument,NULL,'s'}
};
int shell = 0;
int ret = 0;

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
	int child_status;
	waitpid(child_pid,&child_status, 0);

	if(WIFEXITED(child_status))
	{
		printf("child process' exit status: %d\n",WEXITSTATUS(child_status));
	}
	else
	{
		fprintf(stderr, "Return with signal number: %d\n", signal_number);
	}
	exit(num);
}

//sigpipe sighup -> restore terminal modes
void sigpipe_handler(int signum)
{
	exit_handler(1);
}

//thread function-----------------------------------
//one thread read input from the shell pipe and write it to stdout.
void *my_thread(void *arg)
{
	//write to STDOUT from the pipe
	char buffer;
	while(read(from_child_pipe[0], &buffer, 1)>0)//read from_child_pipe[0]
	{
		write(STDOUT_FILENO, &buffer, 1);
	}
	exit_handler(1);
}

//main----------------------
int main(int argc, char *argv[]){

	//handler
  	signal(SIGPIPE,sigpipe_handler);

    while(1)
    {
    	ret = getopt_long(argc,argv,"s",long_options,NULL);
    	if(ret == -1)
    	{
        	break;
        }
    	switch(ret)
	    {
	    	case 's':
	    		shell = 1;
	    		break;
	    	default:
	    		abort();
	    }
    }

    set_input_mode ();

    if(shell)
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

			//create a thread to read from STDIN to the pipe 
			pthread_t thread_1_id;
			if(pthread_create(&thread_1_id,NULL,my_thread,NULL))
			{
				fprintf(stderr, "Error: cannot create new thread.\n");
				exit(1);
			}

			//read input from the keyboard, echo it to stdout, and forward it to the shell
			char buffer;
			while(read(STDIN_FILENO, &buffer, 1)>0)
			{
				//control-C
				if(buffer == '\003')
				{
					kill(child_pid,SIGINT);
					signal_number = SIGINT;
				}
				//control-D
				if(buffer == '\004')
				{
					close(to_child_pipe[1]);
					close(from_child_pipe[0]);
					kill(child_pid,SIGHUP);
					signal_number = SIGHUP;
					exit_handler(0);
				}

				else if (buffer == '\r' || buffer == '\n') 
			    {
			    	write(STDOUT_FILENO,"\r",1);
			    	write(STDOUT_FILENO,"\n",1);
			    	write(to_child_pipe[1],"\n",1);
			    }
				else
				{
					write(STDOUT_FILENO,&buffer,1);
					write(to_child_pipe[1],&buffer,1);
				}
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

		exit_handler(0);
    } 
    //when there's no shell argument, just output whatever typed in
    else
    {
	  	//read characters one by one, put it in char c
	  	char c;
		while(read(STDIN_FILENO,&c,1)>0)
		{
			if (c == '\004')          /* C-d */
		    {
		    	break;
		    }
		    else if (c == '\r' || c == '\n') 
		    {
		    	write(STDOUT_FILENO,"\r\n",2);
		    }
		    else
		    {
		    	write(STDOUT_FILENO,&c,1);
		    }
	    }
    }

    exit(0);
}

