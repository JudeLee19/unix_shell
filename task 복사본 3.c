#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#define MAX_CMD_ARG 10


#define CHECK(x) if(!(x)) { perror(#x " failed"); abort();}

const char *prompt = "myshell> ";
char *cmdvector[MAX_CMD_ARG];
char  cmdline[BUFSIZ];

char *cmdvectorPipe1[2];
char *cmdvectorPipe2[2];
char *cmdvectorPipe3[2];

void fatal(char *str){
	perror(str);
	exit(1);
}

static void child_handler(int sig)
{
    int status;
    pid_t pid;

    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
    }
}

int makelist(char *s, const char *delimiters, char** list, int MAX_LIST){	
  int i = 0;
  int numtokens = 0;
  char *snew = NULL;

  if( (s==NULL) || (delimiters==NULL) ) return -1;

  snew = s + strspn(s, delimiters);
  if( (list[numtokens] = strtok(snew, delimiters)) == NULL )
    return numtokens;
	
  numtokens = 1;
  
  while(1){
     if( (list[numtokens] = strtok(NULL, delimiters)) == NULL)
	break;
     if(numtokens == (MAX_LIST-1)) return -1;
     numtokens++;
  }
  return numtokens;
}

void sig_handler(int signo){
    if (signo == SIGINT){
        printf("\nreceived SIGINT \n");
    }
    else if (signo == SIGKILL)
        printf("\nreceived SIGKILL\n");
    else if (signo == SIGSTOP)
        printf("\nreceived SIGSTOP\n");
    else if (signo == SIGQUIT){
        printf("\nreceived SIGQUIT\n");
    }
}

int main(int argc, char**argv){
    int i=0;
    pid_t pid;

    signal(SIGCHLD, (void *)child_handler);
    
    int pipes[4];
    pipe(pipes); // First pipe.
    pipe(pipes + 2); // Second pipe.

    
    while (1) {
        signal(SIGINT,sig_handler);
        signal(SIGTSTP,sig_handler);
        signal(SIGQUIT, sig_handler);

        //Get Input      
        fputs(prompt, stdout);
        fgets(cmdline, BUFSIZ, stdin);
        cmdline[ strlen(cmdline) -1] ='\0';
        
        makelist(cmdline, " \t", cmdvector, MAX_CMD_ARG);
        
        // Pipe check.
        int t = 0;
        int k = 0;
        int j = 0;
        
        for(t; cmdvector[t] != NULL; t++){
            if(!strcmp(cmdvector[t], "|")){
                cmdvectorPipe1[t] = NULL;
                break;
            }
            cmdvectorPipe1[t] = cmdvector[t];
        }
        
        for(t = t + 1; cmdvector[t] != NULL; t++){
            if(!strcmp(cmdvector[t], "|")){
                cmdvectorPipe2[k] = NULL;
                break;
            }
            cmdvectorPipe2[k] = cmdvector[t];
            k++;
        }
        
        for(t = t + 1; cmdvector[t] != NULL; t++){
            if(!strcmp(cmdvector[t],"|")){
                cmdvectorPipe3[j] = NULL;
                break;
            }
            cmdvectorPipe3[j] = cmdvector[t];
            j++;
        }
        /*
        printf("\n 일단 1: %s %s\n", cmdvectorPipe1[0], cmdvectorPipe1[1]);
        printf("\n 일단 2: %s %s\n", cmdvectorPipe2[0], cmdvectorPipe2[1]);
        printf("\n 일단 3: %s %s\n", cmdvectorPipe3[0], cmdvectorPipe3[1]);
        */
        
        // ----------------------------------------------------------//

        if(!strcmp(cmdline,"exit"))
            return 0;

        char *tmp=strstr(cmdline,"cd");

        if(tmp==NULL){ // In case of not cd command.
            
            char *tmp2=strstr(cmdline,"&");
            
            if(tmp2==NULL){ //In cace of Foreground
                
                if((pid = fork()) == 0){
                    signal(SIGINT,sig_handler);
                    signal(SIGTSTP,sig_handler);
                    signal(SIGQUIT, sig_handler);
                    
                    
                    if(cmdvectorPipe2[0] != NULL){ // If pipe is exists
                        
                    }
                    
                    execvp(cmdvectorPipe1[0], cmdvectorPipe1);
                    
                    fatal("main()");

                }
                else if(pid == -1){
                    fatal("main()");
                }
                else{ // In case of parent.
                    wait(NULL);
                }
            }
        // ------------------------------------------------------------//
        }
        else if(tmp!=NULL){ // In case of cd command.
            makelist(cmdline, " \t", cmdvector, MAX_CMD_ARG);
            chdir(cmdvector[1]);
        }
    }//end of while	
    return 0;
}

