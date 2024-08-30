#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

int main(int argc, char * argv[]){
//        write(STDOUT_FILENO, "jaja", 4);
//        exit(0);
    while (1) {
        char c;
        int n = read(STDIN_FILENO, &c, 1);
        if (n == -1) {

            perror("Error");
            break;
        }

        // Read returns 0 when the end of the file was reached.
        if (n == 0)
            break;

        if (c >= 'a' && c <= 'z')
            c = c - ('a' - 'A');
        write(STDOUT_FILENO, &c, n);
    }

    exit(0);
}