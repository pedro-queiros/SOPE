#include "logs.h"

FILE * logFile;

void logHandler(){
    setenv("LOG_FILENAME", "log.txt", 0);
    logFile = fopen(getenv("LOG_FILENAME"), "a");
    if(logFile == NULL) {
        perror("Error opening file");
        exit(1);
    }
}

void writeToFile(struct log *log){
    char* action;
    switch (log->action){
        case CREATE: action = "CREATE"; break;
        case EXIT: action = "EXIT"; break;
        case RECV_SIGNAL: action = "RECV_SIGNAL"; break;
        case SEND_SIGNAL: action = "SEND_SIGNAL"; break;
        case RECV_PIPE: action = "RECV_PIPE"; break;
        case SEND_PIPE: action = "SEND_PIPE"; break;
        case ENTRY: action = "ENTRY"; break;
    }
    fprintf(logFile, "%.2f - %d - %-11s - %s\n", log->instant, log->pid, action, log->info);
    setbuf(logFile,NULL);
}

void createLog(struct log *log, action action) {
    clock_t time = clock();
    log->instant = time/(CLOCKS_PER_SEC / (double) 1000.0);
    log->pid = getpid();
    log->action = action;
    strncpy(log->info,"Command: ", sizeof("Command: "));
}

void createLogs(int argc, char *argv[]){
    struct log log;
    createLog(&log, CREATE);
    int x;

    for (x = 0; x < argc; x++){
        strcat(log.info, argv[x]);
        if (x != argc-1)
            strcat(log.info," ");
    }

    writeToFile(&log);
}

void logExit (int stat){
    struct log log;
    createLog(&log, EXIT);
    sprintf(log.info, "Exit Status: %d",stat);
    writeToFile(&log);
    fclose(logFile);
    exit(stat);
}

void pipeReceivedLog(int msg){
    struct log log;
    createLog(&log, RECV_PIPE);
    sprintf(log.info, "Received Size: %d",msg);
    writeToFile(&log);
}


void pipeSentLog(int msg){
    struct log log;
    createLog(&log, SEND_PIPE);
    sprintf(log.info,"Sent Size: %d",msg);
    writeToFile(&log);
}


void logEntry(int size, char* path){
    struct log log;
    createLog(&log, ENTRY);
    sprintf(log.info,"Size: %d\tPath: %s", (int) size, path);
    writeToFile(&log);
}

void logSentSignal(int pid, char* sign){
    struct log log;
    createLog(&log, SEND_SIGNAL);
    sprintf(log.info, "Sent Signal %s to %d", sign, pid);
    writeToFile(&log);
}

void logReceivedSignal(int pid, char* sign){
    struct log log;
    createLog(&log, RECV_SIGNAL);
    sprintf(log.info, "Received Signal: %s", sign);
    writeToFile(&log);
}