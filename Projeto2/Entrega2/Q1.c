#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include "timer.h"
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <semaphore.h>
#include "logs.h"

int workingTime = 0;
int toiletId = 1;
int opened = 1;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
int threadLimit = 0, placeLimit = 0;
sem_t threadSem;
sem_t placesSem;

void * serverFunction(void * info){
    int fd, dur, id, pid, clientIn = 0;
    long int tid;
    char fifo[MAX_LEN] = "/tmp/", pidInString[MAX_LEN] = {0}, tidInString[MAX_LEN] = {0}, infoToClient[MAX_LEN] = {0};
    sscanf((char*)info,"[ %d, %d, %ld, %d, -1]",&id,&pid,&tid,&dur);
    printToConsole(id, getpid(), pthread_self(), dur, -1, "RECVD");

    sprintf(pidInString,"%d",pid);
    strcat(fifo,pidInString);
    strcat(fifo,".");
    sprintf(tidInString,"%ld",tid);
    strcat(fifo,tidInString);

    if((fd = open(fifo, O_WRONLY)) < 0){
        printToConsole(id, getpid(), pthread_self(), dur, -1, "GAVUP");
        if(threadLimit)
            sem_post(&threadSem);
        return NULL;
    }

    if(placeLimit){
        sem_wait(&placesSem);
        pthread_mutex_lock(&mutex);
        pthread_mutex_unlock(&mutex);
    }
    else{
        pthread_mutex_lock(&mutex);
        pthread_mutex_unlock(&mutex);
    }

    if (opened) {
        if (getElapsedTime() + dur * 0.001 < workingTime) {
            clientIn = 1;
            sprintf(infoToClient, "[%d, %d, %ld, %d, %d]", id, getpid(), pthread_self(), dur, toiletId);
            printToConsole(id, getpid(), pthread_self(), dur, -1, "ENTER");
        } else {
            sprintf(infoToClient, "[%d, %d, %ld, %d, %d]", id, getpid(), pthread_self(), -1, -1);
            //printToConsole(id, getpid(), pthread_self(), dur, -1, "2LATE");
        }
        toiletId++;
    }
    else{
        printToConsole(id, getpid(), pthread_self(), dur, -1, "2LATE");
    }

    if(write(fd,&infoToClient,MAX_LEN) < 0){
        perror("Error Writing to Private Fifo\n");
        if(threadLimit)
            sem_post(&threadSem);
        if(placeLimit){
            pthread_mutex_lock(&mutex);
            pthread_mutex_unlock(&mutex);
            sem_post(&placesSem);
        }
        return NULL;
    }

    if (clientIn){
        usleep(dur*1000);
        printToConsole(id, getpid(), pthread_self(), dur, -1, "TIMUP");
    }

    if(close(fd) < 0){
        perror("Error Closing Private Fifo\n");
    }

    if(threadLimit)
        sem_post(&threadSem);
    if(placeLimit){
        pthread_mutex_lock(&mutex);
        pthread_mutex_unlock(&mutex);
        sem_post(&placesSem);
    }

    return NULL;
}

void checkArgs(int argc, char* argv[],int *nSecs, char* fifoName, int *nPlaces, int *nThreads){
    if(argc > 4 || argc > 8){
        printf("Usage: Qn <-t nsecs> [-l nplaces] [-n nthreads] fifoname");
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
                printf("Usage: Qn <-t nsecs> [-l nplaces] [-n nthreads] fifoname");
                exit(1);
            }
        }
        else{
            strcpy(fifoName,aux);
        }
    }
}


int main(int argc, char* argv[], char* envp[]){

    startTime();

    int fd;
    char fifo[MAX_LEN] = {0};
    int nPlaces = -1, nThreads = -1;
    char info[MAX_LEN] = {0};
    pthread_t thread;

    checkArgs(argc,argv,&workingTime,fifo,&nPlaces,&nThreads);
    if(nPlaces > 0){
        placeLimit = 1;
    }
    if(nThreads > 0){
        threadLimit = 1;
    }


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

    if(placeLimit)
        sem_init(&placesSem,0,nPlaces);

    while(getElapsedTime() <= workingTime){
        if(read(fd,info,MAX_LEN) > 0 && info[0] == '['){
            if(threadLimit)
                sem_wait(&threadSem);
            if(pthread_create(&thread,NULL,serverFunction,(void *)&info) != 0){
                perror("Error Creating Thread\n");
                break;
            }
            /*if(pthread_detach(thread) != 0){
                perror("Error Detaching Thread\n");
                break;
            }*/
        }
    }

    opened = 0;

    if(close(fd) < 0){
        perror("Error closing file");
    }

    if(unlink(fifo) < 0){
        perror("Error deleting Fifo");
    }

    exit(0);
}
