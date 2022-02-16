/***** LAB4 client base code *****/ 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h> 
#include <netinet/in.h> 
#include <sys/socket.h> 
#include <sys/types.h> 
#include <arpa/inet.h>

#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include <libgen.h>     // for dirname()/basename()
#include <time.h> 

#define MAX 256
#define PORT 2222

char line[MAX], ans[MAX], tokenBuffer[MAX];
char *args[8];
int argCount;
int n;

struct sockaddr_in saddr; 
int sfd;

int tokenize(char *line)
{
  strcpy(tokenBuffer, line);
  args[0] = strtok(tokenBuffer, " ");
  int i = 1;
  while(args[i++] = strtok(NULL, " "))
  {
    continue;
  }
  argCount = i;
}

int main(int argc, char *argv[], char *env[]) 
{ 
  int n; char how[64];
  int i;

  printf("1. create a socket\n");
  sfd = socket(AF_INET, SOCK_STREAM, 0); 
  if (sfd < 0) { 
    printf("socket creation failed\n"); 
      exit(0); 
  }
    
  printf("2. fill in server IP and port number\n");
  bzero(&saddr, sizeof(saddr)); 
  saddr.sin_family = AF_INET; 
  saddr.sin_addr.s_addr = inet_addr("127.0.0.1"); 
  saddr.sin_port = htons(PORT); 
  
  printf("3. connect to server\n");
  if (connect(sfd, (struct sockaddr *)&saddr, sizeof(saddr)) != 0) { 
      printf("connection with the server failed...\n"); 
      exit(0); 
  } 

  printf("********  processing loop  *********\n");
  while (1){
    printf("input a line : ");
    bzero(line, MAX);                // zero out line[ ]
    fgets(line, MAX, stdin);         // get a line (end with \n) from stdin

    line[strlen(line)-1] = 0;        // kill \n at end
    if (line[0]==0)                  // exit if NULL line
      exit(0);

    tokenize(line);

    printf("cmd: %s\n", args[0]);

    if(strcmp(args[0], "lpwd") == 0)
    {
      char cwdBuffer[PATH_MAX];
      getcwd(cwdBuffer, PATH_MAX);
      printf("%s\n", cwdBuffer);
    }
    else if(strcmp(args[0], "pwd") == 0)
    {
      n = write(sfd, line, MAX);
      n = read(sfd, ans, MAX);
      printf("%s\n", ans);
    }
    else if(strcmp(args[0], "lcd") == 0)
    {
      if(argCount < 2)
      {
        printf("lcd FAILED - not enough arguments\n");
        continue;
      }

      int returnValue = chdir(args[1]);

      if(returnValue < 0)
      {
        printf("lcd FAILED - could not chidr to path\n");
      }
      else 
      {
        printf("lcd OK\n");
      }
    }
    else if (strcmp(args[0], "cd") == 0)
    {
      n = write(sfd, line, MAX);
      n = read(sfd, ans, MAX);
      printf("%s\n", ans);
    }

    /*
    // Send ENTIRE line to server
    n = write(sfd, line, MAX);
    printf("client: wrote n=%d bytes; line=(%s)\n", n, line);

    // Read a line from sock and show it
    n = read(sfd, ans, MAX);
    printf("client: read  n=%d bytes; echo=(%s)\n",n, ans);
    */
  }
}


/********************* YOU DO ***********************
    1. The assignment is the Project in 13.17.1 of Chapter 13

    2. Implement 2 sets of commands:

      ********************** menu ***********************
      * get  put  ls   cd   pwd   mkdir   rmdir   rm  *  // executed by server
      * lcat     lls  lcd  lpwd  lmkdir  lrmdir  lrm  *  // executed LOACLLY (this file)
      ***************************************************

    3. EXTRA Credits: make the server MULTI-threaded by processes

    Note: The client and server are in different folders on purpose.
          Get and put should work when cwd of client and cwd of server are different.
****************************************************/