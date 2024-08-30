//application.c

#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

#define MAX_LEN 256     /* Buffer size --> Big enough */
#define SLAVES 5        /* Fixed slave number */
#define ERROR (-1)        /* ERROR code */
#define FIRST_PIPE_FD 3  /* By default, pipe read is on 3 */
#define LAST_PIPE_FD 6   /* By default, after creating 2 pipes, second pipe's write is on 6 */


typedef struct
{
    int pipeWriteFd[2]; /* Child WRITES */ //parent READS from thisPipe[0]
    int pipeReadFd[2];  /* Child READS */ //parent WRITES to thisPipe[1]
    int childPid;
}Child;

char buff[200];
char buff2[200];


int createChild(int childN);
int getChildIndex(int childPid);

Child children[SLAVES];

int main(int argc, char * argv[]){

    printf("el fd es %d     \n",getpid());

    /* Creates CHILDREN */
    for (int i = 0; i < SLAVES; ++i) {
        createChild(i);
    }

    /* Parent closes extra channels that he does not need. */
    for (int i = 0; i < SLAVES; ++i) {
        close(children[i].pipeReadFd[0]);
        close(children[i].pipeWriteFd[1]);
    }
    int status;
    char *arr[8] = {"hola", "mmmmmm", "Pepe", "Gagol", "foda", "up", "A", "0"};
    int i = 0;
    int j = 0;
    while (strcmp(arr[i], "0") != 0){

        if(j == 5){
            j = 0;
        }

        write(children[j].pipeReadFd[1],arr[i], strlen(arr[i]));
        sleep(1); // espero para que termine de escribir en el slave
        ssize_t nRead = read(children[j].pipeWriteFd[0],buff,200);



        buff[nRead] = 0;
        printf("el proceso con PID dice: %s \n",buff);

        i++;
        j++;

    }
//    sleep(10); //para que termine una vez q leyo el 0

    // ACA verdaderamente lo estoy matando porque ya no lo uso mas
    close(children[0].pipeReadFd[1]);
    int childPid = waitpid(-1, &status, 0);
    int m = getChildIndex(childPid);
    printf("el proceso con PID %d es indice %d\n",childPid, m);

    printf("Todos han terminado");
    return 0;
}

int getChildIndex(int childPid)
{
    for (int i = 0; i < SLAVES; ++i) {
        if(childPid == children[i].childPid)
        {
            return i;
        }
    }
    return -1;
}

int createChild(int childN)
{
    char * CHILDPATH = "./child";
    char * text = "rojo";

    if(pipe(children[childN].pipeReadFd) == ERROR){
        perror("Error while creating pipe");
    }
    if(pipe(children[childN].pipeWriteFd) == ERROR){
        perror("Error while creating pipe");
    }

    /* 0,1,2 -> TTy 3-> pipeRead.read  4 -> pipeRead.write 5->pipeWrite.read 6 -> pipeWrite.Write
    *  */

    int pidChild;

    if((pidChild = fork()) < 0){
        perror("Error while creating child");
        exit(1);
    }
    else if (pidChild == 0){ /* child process */
        char * args[] = {CHILDPATH, text, NULL};
        char *envp[] = {NULL};


        /* Child sets his own STDIN and STDOUT*/

        dup2(children[childN].pipeReadFd[0], STDIN_FILENO);
        dup2(children[childN].pipeWriteFd[1], STDOUT_FILENO);

        /* Child closes what he does not need */

        for (int i = FIRST_PIPE_FD; i <= LAST_PIPE_FD + 4 * childN ; ++i) {
            close(i);
        }
        execve(CHILDPATH,args,envp);
    }
    children[childN].childPid = pidChild;
    return 0;
}


