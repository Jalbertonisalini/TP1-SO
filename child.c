#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>


int main(int argc, char * argv[]){
    char buffer[256];
    read(STDIN_FILENO, buffer, 12);
    if(strcmp(buffer, "wachin") == 0){
        write(STDOUT_FILENO, "Vos sos gil", 11);
    }
    exit(0);
}