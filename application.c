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
#define SLAVES 3        /* Fixed slave number */
#define ERROR (-1)        /* ERROR code */
#define FIRST_PIPE_FD 3  /* By default, pipe read is on 3 */
#define LAST_PIPE_FD 6   /* By default, after creating 2 pipes, second pipe's write is on 6 */
#define SHM_PATH "/sharedMem"
#define PATH_LEN 11

typedef struct
{
    int pipeWriteFd[2]; /* Child WRITES */ //parent READS from thisPipe[0]
    int pipeReadFd[2];  /* Child READS */ //parent WRITES to thisPipe[1]
    int childPid;
}Child;

char buff[200];

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

int main(int argc, char * argv[]){

    int shm_fd = shm_open(SHM_PATH, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);

    if (shm_fd == ERROR) {
        errExit("Error in shm_open()");
    }

    //    int outputFd = open("output.txt",O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
    createOutputTxt();

    puts("");

    if (ftruncate(shm_fd, sizeof(struct shmbuf)) == -1) {
        errExit("ftruncate");
    }

   shmp = mmap(NULL, sizeof(*shmp), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);

    if (shmp == MAP_FAILED) {
        errExit("mmap");
    }

    write(STDOUT_FILENO,SHM_PATH, PATH_LEN);
    shmp->totalFiles = argc-1;


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

    int sended = 1; /* variable that counts number of files sended from parent to child. As the executable name is not wanted, we initialize it in 1.*/
    int received = 1; /* variable that counts amount of files received again by parent --> this means they have been processeed already.    */

    /* (sended - 1) is done to identify the child's index that actually starts in 0 */

    char delim = '|' ;
    int i = 0;
    while (sended < argc && i < SLAVES){

        // Calcula el tamaño necesario para la nueva cadena
        size_t totalLength = strlen(argv[sended]) + strlen(argv[sended + 1]) + 3; // +1 para '|' y +1 para '\0'

        // Reserva espacio para la nueva cadena
        char *result = (char *)malloc(totalLength * sizeof(char));

        if (result == NULL) {
            perror("Error allocating memory");
            return 1;
        }

        // Usa snprintf para construir la nueva cadena
        snprintf(result, totalLength, "%s|%s|", argv[sended], argv[sended + 1]);

        write(children[i].pipeReadFd[1], result, totalLength);
//        write(children[i].pipeReadFd[1],&delim, 1);

        // Libera la memoria reservada
        free(result);
        sended += 2;


        i++;
    }
    size_t n;


    while (received < argc){

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
//                    if(view){
//                        sem_wait(&shmp->resultadoLeido); // the second child that finish will wait until the first one finish
//                    }

                    sprintf(shmp->buf + stringToWriteStartOffset,"Hijo con PID: %d produjo Md5: %s \n",children[k].childPid,token);
                    n = strlen(shmp->buf + stringToWriteStartOffset);
                    write(outputFd,shmp->buf +stringToWriteStartOffset, n);

                    //     printf("%s",shmp->buf);

                    if(view){
                        sem_post(&shmp->resultadoDisponible);
                    }
                    received++;
                    token = strtok(NULL, delimiters);
                    stringToWriteStartOffset += n;
                }


                /* FD_ISSET me dice si leer de el fd en cuestion me va a bloquear
                 * si me devuelve TRUE es que no me bloquea, quiere decir que tengo algo para leer
                 * osea que el hijo me escribio algo por el pipe
                 * si me da false es porque no me mando nada y no tengo que leer */


                /* Write solo bloquearia si el pipe esta lleno que creo que no nos va a pasar
                asi que no chequeo */
                if(sended < argc)
                {
                    if(write(children[k].pipeReadFd[1], argv[sended], strlen(argv[sended])) == ERROR){
                        perror("error while writing to child");
                        exit(1);
                    }
                    sended++;
                }
            }
        }
    }

//ultimoFileEOF

   // char bufffffff[1000];
   // sprintf(bufffffff,"application startOffset %zu \n",stringToWriteStartOffset);
   // shmp->buf[0] = EOF; // le mandamos de a uno los archivos
   // write(STDOUT_FILENO,bufffffff,1000);

    sem_post(&shmp->resultadoDisponible);

    // ACA verdaderamente lo estoy matando porque ya no lo uso mas
    close(children[0].pipeReadFd[1]);
    waitpid(-1, &status, 0);


    printf("Todos han terminado");

    if (munmap(shmp, sizeof(*shmp)) == -1) {
        perror("munmap");
        exit(1);
    }

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

//void createShm_Fd(){
//    int shm_fd = shm_open(SHM_PATH, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
//
//    if (shm_fd == ERROR) {
//        errExit("Error in shm_open()");
//    }
//}


//la parte de write la saque porque no deberia bloquear nada. porque siempre voy a poder escribir
//el tema es si los hijos me mandaron algo o no
void initializeFdSets()
{
    /* Note well: Upon return, each of the file descriptor sets is modified
         * in place to indicate which file descriptors are currently "ready".
         * Thus, if using select() within  a  loop,  the  sets  must  be
       reinitialized before each call.*/

    /*This macro clears (removes all file descriptors from) set.
     * It should be employed as the first step in initializing a file descriptor set. */
   // FD_ZERO(&writeFds);
    FD_ZERO(&readFds);

    for (int k = 0; k < SLAVES; ++k) {
        /* This macro adds the file descriptor fd to set. */
        //FD_SET(children[k].pipeReadFd[1],&writeFds);
        FD_SET(children[k].pipeWriteFd[0],&readFds);
    }
}

int getChildIndex(int childPid)
{
    for (int i = 0; i < SLAVES; ++i) {
        if(childPid == children[i].childPid)
        {
            return i;
        }
    }
    return -1;
}

int createChild(int childN)
{
    char * CHILDPATH = "./child";
    char * text = "rojo";

    if(pipe(children[childN].pipeReadFd) == ERROR){
        perror("Error while creating pipe");
        exit(1);
    }
    if(pipe(children[childN].pipeWriteFd) == ERROR){
        perror("Error while creating pipe");
        exit(1);
    }

    /* 0,1,2 -> TTy 3-> pipeRead.read  4 -> pipeRead.write 5->pipeWrite.read 6 -> pipeWrite.Write
    *  */

    int pidChild;

    if((pidChild = fork()) < 0){
        perror("Error while creating child");
        exit(1);
    }
    else if (pidChild == 0){ /* child process */
        char * args[] = {CHILDPATH, text, NULL};
        char *envp[] = {NULL};


        /* Child sets his own STDIN and STDOUT*/

        dup2(children[childN].pipeReadFd[0], STDIN_FILENO);
        dup2(children[childN].pipeWriteFd[1], STDOUT_FILENO);

        /* Child closes what he does not need */

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


