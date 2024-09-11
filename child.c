// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#define MAX_BUFF 256
#define ERROR (-1)

//hice que no lea por caracteres xq se rompia
char buff[200];
char md5Buff[MAX_BUFF];

int main(int argc, char * argv[]){
    while (1) {

        ssize_t n = read(STDIN_FILENO, buff, 170);

        if (n == ERROR){
            perror("Error en la lectura");
            exit(EXIT_FAILURE);
        }

        //archivouno|
        //
        buff[n] = 0;
        //archivouno|\0

        if(n==0){
            exit(1);
        }

        char delimiters[] = "|\n";
        char *token = strtok(buff, delimiters);
        //token = archivouno\0
        while (token!= NULL)
        {
            char command[MAX_BUFF];
            snprintf(command, sizeof(command), "md5sum %s", token);
            //command = md5sum archivouno\0

            FILE *pipe = popen(command, "r");

            if(pipe == NULL){
                perror("Error while opening pipe");
                exit(EXIT_FAILURE);
            }

            fgets(md5Buff,sizeof(md5Buff),pipe);

            int status = pclose(pipe); //close the pipe correct
            if (status == -1) {
                perror("pclose");
                return EXIT_FAILURE;
            }
            write(STDOUT_FILENO, md5Buff, strlen(md5Buff));
           // write(STDOUT_FILENO,"|",1);
            token = strtok(NULL, delimiters);

        }


    }

    exit(0);
}