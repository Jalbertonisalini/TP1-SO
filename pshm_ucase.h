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

#define BUF_SIZE 3000

struct shmbuf {
    sem_t  resultadoDisponible;
    char   buf[BUF_SIZE];
    int totalFiles;
};

#endif //TP1_SO_PSHM_UCASE_H
