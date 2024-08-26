#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>

#define SLAVES 2

int main(int argc, char *argv[]) {

    char *PATH = "./child";
    char *text = "rojo";
    int status;

    int pipefd[2];
    pid_t pidChild;

    if (pipe(pipefd) == -1) {
        perror("Error while creating pipe");
        exit(EXIT_FAILURE);
    }

    printf("Pipe read descriptor: %d \n", pipefd[0]);
    printf("Pipe write descriptor: %d \n", pipefd[1]);

    // Redirigir la entrada estándar al descriptor de lectura de la tubería
    close(STDIN_FILENO); // Cierra la entrada estándar
    dup2(pipefd[0], STDIN_FILENO); // Redirige la entrada estándar al descriptor de lectura
    close(pipefd[0]); // Cierra el descriptor original de lectura
    close(pipefd[1]); // Cierra el descriptor de escritura

    // Crear SLAVES cantidad de procesos hijos
    for (int i = 0; i < SLAVES; i++) {
        
        if ((pidChild = fork()) < 0) {
            perror("Error while creating child");
            exit(EXIT_FAILURE);
        
        } else if (pidChild == 0) { /* child process */
            char *args[] = {PATH, text, NULL};
            char *envp[] = {NULL};
            execve(PATH, args, envp);
            perror("execve failed"); // Manejo de error si execve falla
            exit(EXIT_FAILURE);
        }
    }

    // En el proceso padre, manejar la tubería
    close(pipefd[0]); // El proceso padre no lee de la tubería
    write(pipefd[1], "Hola chuan\n", 11); // Escribe en la tubería
    close(pipefd[1]); // Cierra el descriptor de escritura

    // Esperar a que todos los procesos hijos terminen
    for (int i = 0; i < SLAVES; i++) {
        int pid = waitpid(-1, &status, 0);
        if (pid == -1) {
            perror("waitpid failed");
            exit(EXIT_FAILURE);
        }
        printf("Child process %d finished with status %d\n", pid, WEXITSTATUS(status));
    }

    printf("Todos han terminado\n");

    return 0;
}
