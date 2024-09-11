#ifndef TP1_SO_PSHM_UCASE_H
#define TP1_SO_PSHM_UCASE_H

#include <fcntl.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#define errExit(msg)    do { perror(msg); exit(EXIT_FAILURE); \
                                   } while (0)

#define BUF_SIZE 3000  /* Maximum size for exchanged string */

/* Define a structure that will be imposed on the shared
   memory object */

struct shmbuf {
    sem_t  resultadoDisponible;            /* POSIX unnamed semaphore Puedo leer tranqui*/
    sem_t  resultadoLeido;            /* POSIX unnamed semaphore Puedo escribir tranquilo*/
    size_t cnt;             /* Number of bytes used in 'buf' */
    char   buf[BUF_SIZE];   /* Data being transferred */
};

#endif //TP1_SO_PSHM_UCASE_H
