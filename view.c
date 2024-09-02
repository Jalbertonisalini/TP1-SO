#include <sys/mman.h>
#include <sys/stat.h>        /* For mode constants */
#include <fcntl.h>           /* For O_* constants */
#include <ctype.h>
#include "pshm_ucase.h"
#include <string.h>
#define ERROR (-1)
#define SHM_PATH "/sharedMem"

char shm_path[100];
struct shmbuf  *shmp;

int main(int argc, char * argv[])
{
//    if(read(STDIN_FILENO,shm_path,100) == ERROR){
//        errExit("Error while reading from STDIN");
//    }


    if(argc >= 2)
    {
        strcpy(shm_path,argv[1]);
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

    if(sem_wait(&shmp->resultadoDisponible) == ERROR) //1 --> 0
    {
        perror("sem_wait");
    }
    printf("%s",shmp->buf);

    while (shmp->buf[0] != EOF)
    {
        printf("%s",shmp->buf);
        sem_post(&shmp->resultadoLeido);

        if(sem_wait(&shmp->resultadoDisponible) == ERROR)
        {
            perror("sem_wait");
        }
    }

    printf("Todos han terminado\n");

/* Wait for semaphore */
  //  if (sem_wait(&shmp->resultadoDisponible) == -1) {
  //      errExit("sem_wait");
  //  }

/* Read data */
   // printf("Received message: %s\n", shmp->buf);

/* Clean up */
    if (munmap(shmp, sizeof(*shmp)) == -1) {
        errExit("munmap");
    }

    shm_unlink(SHM_PATH);

}