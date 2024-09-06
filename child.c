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
        buff[n] = 0;

        char command[MAX_BUFF];
        snprintf(command, sizeof(command), "md5sum %s", buff);

        // FILE *popen(const char *command, const char *mode); // le paso el comando y el modo

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

        /*
         *   fgets()  reads in at most one less than size characters from stream and
       stores them into the buffer pointed to by s.  Reading  stops  after  an
       EOF  or a newline.  If a newline is read, it is stored into the buffer.
       A terminating null byte ('\0') is stored after the  last  character  in
       the buffer.

          fgets() returns s on success, and NULL on error or when end of file oc‐
       curs while no characters have been read.

          */

//       char output[MAX_BUFF]
//        while (fgets(md5Buff, sizeof(md5Buff), pipe) != NULL) {
//            // Concatenar la salida en el buffer de salida
//            strncat(output, md5Buff, sizeof(output) - strlen(output) - 1);
//        }



//        if(strcmp(&c, "M") != 0){
//            sleep(2);
//        }

        // Read returns 0 when the end of the file was reached.
        if (n == 0)
            break;

        //if (c >= 'a' && c <= 'z')
          //  c = c - ('a' - 'A');
        write(STDOUT_FILENO, md5Buff, strlen(md5Buff));
    }

    exit(0);
}