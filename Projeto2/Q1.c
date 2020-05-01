#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include "timer.h"
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "logs.h"

int workingTime = 0;
int toiletId = 1;
int opened = 1;

void * serverFunction(void * info){
    int fd, dur, id, pid;
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
        return NULL;
    }

    if(getElapsedTime() + dur*0.001 < workingTime){
        sprintf(infoToClient,"[%d, %d, %ld, %d, %d]",id, getpid(), pthread_self(),dur,toiletId);
        printToConsole(id, getpid(), pthread_self(), dur, -1, "ENTER");
    }
    else{
        sprintf(infoToClient,"[%d, %d, %ld, %d, %d]",id, getpid(), pthread_self(),-1,-1);
        printToConsole(id, getpid(), pthread_self(), dur, -1, "2LATE");
        opened = 0;
    }

    toiletId++;
    if(write(fd,&infoToClient,MAX_LEN) < 0){
        perror("Error Writing to Private Fifo\n");
        return NULL;
    }
    usleep(dur*1000);
    printToConsole(id, getpid(), pthread_self(), dur, -1, "TIMUP");
    if(close(fd) < 0){
        perror("Error Closing Private Fifo\n");
    }
    return NULL;
}



int main(int argc, char* argv[], char* envp[]){

    if(argc != 4){
        fprintf(stderr, "Usage: Un <-t nsecs> fifoname\n");
    }

    startTime();

    int fd;
    char fifo[MAX_LEN] = {0};
    char info[MAX_LEN] = {0};
    pthread_t thread;
    sscanf(argv[2],"%d",&workingTime);


    strcpy(fifo,argv[3]);

    if(mkfifo(fifo,0660) < 0){
        perror("Error creating Fifo");
        exit(1);
    }

    if((fd = open(fifo,O_RDONLY | O_NONBLOCK)) < 0){
        perror("Error opening file");
        if(unlink(fifo) < 0){
            perror("Error deleting Fifo");
        }
        exit(2);
    }

    while(getElapsedTime() <= workingTime){
        if(read(fd,info,MAX_LEN) > 0 && info[0] == '['){
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
    }

    if(unlink(fifo) < 0){
        perror("Error deleting Fifo");
    }

    exit(0);
}
