#include <stdio.h>
#include <unistd.h>

int main(int argc, char * argv[]){

    char buffer[256];
    printf("Hola a todos \n");
    read(STDIN_FILENO, buffer, 11);
    printf("Message got from pipe %s \n", buffer);
    return 0;
}