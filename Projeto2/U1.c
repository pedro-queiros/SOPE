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
    char message[256];
    char msg[256];
    long int dur = rand() % 7000 + 1;
    sprintf(msg,"[ %d, %d, %ld, %ld, -1]",id,(int)getpid(),(long)pthread_self(),dur);

    //char *fifo_name = fifo;
    int fd;
    if((fd = open(fifo, O_WRONLY)) < 0){
        printf("Bathroom Closed\n");
        opened = false;
        return NULL;
    }
    write(fd, &msg, 256);
    printf("Tenho que ir cagar: %s\n", msg);
    close(fd);

    char fifo_priv[256] = "/tmp/", pidInString[50], tidInString[50];
    sprintf(pidInString, "%d", getpid());
    strcat(fifo_priv, pidInString);
    strcat(fifo_priv, ".");
    sprintf(tidInString, "%ld", pthread_self());
    strcat(fifo_priv, tidInString);

    if(mkfifo(fifo_priv, 0660) < 0){
        perror("Error Creating Private Fifo");
    }
    int fd2;
    if((fd2= open(fifo_priv, 0660)) < 0){
        perror("Error Opening File");
    }
    char answer[256];
    read(fd2, &answer, 256);
    printf("Resposta: %s\n", answer);
    close(fd2);
    if(unlink(fifo_priv) < 0){
        perror("Error deleting Fifo");
    }

    sprintf(message,"[%d, %ld, %ld]", (int)getpid(), (long)pthread_self(), dur);
    pthread_exit(message);
    return NULL;
}

int main(int argc, char* argv[], char* envp[]){
    if (argc != 4){
        fprintf(stderr, "Usage: Un <-t nsecs> fifoname\n");
    }

    startTime();

    double workingTime;
    sscanf(argv[2], "%lf", &workingTime);

    pthread_t threads[256];
    char fifo[256];
    //void* status;
    strcat(fifo, argv[3]);
    while(getElapsedTime() <= workingTime && opened){
        pthread_create(&threads[id],NULL,thread_handler,&fifo);
        pthread_join(threads[id],NULL);
        usleep(2000*1000);
        //printf("%s\n", (char *) status);
        id++;

        double time = getElapsedTime();
        printf("Elapsed Time: %f\n", time);
    }

    printf("Exiting...\n");

    pthread_exit(0);
}
