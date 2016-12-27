#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <getopt.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

void handler(int signum)
{
  fprintf(stderr, "Program quitting, signal number: %d.\n",signum);
  exit(3);
}

int main(int argc, char *argv[]){
  struct option long_options[]=
  {
    {"input",required_argument,NULL,'i'},
    {"output",required_argument,NULL,'o'},
    {"segfault",no_argument,NULL,'s'},
    {"catch",no_argument,NULL,'c'},
    {0,0,0,0}
  };

  int segfault = 0;
  int catch = 0;
  int ifd;
  int ofd;
  int ret = 0;
  int index = 0;

  while(1){
    ret = getopt_long(argc,argv,"",long_options,NULL);
    if(ret == -1){
      break;
    }
    switch(ret){
    case 'i':
      ifd = open(optarg, O_RDONLY);
      if (ifd >= 0) {
	close(0);
        dup(ifd);
	close(ifd);
      }
      else{
	perror("Input error");
        fprintf(stderr,"There's an input error.");
	exit(1);
      }
      break;
    case 'o': 
      ofd = creat(optarg, 0666);
      if (ofd >= 0) {
	close(1);
	dup(ofd);
        close(ofd);
      }
      else{
	perror("Output error");
        fprintf(stderr,"There's an output error.");
	exit(2);
      }
      break;
    case 's':
      segfault = 1;
      break;
    case 'c':
      catch = 1;
      break;
    default:
      abort();
    }
  }

  if(catch)
  {
    signal(SIGSEGV,handler);
  }
  if(segfault)
  {
    char* value = NULL;
    value[0] = '1';
  } 

  char buffer;
  int n = 0;
  while(n = read(0,&buffer,1))
  {
    write(1,&buffer,n);
  }

  exit(0);
}
