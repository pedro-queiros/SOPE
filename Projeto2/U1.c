#include <pthread.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "timer.h"

int id = 1;

void *thread_handler(void *fifo){
    char message[256];
    char msg[256];
    int duration = rand() % 25 + 1;
    sprintf(msg,"[ %d, %d, %ld, %d, -1]",id,(int)getpid(),(long)pthread_self(),duration);

    char *fifo_name = fifo;
    int fd = open(fifo_name, O_WRONLY);
    if (fd == -1){
        printf("Closed Bathroom\n");
        return NULL;
    }
    write(fd, &msg, 256);
    close(fd);

    char fifo_priv[256] = "/tmp/", buff[256];
    sprintf(buff, "%d", getpid());
    strcat(fifo_priv, buff);
    strcat(fifo_priv, ".");
    sprintf(buff, "%ld", pthread_self());
    strcat(fifo_priv, buff);

    mkfifo(fifo_priv, 0660);
    int fd2 = open(fifo_priv, 0660);
    if (fd2 == -1){
        printf("nao leu\n");
    }
    char answer[256];
    read(fd2, &answer, 256);
    close(fd2);
    unlink(fifo_priv);

    sprintf(message,"[%d, %ld, %d]", (int)getpid(), (long)pthread_self(), duration);
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
    void* status;
    strcat(fifo, argv[3]);
    while(getElapsedTime() <= workingTime){
        pthread_create(&threads[id],NULL,thread_handler,&fifo);
        pthread_join(threads[id],&status);
        usleep(2000*1000);
        printf("%s\n", (char *) status);
        id++;

        double time = getElapsedTime();
        printf("Elapsed Time: %f\n", time);
    }

    printf("Exiting...\n");

    return 0;
}
