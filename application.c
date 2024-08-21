#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>

int main(int argc, char * argv[]){

    char * PATH = "./child";
    char * args[] = {PATH, NULL};
    int status;

    int pid = fork();


    if(pid == -1){
        perror("Error while creating child");
        exit(1);
    }
    else if (pid == 0){ /* child process */
        execve(PATH,args,NULL);
    }
    int pid2;
    sleep(12);
    pid2 = waitpid(-1, &status, 0);
    if(pid == pid2)
    {
        printf("Yupi \n");
    }
    else
    {
        printf("noYupi, it did not ended \n");
        printf("%i",status);
    }

    printf("Todos han terminado");
    return 0;
}