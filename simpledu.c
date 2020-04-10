#include "logs.h"
#include <sys/stat.h>
#include <errno.h>
#include <dirent.h>
#include <fcntl.h>
#include <string.h>
#include <stdbool.h>
#include <dirent.h>
#include <limits.h>
#include <sys/wait.h>
#include "args.h"
#include "utils.h"

#define MAX_COMMANDS 10
#define READ 0
#define WRITE 1

char *possibleFlags[13] = {"-a","--all","-b","--bytes","-B","--block-size","-l","--count-links","-L","--dereference","-S","--separate-dirs","--max-depth"};

char initialDir[256];

struct flags f = {false,false,0,false,false,false,-1};

bool getNumber(char* cmd){
    char* max_d = "--max-depth=";
    char* blockSize = "--block-size=";

    if (strncmp(cmd, blockSize,13) == 0){
        cmd = strtok(cmd, "=");
        f.blockSize = atoi(strtok(NULL, "="));
    }

    if (strncmp(cmd, max_d, 12) == 0){
        cmd = strtok(cmd, "=");
        f.depth = atoi(strtok(NULL, "="));
        if (f.depth < 0)
            return true;
    }

    return false;
}

bool checkFlags(int size, char* argv[], struct flags *f){
    bool errors = false;

    for (int i = 0; i < size ; ++i) {
        if(strcmp(argv[i],possibleFlags[0]) == 0 || strcmp(argv[i],possibleFlags[1]) == 0) {
            f->all = true;
        }
        else if(strcmp(argv[i],possibleFlags[2]) == 0 || strcmp(argv[i],possibleFlags[3]) == 0)
            f->bytes = true;
        else if(strcmp(argv[i],possibleFlags[4]) == 0 || strcmp(argv[i],possibleFlags[5]) == 0) {
            if (!isNumeric(argv[i+1]) || (i == (size-1)))
                errors = true;
            else
                f->blockSize = atoi(argv[i + 1]);
        }
        else if(strcmp(argv[i],possibleFlags[6]) == 0 || strcmp(argv[i],possibleFlags[7]) == 0)
            f->countLinks = true;
        else if(strcmp(argv[i],possibleFlags[8]) == 0 || strcmp(argv[i],possibleFlags[9]) == 0)
            f->dereference = true;
        else if(strcmp(argv[i],possibleFlags[10]) == 0 || strcmp(argv[i],possibleFlags[11]) == 0)
            f->separate = true;
        else{
            errors = getNumber(argv[i]);
        }
    }

    return errors;
}

void printToConsole(int size, char* filepath){
    if(f.depth != -1){
        char path[256];
        getSubString(filepath, path, strlen(initialDir) + 1, strlen(filepath));
        if (numTimesAppears(path, '/') >= f.depth) return;
    }

    printf("%d",size);
    int count = 0, x;
    do
    {
        count++;
        size /= 10;
    } while(size != 0);

    count = 8 - count;

    for (x = 0; x < count; x++){
        printf(" ");
    }

    printf("%s\n", filepath);

}

void printTotal(int size, char* filepath){
    if(f.depth != -1 && strcmp(filepath, initialDir)){
        char path[256];
        getSubString(filepath, path, strlen(initialDir)+1, strlen(filepath));
        if (numTimesAppears(path, '/') >= f.depth) return;
    }
    
    printf("%d",size);
    int count = 0, x;
    do
    {
        count++;
        size /= 10;
    } while(size != 0);

    count = 8 - count;

    for (x = 0; x < count; x++){
        printf(" ");
    }

    printf("%s\n", filepath);
}

int readRegBlocks (char* filepath){
    struct stat file;
    int size, res;

    if (f.dereference)
        res = stat(filepath, &file);
    else
        res = lstat(filepath, &file);

    if (res < 0) {
        printf("Error reading file stat.\n");
        exit(1);
    }

    if (f.bytes)
        size = file.st_size;
    else
        size = file.st_blocks/2;

    if (f.blockSize) {
        //size = file.st_blocks / 2;
        //int size2 = file.st_size;
        int size2 = size;
        double result = size * (1024.0 / f.blockSize);
        size = (int) result;
        if (result - size > 0)
            size = result + 1;
        if (f.all && !S_ISDIR(file.st_mode)) {
            printToConsole(size, filepath);
            logEntry(size, filepath);
        }
        return size2;
    }

    if (f.all && !S_ISDIR(file.st_mode)) {
        printToConsole(size, filepath);
        logEntry(size, filepath);
    }

    return size;
}

int parentId;
int groupId;

void sigcont_handler (int sign){
    printf("BOAS\n");
    char signal[10];
    strcpy(signal, "SIGCONT");
    logReceivedSignal(getpid(), signal);
}

void sigterm_handler (int sign){
    char signal[10];
    strcpy(signal, "SIGTERM");
    logReceivedSignal(getpid(), signal);
}

void sigstop_handler (int sign){
    char signal[10];
    strcpy(signal, "SIGSTOP");
    logReceivedSignal(getpid(), signal);
}

void sigint_handler(int sign) {
    char input;
    char signal[10];
    kill(-groupId,SIGSTOP);
    printf("\nDo you want to terminate the program (Y/N):\n");
    scanf("%c",&input);
    if(input == 'Y' || input == 'y'){
        kill(getpid(),SIGTERM);
        strcpy(signal, "SIGTERM");
    }
    else if(input == 'N' || input == 'n'){
        kill(-groupId,SIGCONT);
        strcpy(signal, "SIGCONT");
    }
    else{
        printf("Invalid input\n");
        return;
    }
    logSentSignal(getpid(), signal);
    printf("Exiting\n");
}

int readDir (char* path, int argc, char* argv[]){
    DIR* dr;
    struct dirent *dir;
    char filepath[256];
    struct stat check;
    int size = 0, status, aux = 0;
    int fd[1];
    pid_t pid;

    if ((dr = opendir(path)) == NULL){
        perror(path);
        logExit(1);
    }
    while((dir = readdir(dr)) != 0){
        if(strcmp(dir->d_name,".") == 0|| strcmp(dir->d_name,"..") == 0)
            continue;
        strcpy (filepath, path);
        strcat (filepath, "/");
        strcat (filepath, dir->d_name);
        stat(filepath,&check);
        if(S_ISDIR(check.st_mode)){
            pipe(fd);
            pid = fork();
            if (pid == 0){      //processo-filho
                struct sigaction cont_act;
                cont_act.sa_handler = sigcont_handler;
                sigemptyset(&cont_act.sa_mask);
                cont_act.sa_flags = 0;

                if(sigaction(SIGCONT,&cont_act,NULL) < 0){
                    fprintf(stderr,"Unable to install SIGINT handler\n");
                    logExit(4);
                }

                struct sigaction term_act;
                term_act.sa_handler = sigterm_handler;
                sigemptyset(&term_act.sa_mask);
                term_act.sa_flags = 0;

                if(sigaction(SIGTERM,&term_act,NULL) < 0){
                    fprintf(stderr,"Unable to install SIGTERM handler\n");
                    logExit(4);
                }
                createLogs(argc, argv);
                int childSize = 0;
                struct sigaction act;
                act.sa_handler = SIG_IGN;
                sigemptyset(&act.sa_mask);
                act.sa_flags = 0;
                if(parentId != getpid()){
                    sigaction(SIGINT,&act,NULL);
                }
                childSize += readDir(filepath, argc, argv);
                close(fd[READ]);
                write(fd[WRITE],&childSize,sizeof(int));
                pipeSentLog(childSize);
                close(fd[WRITE]);
                logExit(0);
            }
            else if (pid > 0){      //processo-pai
                while (waitpid(-1,&status,0) != -1);
                close(fd[WRITE]);
                read(fd[READ],&aux,sizeof(int));
                pipeReceivedLog(aux);
                close(fd[READ]);
                if(!f.separate)
                    size += aux;
            }
        }
        else{
            size += readRegBlocks(filepath);
        }
    }

    size += readRegBlocks(path);

    if (f.blockSize) {
        int size2 = size;
        double result = size * (1024.0 / f.blockSize);
        size = (int) result;
        if (result - size > 0)
            size = result + 1;
        printTotal(size,path);
        logEntry(size, path);
        return size2;
    }

    printTotal(size,path);
    logEntry(size, path);
    sleep(3);
    return size;
}

int main(int argc, char* argv[], char* envp[]){
    logHandler();
    struct stat stat_buff;
    DIR *dr;
    char path[256];
    if((argc > 8) || strcmp(argv[1], "-l")){
        fprintf(stderr, "Usage: %s -l [path] [-a] [-b] [-B size] [-L] [-S] [--max-depth=N]\n", argv[0]);
        logExit(1);
    }

    strcpy(path,argv[2]);
    strcpy(initialDir, argv[2]);


    if (checkFlags(argc,argv,&f)){
        printf("Non valid set of arguments\n");
        logExit(1);
    }
    createLogs(argc, argv);

    if(stat(argv[2],&stat_buff) == -1){
        printf("simpledu: cannot access '%s': No such file or directory\n", argv[2]);
        logExit(3);
    }

    parentId = getpid();

    struct sigaction act;
    act.sa_handler = sigint_handler;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;

    if(sigaction(SIGINT,&act,NULL) < 0){
        fprintf(stderr,"Unable to install SIGINT handler\n");
        logExit(4);
    }

    struct sigaction term_act;
    term_act.sa_handler = sigterm_handler;
    sigemptyset(&term_act.sa_mask);
    term_act.sa_flags = 0;

    if(sigaction(SIGTERM,&term_act,NULL) < 0){
        fprintf(stderr,"Unable to install SIGTERM handler\n");
        logExit(4);
    }

    pid_t pid;

    pid = fork();

    if(pid == 0){
        struct sigaction cont_act;
        cont_act.sa_handler = sigcont_handler;
        sigemptyset(&cont_act.sa_mask);
        cont_act.sa_flags = 0;

        if(sigaction(SIGCONT,&cont_act,NULL) < 0){
            fprintf(stderr,"Unable to install SIGINT handler\n");
            logExit(4);
        }

        struct sigaction term_act;
        term_act.sa_handler = sigterm_handler;
        sigemptyset(&term_act.sa_mask);
        term_act.sa_flags = 0;

        if(sigaction(SIGTERM,&term_act,NULL) < 0){
            fprintf(stderr,"Unable to install SIGTERM handler\n");
            logExit(4);
        }

        if (setpgid(getpid(), getpid()) == -1) {
            perror("setpgid");
            logExit(-1);
        }
        //groupId = getpgrp();
        if(S_ISDIR(stat_buff.st_mode)){
            dr = opendir(path);
            if(dr){
                readDir(path, argc, argv);
                closedir(dr);
            }
        }
        else {
            int size = 0;
            size = readRegBlocks(path);
            printToConsole(size, path);
            logEntry(size, path);
        }
        //logExit(0);
    }
    else if(pid > 0){
        groupId = pid;
        waitpid(-1,NULL,0);
    }
    logExit(0);
}
