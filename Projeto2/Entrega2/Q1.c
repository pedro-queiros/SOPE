#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include "timer.h"
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <semaphore.h>
#include "queue.h"
#include "logs.h"

int workingTime = 0;
int toiletId = 1;
int opened = 1;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
int threadLimit = 0, placeLimit = 0;
sem_t threadSem;
sem_t placesSem;
queue q;

void * serverFunction(void * info){
    if(pthread_detach(pthread_self()) != 0){
        perror("Error Detaching Thread\n");
        return NULL;
    }

    int fd, dur, id, pid;
    long int tid;
    char fifo_priv[MAX_LEN] = {0} , infoToClient[MAX_LEN] = {0};
    sscanf((char*)info,"[ %d, %d, %ld, %d, -1]",&id,&pid,&tid,&dur);
    printToConsole(id, getpid(), pthread_self(), dur, -1, "RECVD");
    sprintf(fifo_priv, "/tmp/%d.%ld", pid, tid);


    int place;
    if(placeLimit){
        sem_wait(&placesSem);
        pthread_mutex_lock(&mutex);
        place = occupyPlace(&q);
        pthread_mutex_unlock(&mutex);
    }
    else{
        pthread_mutex_lock(&mutex);
        place = toiletId;
        toiletId++;
        pthread_mutex_unlock(&mutex);
    }



    /*int tries = 0;
    while ((fd=open(fifo_priv,O_WRONLY|O_NONBLOCK)) < 0 && tries < 5) {
        fprintf(stderr, "Cant open private fifo %s\n",fifo_priv);
        usleep(200);
        tries++;
    }

    if (tries == 5) {
        //writeRegister(i, pid, tid, duration, -1, GAVEUP);
        if (threadLimit) sem_post(&threadSem);
        if (placeLimit) {
            pthread_mutex_lock(&mutex);
            releasePlace(&q,place);
            pthread_mutex_unlock(&mutex);
            sem_post(&placesSem);
        }
        //pthread_exit(NULL);
        return NULL;
    }*/
    if((fd = open(fifo_priv, O_WRONLY)) < 0){   //thread bloqueia aqui
        printToConsole(id, getpid(), pthread_self(), dur, -1, "GAVUP");
        if(threadLimit)
            sem_post(&threadSem);
        return NULL;
    }
    sprintf(infoToClient, "[%d, %d, %ld, %d, %d]", id, getpid(), pthread_self(), dur, place);
    printToConsole(id, getpid(), pthread_self(), dur, -1, "ENTER");


    if(write(fd,&infoToClient,MAX_LEN) < 0){
        perror("Error Writing to Private Fifo\n");
        if(close(fd)<0) fprintf(stderr, "Error closing private fifo of request %d\n",id);
        if(threadLimit)
            sem_post(&threadSem);
        if(placeLimit){
            pthread_mutex_lock(&mutex);
            releasePlace(&q,place);
            pthread_mutex_unlock(&mutex);
            sem_post(&placesSem);
        }
        //pthread_exit(NULL);
        return NULL;
    }
    usleep(dur*1000);
    printToConsole(id, getpid(), pthread_self(), dur, -1, "TIMUP");


    if(close(fd) < 0){
        perror("Error Closing Private Fifo\n");
        //pthread_exit(NULL);
        return NULL;
    }

    if(threadLimit){
        sem_post(&threadSem);
    }

    if(placeLimit){
        pthread_mutex_lock(&mutex);
        releasePlace(&q,place);
        pthread_mutex_unlock(&mutex);
        sem_post(&placesSem);
    }

    return NULL;
    //pthread_exit(NULL);
}

void checkArgs(int argc, char* argv[],int *nSecs, char* fifoName, int *nPlaces, int *nThreads){
    if(argc < 4 || argc > 8){
        printf("Usage: Qn <-t nsecs> [-l nplaces] [-n nthreads] fifoname\n");
        exit(1);
    }
    for (int i = 1; i < argc; i++){
        char * aux = argv[i];
        if(aux[0] == '-'){
            if(strcmp(aux,"-t") == 0){
                *nSecs = atoi(argv[i+1]);
                i++;
            }
            else if(strcmp(aux,"-l") == 0){
                *nPlaces = atoi(argv[i+1]);
                i++;
            }
            else if(strcmp(aux,"-n") == 0){
                *nThreads = atoi(argv[i+1]);
                i++;
            }
            else{
                printf("Usage: Qn <-t nsecs> [-l nplaces] [-n nthreads] fifoname\n");
                exit(1);
            }
        }
        else{
            strcpy(fifoName,aux);
        }
    }
}

void * clearFifo(void *info){
    pthread_detach(pthread_self());

    int id, pid, dur, fd;
    long int tid;
    char fifo_priv[MAX_LEN] = {0}, infoToClient[MAX_LEN] = {0};
    sscanf((char*)info,"[ %d, %d, %ld, %d, -1]",&id,&pid,&tid,&dur);

    printToConsole(id, getpid(), pthread_self(), dur, -1, "RECVD");

    sprintf(fifo_priv, "/tmp/%d.%ld", pid, tid);
    if((fd = open(fifo_priv,O_WRONLY)) < 0){
        fprintf(stderr, "Cannot open %s for WRITING!\n", fifo_priv);
        if(threadLimit)
            sem_post(&threadSem);
        //pthread_exit(NULL);
        return NULL;
    }
    sprintf(infoToClient, "[%d, %d, %ld, %d, %d]", id, getpid(), pthread_self(), -1, -1);
    if(write(fd,infoToClient,MAX_LEN) < 0){
        fprintf(stderr, "Cannot write to client %d FIFO!", id);
        close(fd);
        if(threadLimit)
            sem_post(&threadSem);
        //pthread_exit(NULL);
        return NULL;
    }

    if(close(fd) < 0){
        perror("Closing Private fifo\n");
    }

    printToConsole(id, getpid(), pthread_self(), -1, -1, "2LATE");

    if(threadLimit)
        sem_post(&threadSem);
    //pthread_exit(NULL);
    return NULL;
}


int main(int argc, char* argv[], char* envp[]){

    int fd;
    char fifo[MAX_LEN] = {0};
    int nPlaces = -1, nThreads = -1;
    char info[MAX_LEN] = {0};


    checkArgs(argc,argv,&workingTime,fifo,&nPlaces,&nThreads);

    if(nPlaces > 0){
        placeLimit = 1;
    }
    if(nThreads > 0){
        threadLimit = 1;
    }
    startTime();

    if(mkfifo(fifo,0660) < 0){
        perror("Error creating Fifo");
        exit(2);
    }

    if((fd = open(fifo,O_RDONLY | O_NONBLOCK)) < 0){
        perror("Error opening file");
        if(unlink(fifo) < 0){
            perror("Error deleting Fifo");
        }
        exit(3);
    }

    if(threadLimit)
        sem_init(&threadSem,0,nThreads);

    if(placeLimit){
        sem_init(&placesSem,0,nPlaces);
        q = createQueue(nPlaces);
        createPlaces(&q);
    }


    while(getElapsedTime() < workingTime){
        if(read(fd,info,MAX_LEN) > 0 && info[0] == '['){
            if(threadLimit){
                sem_wait(&threadSem);
            }
            char *aux;
            aux = strdup(info);
            pthread_t thread;
            if(pthread_create(&thread,NULL,serverFunction,aux) != 0){
                perror("Error Creating Thread\n");
                break;
            }
        }
    }
    opened = 0;

    if(unlink(fifo) < 0){
        perror("Error deleting Fifo");
    }

    while (read(fd, info,MAX_LEN) > 0) {
        if (info[0] == '[') {
            if (threadLimit)
                sem_wait(&threadSem);
            char * message;
            message = strdup(info);
            pthread_t thread;
            if(pthread_create(&thread,NULL,clearFifo,message) != 0){
                perror("Error Creating Thread\n");
                break;
            }
        }
    }

    if(close(fd) < 0){
        perror("Error closing file");
    }

    pthread_exit(0);
}
