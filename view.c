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
int stringToReadStartOffset = 0;
int stringToReadEndOffset = 0;
int filesRead = 0;

void readOutput();
void cleanUp();

int main(int argc, char * argv[])
{
    if(argc >= 2)
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
        errExit("Wrong sharedMem path");
    }

    shmp = mmap(NULL, sizeof(*shmp), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);

    if (shmp == MAP_FAILED) {
        errExit("mmap");
    }

    shmp->buf[0] = 1; //creo que lo podemos sacar, el app nunca le hace sem wait al vista

    while (filesRead < shmp->totalFiles)
    {
        readOutput();
        filesRead++;
    }

    printf("Todos han terminado\n");
    cleanUp();
}

void readOutput()
{
    if(sem_wait(&shmp->resultadoDisponible) == ERROR)
    {
        perror("sem_wait");
    }
    while (shmp->buf[stringToReadEndOffset] != 0){
        stringToReadEndOffset++;
    }
    //stringToReadEndOffset++;
    printf("%s",shmp->buf + stringToReadStartOffset);
    stringToReadStartOffset = stringToReadEndOffset;
}

void cleanUp()
{
    if (munmap(shmp, sizeof(*shmp)) == -1) {
        errExit("munmap");
    }

    shm_unlink(SHM_PATH);
}