/***** LAB4 server base code *****/ 

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
#include <libgen.h>
#include <time.h>

#define MAX 256
#define PORT 2222

int n;

char ans[MAX];
char line[MAX], tokenBuffer[MAX];
char *args[8];
int argCount;

int tokenize(char *line)
{
  strcpy(tokenBuffer, line);
  args[0] = strtok(tokenBuffer, " ");
  int i = 1;
  while(args[i++] = strtok(NULL, " "))
  {
    continue;
  }
  argCount = i-1;
}

int main() 
{ 
  int sfd, cfd, len; 
  struct sockaddr_in saddr, caddr; 
  int i, length;
    
  printf("1. create a socket\n");
  sfd = socket(AF_INET, SOCK_STREAM, 0); 
  if (sfd < 0) { 
    printf("socket creation failed\n"); 
    exit(0); 
  }
    
  printf("2. fill in server IP and port number\n");
  bzero(&saddr, sizeof(saddr)); 
  saddr.sin_family = AF_INET; 
  //saddr.sin_addr.s_addr = htonl(INADDR_ANY); 
  saddr.sin_addr.s_addr = inet_addr("127.0.0.1"); 
  saddr.sin_port = htons(PORT);
    
  printf("3. bind socket to server\n");
  if ((bind(sfd, (struct sockaddr *)&saddr, sizeof(saddr))) != 0) { 
    printf("socket bind failed\n"); 
    exit(0); 
  }
      
  // Now server is ready to listen and verification 
  if ((listen(sfd, 5)) != 0) { 
      printf("Listen failed\n"); 
      exit(0); 
  }
  while(1){
    // Try to accept a client connection as descriptor newsock
    length = sizeof(caddr);
    cfd = accept(sfd, (struct sockaddr *)&caddr, &length);
    if (cfd < 0){
      printf("server: accept error\n");
      exit(1);
    }
      
    if(!fork())
    {
      printf("server: accepted a client connection from\n");
      printf("-----------------------------------------------\n");
      printf("    IP=%s  port=%d\n", "127.0.0.1", ntohs(caddr.sin_port));
      printf("-----------------------------------------------\n");

      // Processing loop
      while(1){
        printf("server ready for next request ....\n");
        n = read(cfd, line, MAX);
        if (n==0){
          printf("server: client died, server loops\n");
          close(cfd);
          break;
        }

        // show the line string
        printf("server: read  n=%d bytes; line=[%s]\n", n, line);

        tokenize(line);

        if(strcmp(args[0], "pwd") == 0)
        {
          char cwdBuffer[PATH_MAX];
          getcwd(cwdBuffer, PATH_MAX);
          n = write(cfd, cwdBuffer, MAX);
        }
        else if(strcmp(args[0], "cd") == 0)
        {
          if(argCount < 2)
          {
            n = write(cfd, "cd FAILED - not enough arguments", MAX);
            continue;
          }

          int returnValue = chdir(args[1]);

          if(returnValue < 0)
          {
            n = write(cfd, "lcd FAILED - could not chidr to path", MAX);
          }
          else 
          {
            n = write(cfd, "lcd OK", MAX);
          }
        }
        else if (strcmp(args[0], "ls") == 0)
        {
          char lsPath[PATH_MAX];
          if(argCount < 2)
          {
            getcwd(lsPath, PATH_MAX);
          }
          else
          {
            strcpy(lsPath, args[1]);
          }

          DIR *dir = opendir(lsPath);

          if(dir != NULL)
          {
            struct dirent *dp = NULL;
            while(dp = readdir(dir))
            {
              n = write(cfd, dp->d_name, MAX);
            }
            closedir(dir);
          }
          else
          {
            n = write(cfd, "ls FAILED - could not locate directory", MAX);
          }
          n = write(cfd, "", MAX);
        }
        else if(strcmp(args[0], "put") == 0)
        {
          int fd = open(args[1], O_WRONLY|O_TRUNC|O_CREAT);
          n = read(cfd, ans, MAX);
          int transferRemaining = atoi(ans);
          printf("Transfering: %d bytes\n",transferRemaining);
          while(transferRemaining > MAX)
          {
            n = read(cfd, ans, MAX);
            write(fd, ans, MAX);
            transferRemaining -= MAX;
          }
          n=read(cfd, ans, transferRemaining);
          write(fd, ans, transferRemaining);
          
          close(fd);
        }
        else if(strcmp(args[0], "get") == 0)
        {
          struct stat fileStats;
          printf("%s\n", args[1]);
          int returnCode = stat(args[1], &fileStats);

          if(returnCode < 0)
          {
            char error[100];
            perror(error);
            printf("get FAILED - cannot find file\n");
            continue;
          }

          int fd = open(args[1], O_RDONLY);
          char sizeString[128];
          sprintf(sizeString, "%ld", fileStats.st_size);
          printf("Transfering %s bytes.\n", sizeString);
          n = write(cfd, sizeString, MAX);

          //Transfer data
          int transferRemaining = fileStats.st_size;
          void *fileBuffer[MAX];
          while(transferRemaining > MAX)
          {
            read(fd, fileBuffer, MAX);
            n = write(cfd, fileBuffer, MAX);
            transferRemaining -= MAX;
          }
          read(fd, fileBuffer, transferRemaining);
          n = write(cfd, fileBuffer, transferRemaining);
          close(fd);
        }
        else if(strcmp(args[0], "rm") == 0)
        {
          if(argCount < 2)
          {
            n = write(cfd, "rm FAILED - not enough arguments", MAX);
            continue;
          }      

          remove(args[1]);
          n = write(cfd, "rm OK\n", MAX);
        }
        else if(strcmp(args[0], "rmdir") == 0)
        {
          if(argCount < 2)
          {
            n = write(cfd, "rmdir FAILED - not enough arguments", MAX);
            continue;
          }      

          rmdir(args[1]);
          n = write(cfd, "rmdir OK\n", MAX);
        }
        else if(strcmp(args[0], "mkdir") == 0)
        {
          if(argCount < 2)
          {
            n = write(cfd, "mkdir FAILED - not enough arguments", MAX);
            continue;
          }      

          mkdir(args[1], 0755);
          n = write(cfd, "mkdir OK", MAX);
        }

        /*
        strcat(line, " ECHO");

        // send the echo line to client 
        n = write(cfd, line, MAX);

        printf("server: wrote n=%d bytes; ECHO=[%s]\n", n, line);
        printf("server: ready for next request\n");
        */
      }
    }
  }
}

/********************* YOU DO ***********************
    1. The assignment is the Project in 13.17.1 of Chapter 13

    2. Implement 2 sets of commands:

      ********************** menu ***********************
      * get  put  ls   cd   pwd   mkdir   rmdir   rm  *  // executed by server (this file)
      * lcat     lls  lcd  lpwd  lmkdir  lrmdir  lrm  *  // executed LOACLLY
      ***************************************************

    3. EXTRA Credits: make the server MULTI-threaded by processes
    
    Note: The client and server are in different folders on purpose.
          Get and put should work when cwd of client and cwd of server are different.
****************************************************/