#include "timer.h"

//struct timeval startingTime;
//struct timeval currentTime;
double elapsed;

void startTime() {
    elapsed = time(NULL);
}

double getElapsedTime() {
    return time(NULL)-elapsed;
}

