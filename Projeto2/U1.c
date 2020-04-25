#include "timer.h"

#define MAX_COMMANDS 10
#define READ 0
#define WRITE 1

int main(int argc, char* argv[], char* envp[]){
    if (argc != 4){
        fprintf(stderr, "Usage: Un <-t nsecs> fifoname\n");
    }

    double workingTime;
    sscanf(argv[2], "%lf", &workingTime);

    printf("Time: %lf\n", workingTime);

    while(getElapsedTime() <= workingTime){
        double time = getElapsedTime();
        printf("%f\n", time);
    }

    printf("Exiting...\n");

    return 0;
}
