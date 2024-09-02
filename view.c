#include <sys/mman.h>
#include <sys/stat.h>        /* For mode constants */
#include <fcntl.h>           /* For O_* constants */
#include <ctype.h>
#include "pshm_ucase.h"

#define ERROR (-1)
#define SHM_PATH "/sharedMem"


struct shmbuf  *shmp;

int main(int argc, char * argv[])
{
    int shm_fd = shm_open(SHM_PATH, O_RDWR, S_IRUSR | S_IWUSR);

    if (shm_fd == -1) {
        errExit("Error in shm_open()");
    }

    shmp = mmap(NULL, sizeof(*shmp), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);

    if (shmp == MAP_FAILED) {
        errExit("mmap");
    }

    printf("Proceso 2: Esperando por el semáforo...\n");

/* Wait for semaphore */
    if (sem_wait(&shmp->sem1) == -1) {
        errExit("sem_wait");
    }

/* Read data */
    printf("Received message: %s\n", shmp->buf);

/* Clean up */
    if (munmap(shmp, sizeof(*shmp)) == -1) {
        errExit("munmap");
    }

    shm_unlink(SHM_PATH);

}