#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <sys/mman.h>
#include <stdbool.h>

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

void initializeFdSets();
void createOutputTxt();
void cleanUp();
void initializeSharedMem();
void initializeViewProcessData();
void initializeSlaves();
void checkChildOutput(int childIndex);

Child children[SLAVES];

fd_set readFds;
int nfds = 3 + SLAVES *4 +1;

struct shmbuf  *shmp;
int waitPidStatus;
int shm_fd = 0;
int outputFd;


char view = false;
char delimiters[] = "\n";
size_t stringToWriteStartOffset = 0;
char ** files;
int totalFiles;
size_t childOutputTokenLen;



int sended = 1; /* variable that counts number of files sended from parent to child. As the executable name is not wanted, we initialize it in 1.*/
int received = 1; /* variable that counts amount of files received again by parent --> this means they have been processeed already.    */

void passFilesToChild(int numFiles, int childIndex);

int main(int argc, char * argv[]){

   totalFiles = argc;
    files = argv;

    initializeSharedMem();
    createOutputTxt();
    initializeViewProcessData();


    initializeSlaves();




    for (int i = 0; i < SLAVES; ++i) {
        passFilesToChild(FILES_TO_CHILD,i);
    }



    while (received <totalFiles){
        initializeFdSets();
        select(nfds,&readFds,NULL,NULL,NULL);
        for (int k = 0; k < SLAVES; ++k) {
             if(FD_ISSET(children[k].pipeWriteFd[0],&readFds)) {
                 checkChildOutput(k);
                 passFilesToChild(1,k);
            }
        }
    }
    shmp->buf[stringToWriteStartOffset] = EOF;
    sem_post(&shmp->resultadoDisponible);

    cleanUp();


    return 0;
}

void initializeSlaves()
{
    for (int i = 0; i < SLAVES; ++i) {
        createChild(i);
    }

    for (int i = 0; i < SLAVES; ++i) {
        close(children[i].pipeReadFd[0]);
        close(children[i].pipeWriteFd[1]);
    }
}

void cleanUp() {

    for(int i = 0; i < SLAVES; i++){
        close(children[i].pipeReadFd[1]);
        close(children[i].pipeWriteFd[0]);
        waitpid(-1,&waitPidStatus,0);
    }
    close(outputFd);

    if (munmap(shmp, sizeof(*shmp)) == -1) {
        perror("munmap");
        exit(1);
    }
    close(shm_fd);
    fflush(stdout);
    close(STDOUT_FILENO);
    shm_unlink(SHM_PATH);
}

void initializeSharedMem()
{
    shm_fd = shm_open(SHM_PATH, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);

    if (shm_fd == ERROR) {
        errExit("Error in shm_open()");
    }

    if (ftruncate(shm_fd, sizeof(struct shmbuf)) == -1) {
        errExit("ftruncate");
    }

    shmp = mmap(NULL, sizeof(*shmp), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);

    if (shmp == MAP_FAILED) {
        errExit("mmap");
    }
    shmp->totalFiles = totalFiles;
}

void createOutputTxt(){
    outputFd = open("result.txt", O_WRONLY | O_CREAT, 0644);

    if (outputFd < 0) {
        perror("Error al abrir o crear el archivo");
        return;
    }
}

void initializeViewProcessData()
{
    if(shm_fd == 0)
    {
        perror("call to initializeSharedMem required");
        exit(1);
    }
    write(STDOUT_FILENO,SHM_PATH, PATH_LEN);
    shmp->totalFiles =totalFiles-1;


    if (sem_init(&shmp->resultadoDisponible, 1, 0) == -1) {
        errExit("sem_init-resultadoDisponible");
    }

    sleep(2);

    if(shmp->buf[0]){
        view = true;
    }
}

void checkChildOutput(int childIndex)
{
    ssize_t nRead;
    if((nRead = read(children[childIndex].pipeWriteFd[0], buff, 199)) == ERROR) {
        errExit("Error while reading from child");
    }
    buff[nRead] = 0;
    char *token = strtok(buff, delimiters);

    while (token!= NULL)
    {
        sprintf(shmp->buf + stringToWriteStartOffset,"Hijo con PID: %d produjo Md5: %s \n",children[childIndex].childPid,token);
        childOutputTokenLen = strlen(shmp->buf + stringToWriteStartOffset);
        write(outputFd,shmp->buf +stringToWriteStartOffset, childOutputTokenLen);
        sem_post(&shmp->resultadoDisponible);

        received++;
        token = strtok(NULL, delimiters);
        stringToWriteStartOffset += childOutputTokenLen + 1;
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

    if(sended >=totalFiles|| childIndex >= SLAVES)
    {
        return;
    }
    if(sended + numFiles >totalFiles)
    {
        numFiles =totalFiles - sended;
    }

    size_t totalLength = 0;
    size_t filePathSize[numFiles];

    for (int i = 0; i < numFiles; ++i) {
        filePathSize[i] = strlen(files[sended + i]);

        totalLength += filePathSize[i];
    }
    totalLength += numFiles + 1;

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

    if(write(children[childIndex].pipeReadFd[1], result, totalLength) == ERROR){
        perror("error while writing to child");
        exit(1);
    }
    free(result);
    sended += numFiles;
}
