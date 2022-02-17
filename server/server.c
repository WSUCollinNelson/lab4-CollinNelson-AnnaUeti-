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
#define PORT 2244

int n;

char ans[MAX];
char line[MAX], tokenBuffer[MAX];
char *args[8];
int argCount;
char *t1 = "xrwxrwxrw-------";
char *t2 = "----------------";

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

int sls_file(char *fname, char* buffer) 
{ 
  char tempBuffer[128];
  buffer[0] = '\0';
  struct stat fstat, *sp; int r, i; 
  char ftime[64]; sp = &fstat; 
  if ( (r = lstat(fname, &fstat)) < 0)
  { 
    sprintf(tempBuffer, "can't stat %s\n", fname);
    exit(1); 
  } 
  if ((sp->st_mode & 0xF000) == 0x8000) // if (S ISREG()) 
    sprintf(tempBuffer, "%c", ' '); 
  if ((sp->st_mode & 0xF000) == 0x4000) // if (S ISDIR()) 
    sprintf(tempBuffer, "%c",'d'); 
  if ((sp->st_mode & 0xF000) == 0xA000) // if (S ISLNK()) 
    sprintf(tempBuffer, "%c",'l'); 
  strcat(buffer, tempBuffer);
  for (i=8; i >= 0; i-- )
  { 
    if (sp->st_mode & (1 << i)) // print r|w|x 
      sprintf(tempBuffer, "%c", t1[i]); 
    else 
      sprintf(tempBuffer, "%c", t2[i]); // or print 
    strcat(buffer, tempBuffer);
  } 
  sprintf(tempBuffer, "%4d ",sp->st_nlink); // link count 
  strcat(buffer, tempBuffer);

  sprintf(tempBuffer, "%4d ",sp->st_gid); // gid 
  strcat(buffer, tempBuffer);
  sprintf(tempBuffer, "%4d ",sp->st_uid); // uid 
  strcat(buffer, tempBuffer);
  sprintf(tempBuffer, "%8d ",sp->st_size); // file size 
  strcat(buffer, tempBuffer);
  
  // print time 
  strcpy(ftime, ctime(&sp->st_ctime)); // print time in calendar form 
  ftime[strlen(ftime) - 1] = 0; // kill \n at end 


  sprintf(tempBuffer, "%s ",ftime); 
  strcat(buffer, tempBuffer);
  sprintf(tempBuffer, "%s", basename(fname)); // print file basename 
  strcat(buffer, tempBuffer);
  
  // print -> linkname if symbolic file 
  if ((sp->st_mode & 0xF000)== 0xA000)
  { 
    char linknameBuffer[NAME_MAX];
    ssize_t linkname = readlink(fname, linknameBuffer, NAME_MAX); // use readlink() to read linkname 
    sprintf(tempBuffer, " > %s", linkname); // print linked name 
    strcat(buffer, tempBuffer);
  } 
  sprintf(tempBuffer, "\n");
  strcat(buffer, tempBuffer);
}

int main(int argc, char *argv[], char *env[]) 
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

  if(argc == 1)
  {
    saddr.sin_port = htons(PORT);
  }
  else
  {
    saddr.sin_port = htons(atoi(argv[1]));
  }
  
    
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
              sls_file(dp->d_name, ans);
              n = write(cfd, ans, MAX);
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
          printf("Transfering: %d bytes",transferRemaining);
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
          int returnCode = stat(args[1], &fileStats);

          if(returnCode < 0)
          {
            char error[100];
            perror(error);
            n = write(cfd, "get FAILED - cannot find file", MAX);
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
          n = write(cfd, "rm OK", MAX);
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
      }
    }
  }
}