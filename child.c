#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>


int main(int argc, char * argv[]){
    while (1) {
        char buffer[256];
        int nRead = read(STDIN_FILENO, buffer, 12);
//        if (strcmp(buffer, "wachin") == 0) {

            write(STDOUT_FILENO, "Vos sos gil", 11);
//        }
//        if (strcmp(buffer, "wachin55") == 0) {
//            write(STDOUT_FILENO, "El rojo sale campeon de la copa argentina", 41);
//        }
        if(nRead == 0) {
            exit(0);
        }

    }
    exit(0);
//    exit(0);
}