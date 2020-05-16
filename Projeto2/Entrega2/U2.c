#include <pthread.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdbool.h>
#include <errno.h>
#include "timer.h"
#include "logs.h"
#include "utils.h"

int id = 1;
int fd;
int opened = true;
char fifo_name[MAX_LEN] = {0};

void *thread_handler(void *arg){
    if(pthread_detach(pthread_self()) != 0){
        perror("Error Detaching Thread\n");
        return NULL;
    }
 
    int clientId = id;	
 
    char fifo_priv[MAX_LEN] = {0};
    sprintf(fifo_priv, "/tmp/%d.%ld", getpid(), pthread_self());

    if(mkfifo(fifo_priv, 0660) < 0){
        printToConsole(id,getpid(),pthread_self(),-1,-1,"FAILD");
        perror("Error Creating Private Fifo");
        return NULL;
    }
    char msg[MAX_LEN] = {0};
    long int dur = rand() % 900 + 300;

    sprintf(msg,"[ %d, %d, %ld, %ld, -1]",clientId,(int)getpid(),(long)pthread_self(),dur);
    
    printToConsole(clientId,getpid(),pthread_self(),dur,-1,"IWANT");	
    if(write(fd, &msg, MAX_LEN) < 0){
        printToConsole(id,getpid(),pthread_self(),-1,-1,"FAILD");
        perror("Error Writing to Public Fifo\n");
        //if(close(fd)<0) fprintf(stderr, "Cannot close public FIFO");
        if (unlink(fifo_priv) < 0) fprintf(stderr, "Cannot delete private FIFO");
        opened = false;
        return NULL;
    }


    int fd2;

    if ((fd2 = open(fifo_priv,O_RDONLY)) < 0){
        printToConsole(id,getpid(),pthread_self(),-1,-1,"FAILD");
        perror("Error Opening File");
        if(unlink(fifo_priv) < 0){
            perror("Error Deleting Fifo");
        }
        return NULL;
    }

    char answer[MAX_LEN];


    if(read(fd2, &answer, MAX_LEN) <= 0){
        printToConsole(id,getpid(),pthread_self(),-1,-1,"FAILD");
        perror("Error Reading Private Fifo\n");
        if(close(fd2) < 0){
            perror("Error Closing Private Fifo\n");
            if(unlink(fifo_priv) < 0){
                perror("Error Deleting Fifo");
            }
        }
        return NULL;
    }

    int num1, pid, place;
    long tid;
    sscanf(answer,"[%d, %d, %ld, %ld, %d]",&num1,&pid,&tid,&dur, &place);
    if(place==-1 && dur==-1){
        printToConsole(num1,getpid(),pthread_self(),dur,-1,"CLOSD");
        opened = false;
    }
    else
        printToConsole(num1, getpid(),pthread_self(), dur, place, "IAMIN");

    if(close(fd2) < 0){
        perror("Error Closing Private Fifo\n");
        if(unlink(fifo_priv) < 0){
            perror("Error Deleting Fifo");
        }
        return NULL;
    }

    if(unlink(fifo_priv) < 0){
        perror("Error Deleting Fifo");
    }
    return NULL;
}


int main(int argc, char* argv[], char* envp[]){

    int workingTime;
    checkClientArgs(argc,argv,&workingTime,fifo_name);

    if(workingTime <= 0){
        fprintf(stderr, "Time needs to be greater than zero\n");
        exit(3);
    }
    startTime();

    if((fd = open(fifo_name, O_WRONLY)) < 0){
        printToConsole(id,getpid(),pthread_self(),-1,-1,"FAILD");
        opened = false;
        fprintf(stderr, "Service is closed !\n");
        exit(4);
    }

    while(getElapsedTime() < workingTime && opened){
        pthread_t thread;
        if(pthread_create(&thread,NULL,thread_handler,NULL) != 0){
            perror("Error Creating Thread\n");
        }
        usleep(5*1000);
	    id++;
    }

    if(close(fd) < 0){
        perror("Error Closing Public Fifo\n");
    }

    pthread_exit(0);
}
