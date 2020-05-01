#include <pthread.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdbool.h>
#include "timer.h"

int id = 1;
int opened = true;

void *thread_handler(void *fifo){
    //char message[256];
    char msg[1024] = {0};
    srand(time(NULL));
    long int dur = rand() % 7000 + 1;
    sprintf(msg,"[ %d, %d, %ld, %ld, -1]",id,(int)getpid(),(long)pthread_self(),dur);

    char fifo_priv[1024] = "/tmp/", pidInString[1024] = {0}, tidInString[1024] = {0};
    sprintf(pidInString, "%d", getpid());
    strcat(fifo_priv, pidInString);
    strcat(fifo_priv, ".");
    sprintf(tidInString, "%ld", pthread_self());
    strcat(fifo_priv, tidInString);

    if(mkfifo(fifo_priv, 0660) < 0){
        perror("Error Creating Private Fifo");
    }

    char *fifo_name = (char *)fifo;
    int fd;
    if((fd = open(fifo_name, O_WRONLY)) < 0){
        printf("Bathroom Closed\n");
        opened = false;
        return NULL;
    }
    if(write(fd, &msg, 1024) < 0){
        perror("Error Writing to Public Fifo\n");
        return NULL;
    }

    printf("Enviei pedido: %s\n", msg);
    if(close(fd) < 0){
        perror("Error Closing Public Fifo\n");
        return NULL;
    }

    int fd2;

    if ((fd2 = open(fifo_priv,O_RDONLY)) < 0){
        perror("Error Opening File");
    }

    char answer[1024];
    if(read(fd2, &answer, 1024) < 0){
        perror("Error Reading Private Fifo\n");
        return NULL;
    }

    printf("Resposta: %s\n", answer);
    if(close(fd2) < 0){
        perror("Error Closing Private Fifo\n");
        return NULL;
    }

    if(unlink(fifo_priv) < 0){
        perror("Error Deleting Fifo");
        return NULL;
    }

    //sprintf(message,"[%d, %ld, %ld]", (int)getpid(), (long)pthread_self(), dur);
    //pthread_exit(message);
    return NULL;
}

int main(int argc, char* argv[], char* envp[]){
    if (argc != 4){
        fprintf(stderr, "Usage: Un <-t nsecs> fifoname\n");
    }

    startTime();

    double workingTime;
    sscanf(argv[2], "%lf", &workingTime);

    //pthread_t threads[512];
    pthread_t thread;
    char fifo[1024] = {0};
    //void* status;
    strcat(fifo, argv[3]);
    while(getElapsedTime() <= workingTime && opened){
        if(pthread_create(&thread,NULL,thread_handler,&fifo) != 0){
            perror("Error Creating Thread\n");
            break;
        }
        if(pthread_detach(thread) != 0){
            perror("Error Detaching Thread\n");
            break;
        }
        usleep(50*1000);
        id++;

        double time = getElapsedTime();
        printf("Elapsed Time: %f\n", time);
    }

    printf("Exiting...\n");

    exit(0);
}
