#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include "timer.h"
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>


int workingTime = 0;
int toiletId = 1;

void * serverFunction(void * info){
    int fd;
    char fifo[1024] = "/tmp/", pidInString[1024] = {0}, tidInString[1024] = {0}, infoToClient[1024] = {0};
    int i, dur, pid;
    long int tid;
    sscanf((char*)info,"[ %d, %d, %ld, %d, -1]",&i,&pid,&tid,&dur);

    sprintf(pidInString,"%d",pid);
    strcat(fifo,pidInString);
    strcat(fifo,".");
    sprintf(tidInString,"%ld",tid);
    strcat(fifo,tidInString);

    if((fd = open(fifo, O_WRONLY)) < 0){
        printf("Client gave up\n");
        return NULL;
    }

    if(getElapsedTime() + dur*0.001 < workingTime){
        sprintf(infoToClient,"[%d, %d, %ld, %d, %d]",i, getpid(), pthread_self(),dur,toiletId);
    }
    else{
        sprintf(infoToClient,"[%d, %d, %ld, %d, %d]",i, getpid(), pthread_self(),-1,-1);
    }
    toiletId++;
    if(write(fd,&infoToClient,1024) < 0){
        perror("Error Writing to Private Fifo\n");
        return NULL;
    }
    usleep(dur*1000);
    if(close(fd) < 0){
        perror("Error Closing Private Fifo\n");
        return NULL;
    }
    return NULL;
}



int main(int argc, char* argv[], char* envp[]){

    if(argc != 4){
        fprintf(stderr, "Usage: Un <-t nsecs> fifoname\n");
    }

    startTime();

    int fd;
    char fifo[1024] = {0};
    char info[1024] = {0};
    pthread_t thread;
    sscanf(argv[2],"%d",&workingTime);


    strcpy(fifo,argv[3]);

    if(mkfifo(fifo,0660) < 0){
        perror("Error creating Fifo");
    }

    if((fd = open(fifo,O_RDONLY | O_NONBLOCK)) < 0){
        perror("Error opening file");
        if(unlink(fifo) < 0){
            perror("Error deleting Fifo");
        }
    }

    while(getElapsedTime() < workingTime){
        if(read(fd,info,1024) > 0 && info[0] == '['){
            printf("Recebi o pedido: %s\n", info);
            if(pthread_create(&thread,NULL,serverFunction,(void *)&info) != 0){
                perror("Error Creating Thread\n");
                break;
            }
            if(pthread_detach(thread) != 0){
                perror("Error Detaching Thread\n");
                break;
            }
        }
    }

    if(close(fd) < 0){
        perror("Error closing file");
        if(unlink(fifo) < 0){
            perror("Error deleting Fifo");
        }
    }

    if(unlink(fifo) < 0){
        perror("Error deleting Fifo");
    }

    exit(0);
}
