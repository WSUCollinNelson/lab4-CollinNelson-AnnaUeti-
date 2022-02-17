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
#define PORT 2244

char line[MAX], ans[MAX], tokenBuffer[MAX];
char *args[8];
int argCount;
int n;
char *t1 = "xrwxrwxrw-------";
char *t2 = "----------------";

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
  argCount = i-1;
}

int main(int argc, char *argv[], char *env[]) 
{ 
  chroot("../");
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
  if(argc == 1)
  {
    saddr.sin_port = htons(PORT);
  }
  else
  {
    saddr.sin_port = htons(atoi(argv[1]));
  }
  
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
    else if (strcmp(args[0], "lls") == 0)
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
          char filepath[MAX];
          strcpy(filepath, lsPath);
          strcat(filepath, "/");
          strcat(filepath, dp->d_name);
          ls_file(filepath);
        }
        closedir(dir);
      }
      else
      {
        printf("lls FAILED - could not locate directory\n");
      }
    }
    else if(strcmp(args[0], "ls") == 0)
    {
      n = write(sfd, line, MAX);

      while(1)
      {
        n = read(sfd, ans, MAX);
        if(strlen(ans) == 0) break;
        printf("%s", ans);
      }
    }
    else if(strcmp(args[0], "put") == 0)
    {
      struct stat fileStats;
      int returnCode = stat(args[1], &fileStats);

      if(returnCode < 0)
      {
        char error[100];
        perror(error);
        printf("put FAILED - cannot find file\n");
        continue;
      }

      //Send put request to server, then send file size
      n = write(sfd, line, MAX);
      int fd = open(args[1], O_RDONLY);
      char sizeString[128];
      sprintf(sizeString, "%ld", fileStats.st_size);
      printf("Transfering %s bytes.\n", sizeString);
      n = write(sfd, sizeString, MAX);

      //Transfer data
      int transferRemaining = fileStats.st_size;
      void *fileBuffer[MAX];
      while(transferRemaining > MAX)
      {
        read(fd, fileBuffer, MAX);
        n = write(sfd, fileBuffer, MAX);
        transferRemaining -= MAX;
      }
      read(fd, fileBuffer, transferRemaining);
      n = write(sfd, fileBuffer, transferRemaining);
      close(fd);
    }
    else if(strcmp(args[0], "get") == 0)
    {
      if(argCount < 2)
      {
        printf("get FAILED - not enough arguments\n");
        continue;
      }  

      n = write(sfd, line, MAX);
      n = read(sfd, ans, MAX);
      int transferRemaining = atoi(ans);

      if(transferRemaining > 0)
      {
        int fd = open(args[1], O_WRONLY|O_TRUNC|O_CREAT);
        printf("Transfering: %d bytes\n",transferRemaining);
        while(transferRemaining > MAX)
        {
          n = read(sfd, ans, MAX);
          write(fd, ans, MAX);
          transferRemaining -= MAX;
        }
        n=read(sfd, ans, transferRemaining);
        write(fd, ans, transferRemaining);
        
        close(fd);
      }
      else
      {
        printf("%s", ans);
      }
    }
    else if (strcmp(args[0], "lrm") == 0)
    {
      if(argCount < 2)
      {
        printf("lrm FAILED - not enough arguments\n");
        continue;
      }      

      remove(args[1]);
      printf("%s OK\n", args[0]);

    }
    else if (strcmp(args[0], "rm") == 0)
    {
      n = write(sfd, line, MAX);
      n = read(sfd, ans, MAX);
      printf("%s\n", ans);
    }
    else if (strcmp(args[0], "lrmdir") == 0)
    {
      if(argCount < 2)
      {
        printf("lrmdir FAILED - not enough arguments\n");
        continue;
      }      

      rmdir(args[1]);
	    printf("%s OK\n", args[0]);

    }
    else if (strcmp(args[0], "rmdir") == 0)
    {
      n = write(sfd, line, MAX);
      n = read(sfd, ans, MAX);
      printf("%s\n", ans);
    }
    else if (strcmp(args[0], "lmkdir") == 0)
    {
      if(argCount < 2)
      {
        printf("lmkdir FAILED - not enough arguments\n");
        continue;
      }      

      mkdir(args[1], 0755);
	    printf("%s OK\n", args[0]);

    }
    else if (strcmp(args[0], "mkdir") == 0)
    {
      n = write(sfd, line, MAX);
      n = read(sfd, ans, MAX);
      printf("%s\n", ans);
    }
    else if (strcmp(args[0], "lcat") == 0)
    {
      if(argCount < 2)
      {
        printf("lcat FAILED - not enough arguments\n");
        continue;
      }    

      FILE* fileID = fopen(args[1], "r");
      char currentChar = getc(fileID);

      while(currentChar != EOF)
      {
        putchar(currentChar);
        currentChar = getc(fileID);
      }
      
      fclose(fileID);

      printf("%s OK\n", args[0]);
    }
  }
}

int ls_file(char *fname) 
{ 
  struct stat fstat, *sp; int r, i; 
  char ftime[64]; sp = &fstat; 
  if ( (r = lstat(fname, &fstat)) < 0)
  { 
    printf("can't stat %s\n", fname);
    exit(1); 
  } 
  if ((sp->st_mode & 0xF000) == 0x8000) // if (S ISREG()) 
    printf("%c", ' '); 
  if ((sp->st_mode & 0xF000) == 0x4000) // if (S ISDIR()) 
    printf("%c",'d'); 
  if ((sp->st_mode & 0xF000) == 0xA000) // if (S ISLNK()) 
    printf("%c",'l'); 
  for (i=8; i >= 0; i-- )
  { 
    if (sp->st_mode & (1 << i)) // print r|w|x 
      printf("%c", t1[i]); 
    else printf("%c", t2[i]); // or print 
  } 
  printf("%4d ",sp->st_nlink); // link count 

  printf("%4d ",sp->st_gid); // gid 
  printf("%4d ",sp->st_uid); // uid 
  printf("%8d ",sp->st_size); // file size 
  
  // print time 
  strcpy(ftime, ctime(&sp->st_ctime)); // print time in calendar form 
  ftime[strlen(ftime) - 1] = 0; // kill \n at end 


  printf("%s ",ftime); 
  printf("%s", basename(fname)); // print file basename 
  
  // print -> linkname if symbolic file 
  if ((sp->st_mode & 0xF000)== 0xA000)
  { 
    char linknameBuffer[NAME_MAX];
    ssize_t linkname = readlink(fname, linknameBuffer, NAME_MAX); // use readlink() to read linkname 
    printf(" > %s", linkname); // print linked name 
  } 
  printf("\n");
}