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
    int shm_fd = shm_open(SHM_PATH,O_CREAT | O_RDWR,S_IRUSR | S_IWUSR);

    if(shm_fd == ERROR){
        errExit("Error in shm_open()");
    }

    shmp = mmap(NULL, sizeof(*shmp), PROT_READ,
                MAP_SHARED, shm_fd, 0);

//    if (sem_post(&shmp->sem1) == -1)
//        errExit("sem_post");

    printf("%s",shmp->buf);/* skill issue */

    if (munmap(shmp, sizeof(*shmp)) == -1) {
        perror("munmap");
        exit(1);
    }

    shm_unlink(SHM_PATH);

}