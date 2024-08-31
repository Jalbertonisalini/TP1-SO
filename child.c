#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>



//hice que no lea por caracteres xq se rompia
char buff[200];

int main(int argc, char * argv[]){
    while (1) {
       // char c;
        ssize_t n = read(STDIN_FILENO, buff, 200);
        buff[n] = 0;

       // sleep(rand() % 2);
        sleep(1);
        if (n == -1) {

            perror("Error");
            break;
        }

//        if(strcmp(&c, "M") != 0){
//            sleep(2);
//        }

        // Read returns 0 when the end of the file was reached.
        if (n == 0)
            break;

        //if (c >= 'a' && c <= 'z')
          //  c = c - ('a' - 'A');
        write(STDOUT_FILENO, buff, n);
    }

    exit(0);
}