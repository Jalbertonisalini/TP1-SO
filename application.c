//application.c

#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>

#define MAX_LEN 256     /* Buffer size --> Big enough */
#define SLAVES 5        /* Fixed slave number */
#define ERROR -1        /* ERROR code */

typedef struct
{
    int pipeWriteFd[2]; /* Child WRITES */ //parent READS from thisPipe[0]
    int pipeReadFd[2];  /* Child READS */ //parent WRITES to thisPipe[1]
    char pipeReadPath[MAX_LEN];
    char pipeWritePath[MAX_LEN];
    int childPid;
}Child;

int createChild(int childN);

char buffStdOut[MAX_LEN];
ssize_t readBytesStd;

Child children[1];

int main(int argc, char * argv[]){

    readBytesStd = readlink("/proc/self/fd/1", buffStdOut,MAX_LEN - 1);
    buffStdOut[readBytesStd] = 0; /* Null terminated */

    printf("el fd es %d     \n",getpid());
    printf("%s \n", buffStdOut);
    createChild(0);

    int status;
//
//    int pipefd[2];
//    pid_t pidChild;
//
//    if(pipe(pipefd) == ERROR){
//        perror("Error while creating pipe");
//    }
//    printf("%d \n", pipefd[0]);
//    printf("%d \n", pipefd[1]);
//
//
////    close(STDIN_FILENO);
//    //close(1);
//
//    dup2(pipefd[0], STDIN_FILENO);
//    dup2(pipefd[1], STDOUT_FILENO);
//    //dup(4);
//
//    if((pidChild = fork()) < 0){
//        perror("Error while creating child");
//        exit(1);
//    }
//    else if (pidChild == 0){ /* child process */
//        char * args[] = {CHILDPATH, NULL};
//        char *envp[] = {NULL};
//        execve(CHILDPATH,args,envp);
//    }
//
//
//    close(0);
//    close(1);
//    open(buffStdOut, O_RDWR);
//    open(buffStdOut, O_RDWR);
//    sleep(15);
//
//    write(pipefd[1],"Hola chuan\n", 11);
//    char bufferChuan[256];
////    read(STDIN_FILENO, bufferChuan, 11);
////    bufferChuan[12] = 0;
////    printf("%s", bufferChuan);
//
////    for (int i = 0; i < SLAVES; i++){
////
////        if((pidChild = fork()) < 0){
////            perror("Error while creating child");
////            exit(1);
////        }
////        else if (pidChild == 0){ /* child process */
////            char * args[] = {PATH, text, NULL};
////            char *envp[] = {NULL};
////            execve(PATH,args,envp);
////        }
////    }
////    int pid2;
////    //sleep(12);
////    for(int i = 0; i < SLAVES; i++) {
////        pid2 = waitpid(-1, &status, 0);
////        printf("%d \n", pid2);
////    }
    waitpid(-1, &status, 0);
    printf("Todos han terminado");
    return 0;
}

int createChild(int childN)
{

    ssize_t readBytes;

    char * CHILDPATH = "./child";
    char * text = "rojo";

    if(pipe(children[childN].pipeReadFd) == ERROR){
        perror("Error while creating pipe");
    }
    if(pipe(children[childN].pipeWriteFd) == ERROR){
        perror("Error while creating pipe");
    }

    dup2(children[childN].pipeReadFd[0], STDIN_FILENO);
    dup2(children[childN].pipeWriteFd[1], STDOUT_FILENO);

    /* What parent needs to keep track of each CHILD*/

    readBytes = readlink("/proc/self/fd/4", children[childN].pipeReadPath,MAX_LEN - 1); //pipeRead[1] (child READS, parent WRITES)
    children[childN].pipeReadPath[readBytes] = 0; /* Null terminated */

    readBytes = readlink("/proc/self/fd/5", children[childN].pipeWritePath,MAX_LEN - 1); //pipeWrite[0] (child WRITES, parent READS)
    children[childN].pipeWritePath[readBytes] = 0; /* Null terminated */




    close(children[childN].pipeReadFd[0]);
    close(children[childN].pipeReadFd[1]);
    close(children[childN].pipeWriteFd[1]);
    close(children[childN].pipeWriteFd[0]);
    

    int pidChild;
    if((pidChild = fork()) < 0){
        perror("Error while creating child");
        exit(1);
    }
    else if (pidChild == 0){ /* child process */
        char * args[] = {CHILDPATH, text, NULL};
        char *envp[] = {NULL};
        execve(CHILDPATH,args,envp);
    }
    children[childN].childPid = pidChild;

    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    open(buffStdOut, O_RDWR);
    open(buffStdOut, O_RDWR);
    printf("%s\n",children[childN].pipeReadPath);
    printf("%s \n",children[childN].pipeWritePath);

    puts("chuan <3 GAGO");
    puts("chuan");
    puts("chuan");
    puts("chuan");
    puts("chuan");
    puts("chuan");

    sleep(7);

    return 0;
}

