#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

int main(int argc, char * argv[]){
    while (1) {
        write(STDOUT_FILENO, "jaja", 5);
        sleep(3);
    }
    exit(0);
}