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

        // ----------------------------------------------------------//

        if(!strcmp(cmdline,"exit"))
            return 0;

        char *tmp=strstr(cmdline,"cd");

        if(tmp==NULL){ // In case of not cd command.
            
            char *tmp2=strstr(cmdline,"&");
            
            if(tmp2==NULL){ //In cace of Foreground
                
                switch( pid = fork() ){ // (To excute first commnad ( when in case of pipe also first command)
                    case 0: // In case of child.
                        
                        signal(SIGINT,sig_handler);
                        signal(SIGTSTP,sig_handler);
                        signal(SIGQUIT, sig_handler);
                        
                        
                        printf("\n cmdvector[1] : %s\n", cmdvector[1]);
                        printf("\n cmdvector[2] : %s\n", cmdvector[2]);
                        
                        if(cmdvector[1] != NULL){
                            if(strcmp(cmdvector[1], "|") == 0){
                                
                                dup2(pipes[1], 1);
                                
                                close(pipes[0]);
                                close(pipes[1]);
                                close(pipes[2]);
                                close(pipes[3]);
                                
                                char *pipe_args[] = {cmdvector[0], NULL};
                                execvp(cmdvector[0], pipe_args);
                            }
                            else{
                                execvp(cmdvector[0], cmdvector);
                            }
                        }
                        else if(cmdvector[2] != NULL){
                            if(strcmp(cmdvector[2], "|") == 0){
                                dup2(pipes[1], 1);
                                
                                close(pipes[0]);
                                close(pipes[1]);
                                close(pipes[2]);
                                close(pipes[3]);
                                
                                char *pipe_args[] = {cmdvector[0], cmdvector[1], NULL};
                                execvp(cmdvector[0], pipe_args);
                            }
                            else{
                                execvp(cmdvector[0], cmdvector);
                            }
                        }
                        else{
                            execvp(cmdvector[0], cmdvector);
                        }
                        fatal("main()");
                         
                    case -1:
                        fatal("main()");
                        
                    default: // In case of parent
                        wait(NULL);
			/*
                        if(strcmp(cmdvector[1], "|") == 0){
                            
                            if (fork() == 0){
                                
                                if(cmdvector[3] != NULL){ // ls | ls |
                                    if(strcmp(cmdvector[3], "|") == 0){
                                        printf("\n떨드 파이프에유\n ");
                                        // replace grep's stdin with read end of 1st pipe
                                        
                                        dup2(pipes[0], 0);
                                        
                                        // replace grep's stdout with write end of 2nd pipe
                                        
                                        dup2(pipes[3], 1);
                                        
                                        // close all ends of pipes
                                        
                                        close(pipes[0]);
                                        close(pipes[1]);
                                        close(pipes[2]);
                                        close(pipes[3]);
                                        
                                        char *pipe_args[] = {cmdvector[2],NULL};
                                        execvp(cmdvector[2], pipe_args);
                                    }
                                    else{
                                        printf("\n아니래유\n");
                                        char *args[] = {cmdvector[2], cmdvector[3], NULL};
                                        execvp(cmdvector[2],args);
                                    }
                                }
                            
                            }
                            else{ // To excute third pipe command.
                                
                            }
                        }
                    */    
                } // end of switch.
            }
            else{ //In case of Background
                cmdline[strlen(cmdline)-1] = NULL;
                makelist(cmdline, " \t", cmdvector, MAX_CMD_ARG);
                pid=fork();
                if(pid==0){
                    execvp(cmdvector[0], cmdvector);
                                    fatal("main()");
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

