#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void checkServerArgs(int argc, char* argv[],int *nSecs, char* fifoName, int *nPlaces, int *nThreads);

void checkClientArgs(int argc, char* argv[],int * workingTime, char * fifoName);