#include "logs.h"
#include "timer.h"

void printToConsole(int i, int pid, long int tid, int dur, int pl, char oper[100]){
    printf("%f ; %d ; %d ; %ld ; %d ; %d ; %s\n", getElapsedTime(), i, pid, tid, dur, pl, oper);
}