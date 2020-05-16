#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

void checkServerArgs(int argc, char* argv[],int *nSecs, char* fifoName, int *nPlaces, int *nThreads);

void checkClientArgs(int argc, char* argv[],int * workingTime, char * fifoName);

int checkIfOpen(char * fifoName);