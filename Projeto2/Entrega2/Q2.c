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
#include "utils.h"

int workingTime = 0;
int toiletId = 1;
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

    if((fd = open(fifo_priv, O_WRONLY)) < 0){
        printToConsole(id, getpid(), pthread_self(), dur, -1, "GAVUP");
        if(threadLimit)
            sem_post(&threadSem);
        return NULL;
    }
    sprintf(infoToClient, "[%d, %d, %ld, %d, %d]", id, getpid(), pthread_self(), dur, place);
    printToConsole(id, getpid(), pthread_self(), dur, place, "ENTER");


    if(write(fd,&infoToClient,MAX_LEN) < 0){
        perror("Error Writing to Private Fifo\n");
        if(close(fd)<0) fprintf(stderr, "Error closing private fifo of the id: %d\n",id);
        printToConsole(id, getpid(), pthread_self(), dur, -1, "GAVUP");
        if(threadLimit)
            sem_post(&threadSem);
        if(placeLimit){
            pthread_mutex_lock(&mutex);
            releasePlace(&q,place);
            pthread_mutex_unlock(&mutex);
            sem_post(&placesSem);
        }
        return NULL;
    }
    usleep(dur*1000);
    printToConsole(id, getpid(), pthread_self(), dur, place, "TIMUP");


    if(close(fd) < 0){
        perror("Error Closing Private Fifo\n");
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
        fprintf(stderr, "Cannot open %s for Writing!\n", fifo_priv);
        printToConsole(id, getpid(), pthread_self(), dur, -1, "GAVUP");
        if(threadLimit)
            sem_post(&threadSem);
        return NULL;
    }
    sprintf(infoToClient, "[%d, %d, %ld, %d, %d]", id, getpid(), pthread_self(), -1, -1);
    if(write(fd,infoToClient,MAX_LEN) < 0){
        fprintf(stderr, "Cannot write to client %d FIFO!", id);
        printToConsole(id, getpid(), pthread_self(), dur, -1, "GAVUP");
        close(fd);
        if(threadLimit)
            sem_post(&threadSem);
        return NULL;
    }

    if(close(fd) < 0){
        perror("Closing Private fifo\n");
    }

    printToConsole(id, getpid(), pthread_self(), -1, -1, "2LATE");

    if(threadLimit)
        sem_post(&threadSem);
    return NULL;
}


int main(int argc, char* argv[], char* envp[]){

    int fd;
    char fifo[MAX_LEN] = {0};
    int nPlaces = -1, nThreads = -1;
    char info[MAX_LEN] = {0};


    checkServerArgs(argc,argv,&workingTime,fifo,&nPlaces,&nThreads);

    if(workingTime <= 0){
        fprintf(stderr, "Time needs to be greater than zero\n");
        exit(3);
    }

    if(nPlaces > 0){
        placeLimit = 1;
    }
    if(nThreads > 0){
        threadLimit = 1;
    }
    startTime();

    if(mkfifo(fifo,0660) < 0){
        perror("Error creating Fifo");
        exit(4);
    }

    if((fd = open(fifo,O_RDONLY | O_NONBLOCK)) < 0){
        perror("Error opening file");
        if(unlink(fifo) < 0){
            perror("Error deleting Fifo");
        }
        exit(5);
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
