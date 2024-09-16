#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#define MAX_BUFF 256    /* Considered enough */
#define ERROR (-1)

char buff[MAX_BUFF];
char md5Buff[MAX_BUFF];

int main(int argc, char * argv[]){
    while (1) {

        ssize_t n = read(STDIN_FILENO, buff, MAX_BUFF - 1);

        if (n == ERROR){
            perror("Error en la lectura");
            exit(EXIT_FAILURE);
        }
        buff[n] = 0;

        if(n==0){
            exit(1);
        }

        char delimiters[] = "|\n";
        char *token = strtok(buff, delimiters);

        while (token!= NULL)
        {
            char command[MAX_BUFF];
            snprintf(command, sizeof(command), "md5sum %s", token);

            FILE *pipe = popen(command, "r");

            if(pipe == NULL){
                perror("Error while opening pipe");
                exit(EXIT_FAILURE);
            }

            fgets(md5Buff,sizeof(md5Buff),pipe);

            int status = pclose(pipe);
            if (status == -1) {
                perror("pclose");
                return EXIT_FAILURE;
            }
            write(STDOUT_FILENO, md5Buff, strlen(md5Buff));
            token = strtok(NULL, delimiters);
        }
    }
}