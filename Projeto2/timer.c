#include "timer.h"

double getElapsedTime(){
    clock_t startingTime = clock();
    return startingTime / (CLOCKS_PER_SEC / (double) 1000.0);
}
