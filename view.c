// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include <sys/mman.h>
#include <sys/stat.h>        /* For mode constants */
#include <fcntl.h>           /* For O_* constants */
#include <ctype.h>
#include "pshm_ucase.h"
#include <string.h>
#define ERROR (-1)
#define SHM_PATH "/sharedMem"

#define MAXLENTH 256

char shm_path[MAXLENTH];
struct shmbuf  *shmp;

int main(int argc, char * argv[])
{
    if(argc >= 2) // ./view
    {
        strncpy(shm_path, argv[1], 255);
        shm_path[255] = '\0';
    }
    else
    {
        if (read(STDIN_FILENO,shm_path,100) == ERROR){
            errExit("Error while reading from STDIN");
        }
    }

    int shm_fd = shm_open(shm_path, O_RDWR, S_IRUSR | S_IWUSR);

    if (shm_fd == -1) {
        errExit("Error in shm_open()");
    }

    shmp = mmap(NULL, sizeof(*shmp), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);

    if (shmp == MAP_FAILED) {
        errExit("mmap");
    }

    shmp->buf[0] = 1;

    if(sem_wait(&shmp->resultadoDisponible) == ERROR) //1 --> 0
    {
        perror("sem_wait");
    }

    while ((int)shmp->buf[0] != EOF) // root/TP1-SO/view.c	47	err	V739 EOF should not be compared with a value of the 'char' type. The 'shmp->buf[0]' should be of the 'int' type.
    {
        printf("%s",shmp->buf);
        sem_post(&shmp->resultadoLeido);

        if(sem_wait(&shmp->resultadoDisponible) == ERROR)
        {
            perror("sem_wait");
        }
    }

//    printf("Todos han terminado\n");

/* Clean up */
    if (munmap(shmp, sizeof(*shmp)) == -1) {
        errExit("munmap");
    }

    shm_unlink(SHM_PATH);

}