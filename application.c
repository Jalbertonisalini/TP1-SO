// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
//application.c

#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <sys/mman.h>


#include "pshm_ucase.h"

#define MAX_LEN 256     /* Buffer size --> Big enough */
#define SLAVES 5        /* Fixed slave number */
#define ERROR (-1)        /* ERROR code */
#define FIRST_PIPE_FD 3  /* By default, pipe read is on 3 */
#define LAST_PIPE_FD 6   /* By default, after creating 2 pipes, second pipe's write is on 6 */
#define SHM_PATH "/sharedMem"
#define PATH_LEN 11
#define FILES_TO_CHILD 3

typedef struct
{
    int pipeWriteFd[2]; /* Child WRITES */ //parent READS from thisPipe[0]
    int pipeReadFd[2];  /* Child READS */ //parent WRITES to thisPipe[1]
    int childPid;
}Child;

char buff[MAX_LEN];

int createChild(int childN);
int getChildIndex(int childPid);
void initializeFdSets();
void createOutputTxt();

Child children[SLAVES];

/* select parameters */
fd_set readFds;  //A structure type that can represent a set of file descriptors.
int nfds = 3 + SLAVES *4 +1;

struct shmbuf  *shmp;


int outputFd;
//int shm_fd;

char view = 0;
char delimiters[] = "\n";
size_t stringToWriteStartOffset = 0;
char ** files;
int nfiles;
int sended = 1;
int received = 1;

void passFilesToChild(int numFiles, int childIndex);

int main(int argc, char * argv[]){

    nfiles = argc;
    files = argv;
    int shm_fd = shm_open(SHM_PATH, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);

    if (shm_fd == ERROR) {
        errExit("Error in shm_open()");
    }

    //    int outputFd = open("output.txt",O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
    createOutputTxt();



    if (ftruncate(shm_fd, sizeof(struct shmbuf)) == -1) {
        errExit("ftruncate");
    }

   shmp = mmap(NULL, sizeof(*shmp), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);

    if (shmp == MAP_FAILED) {
        errExit("mmap");
    }

    write(STDOUT_FILENO,SHM_PATH, PATH_LEN);
    shmp->totalFiles = nfiles-1;


/* Initialize semaphore */
    if (sem_init(&shmp->resultadoDisponible, 1, 0) == -1) {
        errExit("sem_init-resultadoDisponible");
    }
    if (sem_init(&shmp->resultadoLeido, 1, 1) == -1) {
        errExit("sem_init-resultadoDisponible");
    }

     sleep(2);

    if(shmp->buf[0]){
        view = 1;
    }

    /* Wait for 'resultadoDisponible' to be posted by peer before touching
       shared memory. */

//    if (sem_wait(&shmp->resultadoDisponible) == -1)
//        errExit("sem_wait");

    /* Creates CHILDREN */
    for (int i = 0; i < SLAVES; ++i) {
        createChild(i);
    }

    /* Parent closes extra channels that he does not need. */
    for (int i = 0; i < SLAVES; ++i) {
        close(children[i].pipeReadFd[0]);
        close(children[i].pipeWriteFd[1]);
    }

    int status;

     /* variable that counts number of files sended from parent to child. As the executable name is not wanted, we initialize it in 1.*/
     /* variable that counts amount of files received again by parent --> this means they have been processeed already.    */

    /* (sended - 1) is done to identify the child's index that actually starts in 0 */

    for (int i = 0; i < SLAVES; ++i) {
        passFilesToChild(FILES_TO_CHILD,i);
    }
    size_t n;


    while (received < nfiles){

        initializeFdSets();

        /* If timeout is specified as NULL, select() blocks indefinitely waiting for a file descriptor to become ready. */
        select(nfds,&readFds,NULL,NULL,NULL);

        /* veo cual de los hijos termino de escribirme */
        for (int k = 0; k < SLAVES; ++k) {

             if(FD_ISSET(children[k].pipeWriteFd[0],&readFds)) {
                ssize_t nRead;
                if((nRead = read(children[k].pipeWriteFd[0], buff, 199)) == ERROR) {
                    errExit("Error while reading from child");
                }
                buff[nRead] = 0;
                char *token = strtok(buff, delimiters);

                while (token!= NULL)
                {
                    sprintf(shmp->buf + stringToWriteStartOffset,"Hijo con PID: %d produjo Md5: %s \n",children[k].childPid,token);
                    n = strlen(shmp->buf + stringToWriteStartOffset);
                    write(outputFd,shmp->buf +stringToWriteStartOffset, n);

                    if(view){
                        sem_post(&shmp->resultadoDisponible);
                    }
                    received++;
                    token = strtok(NULL, delimiters);
                    stringToWriteStartOffset += n;
                }

                 passFilesToChild(1,k);

            }
        }
    }

    sem_post(&shmp->resultadoDisponible);

    close(children[0].pipeReadFd[1]);
    waitpid(-1, &status, 0);




    if (munmap(shmp, sizeof(*shmp)) == -1) {
        perror("munmap");
        exit(1);
    }
    close(shm_fd);
    fflush(stdout);
    close(STDOUT_FILENO);
    shm_unlink(SHM_PATH);

    return 0;
}


void createOutputTxt(){
    outputFd = open("result.txt", O_WRONLY | O_CREAT, 0644);

    if (outputFd < 0) {
        perror("Error al abrir o crear el archivo");
        return;
    }
}


void initializeFdSets()
{
    FD_ZERO(&readFds);

    for (int k = 0; k < SLAVES; ++k) {
        FD_SET(children[k].pipeWriteFd[0],&readFds);
    }
}

int createChild(int childN)
{
    char * CHILDPATH = "./child";

    if(pipe(children[childN].pipeReadFd) == ERROR){
        perror("Error while creating pipe");
        exit(1);
    }
    if(pipe(children[childN].pipeWriteFd) == ERROR){
        perror("Error while creating pipe");
        exit(1);
    }

    int pidChild;

    if((pidChild = fork()) < 0){
        perror("Error while creating child");
        exit(1);
    }
    else if (pidChild == 0){ /* child process */
        char * args[] = {CHILDPATH, NULL};
        char *envp[] = {NULL};

        dup2(children[childN].pipeReadFd[0], STDIN_FILENO);
        dup2(children[childN].pipeWriteFd[1], STDOUT_FILENO);

        for (int i = FIRST_PIPE_FD; i <= LAST_PIPE_FD + 4 * childN ; ++i) {
            close(i);
        }
        if((execve(CHILDPATH,args,envp)) == ERROR){
            perror("Error while doing execve...");
            exit(1);
        }
    }
    children[childN].childPid = pidChild;
    return 0;
}


void passFilesToChild(int numFiles, int childIndex)
{

    if(sended >= nfiles|| childIndex >= SLAVES)
    {
        return;
    }
    if(sended + numFiles > nfiles)
    {
        numFiles = nfiles - sended;
    }

    size_t totalLength = 0;
    size_t filePathSize[numFiles];

    for (int i = 0; i < numFiles; ++i) {
        filePathSize[i] = strlen(files[sended + i]);

        totalLength += filePathSize[i];
    }
    totalLength += numFiles + 1;

 //   totalLength = strlen(files[sended]) + strlen(files[sended + 1]) + 3; // +1 para '|' y +1 para '\0'

    char *result = (char *)malloc(totalLength * sizeof(char));

    if (result == NULL) {
        perror("Error allocating memory");
        exit(1);
    }
    char *resultTemp = result;

    for (int i = 0; i < numFiles; ++i) {
        snprintf(resultTemp, filePathSize[i] +2, "%s|", files[sended + i]);
        resultTemp += filePathSize[i] + 1;
    }
    result[totalLength -1] = 0;

    // Usa snprintf para construir la nueva cadena
   // snprintf(result, totalLength, "%s|%s|", files[sended], files[sended + 1]);

    //write(children[childIndex].pipeReadFd[1], result, totalLength);

    if(write(children[childIndex].pipeReadFd[1], result, totalLength) == ERROR){
        perror("error while writing to child");
        exit(1);
    }
    free(result);
    sended += numFiles;
}
