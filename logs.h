#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <time.h>
#include <string.h>

typedef enum {CREATE, EXIT, RECV_SIGNAL, SEND_SIGNAL, RECV_PIPE, SEND_PIPE, ENTRY} action;

struct log {
    double instant;
    pid_t pid;
    action action;
    char info[256];
};

void logHandler();
void writeToFile(struct log *log);
void createLog(struct log *log, action action);
void createLogs(int argc, char *argv[]);
void logExit (int stat);
void pipeReceivedLog(int msg);
void pipeSentLog(int msg);
void logEntry(int size, char* path);
void logSentSignal(int pid, char* sign);
void logReceivedSignal(int pid, char* sign);