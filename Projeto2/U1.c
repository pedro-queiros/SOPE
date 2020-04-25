#include <pthread.h>
#include <string.h>
#include <stdlib.h>
#include "timer.h"

void *thread_handler(void *arg){
    char message[256];
    int duration = rand() % 25 + 1;

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

    printf("Time: %lf\n", workingTime);

    int index = 0;
    pthread_t threads[256];
    char fifo[20];
    void* status;
    strcat(fifo, argv[3]);
    while(getElapsedTime() <= workingTime){
        pthread_create(&threads[index],NULL,thread_handler,&fifo);
        pthread_join(threads[index],&status);
        usleep(2000*1000);
        printf("%s\n", (char *) status);
        index++;

        double time = getElapsedTime();
        printf("Elapsed Time: %f\n", time);
    }

    printf("Exiting...\n");

    return 0;
}
