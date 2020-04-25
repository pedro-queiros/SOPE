#include "timer.h"

struct timeval startingTime;
struct timeval currentTime;
double elapsed;

void startTime() {
    gettimeofday(&startingTime, NULL);
    elapsed = 0;
}

double getElapsedTime() {
    gettimeofday(&currentTime, NULL);
    return (((currentTime.tv_sec - startingTime.tv_sec) * 1000000) + (currentTime.tv_usec - startingTime.tv_usec)) * 0.000001;
}

