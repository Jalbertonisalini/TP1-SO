//application.c

#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>


#define SLAVES 2

int main(int argc, char * argv[]){

    char * PATH = "./child";
    char * text = "rojo";
    int status;

    int pipefd[2];
    pid_t pidChild;

    if(pipe(pipefd) == -1){
        perror("Error while creating pipe");
    }
    printf("%d \n", pipefd[0]);
    printf("%d \n", pipefd[1]);

//    close(STDIN_FILENO);
    //close(1);
    
    dup2(pipefd[0], STDIN_FILENO);
    //dup(4);

    if((pidChild = fork()) < 0){
        perror("Error while creating child");
        exit(1);
    }
    else if (pidChild == 0){ /* child process */
        char * args[] = {PATH, text, NULL};
        char *envp[] = {NULL};
        execve(PATH,args,envp);
    }

    close(0);
    close(1);
    open("/dev/tty", O_RDWR);
    open("/dev/tty", O_RDWR);

    write(pipefd[1],"Hola chuan\n", 11);


//    for (int i = 0; i < SLAVES; i++){
//
//        if((pidChild = fork()) < 0){
//            perror("Error while creating child");
//            exit(1);
//        }
//        else if (pidChild == 0){ /* child process */
//            char * args[] = {PATH, text, NULL};
//            char *envp[] = {NULL};
//            execve(PATH,args,envp);
//        }
//    }
//    int pid2;
//    //sleep(12);
//    for(int i = 0; i < SLAVES; i++) {
//        pid2 = waitpid(-1, &status, 0);
//        printf("%d \n", pid2);
//    }
    int pid2 = waitpid(-1, &status, 0);
    printf("Todos han terminado");
    return 0;
}

