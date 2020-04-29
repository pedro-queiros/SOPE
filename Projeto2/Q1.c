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
    char fifo[256] = "/tmp/", pidInString[50], tidInString[50], infoToClient[50];
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
    }

    if(getElapsedTime() + dur*0.001 < workingTime){
        sprintf(infoToClient,"[%d, %d, %ld, %d, %d]",i, getpid(), pthread_self(),dur,toiletId);
    }
    else{
        sprintf(infoToClient,"[%d, %d, %ld, %d, %d]",i, getpid(), pthread_self(),-1,-1);
    }
    toiletId++;
    write(fd,&infoToClient,256);
    usleep(dur*1000);
    close(fd);
    return NULL;
}



int main(int argc, char* argv[], char* envp[]){

    if(argc != 4){
        fprintf(stderr, "Usage: Un <-t nsecs> fifoname\n");
    }

    startTime();

    int fd;
    char fifo[256];
    char info[256];
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
        if(read(fd,info,256) > 0 && info[0] == '['){
            printf("Recebi o pedido: %s\n", info);
            pthread_create(&thread,NULL,serverFunction,(void *)&info);
            pthread_detach(thread);
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
