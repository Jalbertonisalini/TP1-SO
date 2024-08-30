#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

int main(int argc, char * argv[]){
        write(STDOUT_FILENO, "jaja", 4);
        exit(0);
}