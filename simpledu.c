#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <dirent.h>
#include <fcntl.h>
#include <string.h>
#include <stdbool.h>
#include <dirent.h>
#include <limits.h>
#include <sys/wait.h>
#include <sys/types.h>
#include "args.h"

#define MAX_COMMANDS 10
#define READ 0
#define WRITE 1

char *possibleFlags[13] = {"-a","--all","-b","--bytes","-B","--block-size","-l","--count-links","-L","--dereference","-S","--separate-dirs","--max-depth"};

char initialDir[256];

struct flags{
    bool all;
    bool bytes;
    bool blockSize;
    bool countLinks;
    bool dereference;
    bool separate;
    bool depth;
};

struct flags f = {false,false,0,false,false,false,0};

struct Args args = {0,0,1024,0,0,0,-1};

void getNumber(char* cmd){
    char* blockSize = "--block-size=";
    char* max_d = "--max-depth=";
    if (strncmp(cmd, blockSize,13) == 0){
        cmd = strtok(cmd, "=");
        args.block_size = atoi(strtok(NULL, "="));
    }

    else if (strncmp(cmd, max_d, 12) == 0){
        cmd = strtok(cmd, "=");
        args.max_depth = atoi(strtok(NULL, "="));
    }
}

void checkFlags(int size, char* argv[], struct flags *f){
    for (int i = 0; i < size ; ++i) {
        if(strcmp(argv[i],possibleFlags[0]) == 0 || strcmp(argv[i],possibleFlags[1]) == 0) {
            f->all = true;
        }
        else if(strcmp(argv[i],possibleFlags[2]) == 0 || strcmp(argv[i],possibleFlags[3]) == 0)
            f->bytes = true;
        else if(strcmp(argv[i],possibleFlags[4]) == 0 || strcmp(argv[i],possibleFlags[5]) == 0) {
            f->blockSize = true;
            args.block_size = atoi(argv[i+1]);
        }
        else if(strcmp(argv[i],possibleFlags[6]) == 0 || strcmp(argv[i],possibleFlags[7]) == 0)
            f->countLinks = true;
        else if(strcmp(argv[i],possibleFlags[8]) == 0 || strcmp(argv[i],possibleFlags[9]) == 0)
            f->dereference = true;
        else if(strcmp(argv[i],possibleFlags[10]) == 0 || strcmp(argv[i],possibleFlags[11]) == 0)
            f->separate = true;
        else getNumber(argv[i]);
    }
}

/*void writeToFile(int src,struct flags f){
    dup2(src,STDOUT_FILENO);
    if(f.all){
        printf();
}*/

int numTimesAppears(char* string, char ch)
{
    int i;
    int count = 0;
    for(i = 0; string[i] != '\0' ; ++i)
    {
        if (string[i] == ch)
        {
            count++;
        }
    }
    return count;
}

int getSubString(char *source, char *target,int from, int to)
{
    int length=0;
    int i=0,j=0;

    //get length
    while(source[i++]!='\0')
        length++;

    if(from<0 || from>length){
        printf("Invalid \'from\' index\n");
        return 1;
    }
    if(to>length){
        printf("Invalid \'to\' index\n");
        return 1;
    }

    for(i=from,j=0;i<=to;i++,j++){
        target[j]=source[i];
    }

    //assign NULL at the end of string
    target[j]='\0';

    return 0;
}

void printToConsole(int size, char* filepath){
    if(f.depth){
        char path[256];
        getSubString(filepath, path, strlen(initialDir) + 1, strlen(filepath));
        if (numTimesAppears(path, '/') >= args.max_depth) return;
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

    if(f.separate){
        if(S_ISDIR(file.st_mode)){
            return 0;
        }
    }

    //size = file.st_blocks / 2;
    if (f.bytes)
        size = file.st_size;
    else
        size = file.st_blocks/2;

    if (f.blockSize) {
        //size = file.st_blocks / 2;
        double result = size * (1024.0 / args.block_size);
        size = (int) result;
        if (result - size > 0)
            size = result + 1;
    }

    if (f.all && !S_ISDIR(file.st_mode))
        printToConsole(size, filepath);

    return size;
}


/*int readRegBytes (char* filepath){
    struct stat file;
    int size;

    if (stat(filepath, &file) < 0) {
        printf("Error reading file stat.\n");
        exit(1);
    }

    size = file.st_size;
    return size;
}*/


int readDir (char* path){
    DIR* dr;
    struct dirent *dir;
    char filepath[256];
    struct stat check;
    int size = 0, status, aux = 0;
    int fd[1];
    pid_t pid;

    if ((dr = opendir(path)) == NULL){
        perror(path);
        exit(1);
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
            if (pid == 0){            //processo-filho
                int childSize = 0;
                childSize += readDir(filepath);
                //childSize += check.st_blocks/2;
                /*if (f.bytes)
                    childSize += check.st_size;
                else
                    childSize += check.st_blocks/2;*/
                close(fd[READ]);
                write(fd[WRITE],&childSize,sizeof(int));
                close(fd[WRITE]);
                exit(0);
            }
            else if (pid > 0){        //processo-pai
                waitpid(-1,&status,0);
                close(fd[WRITE]);
                read(fd[READ],&aux,sizeof(int));
                close(fd[READ]);
                size += aux;
                //printToConsole(aux, filepath);
            }
        }
        else{
            //waitpid(-1,&status,0);
            size += readRegBlocks(filepath);
            /*if (f.all || f.dereference)
                size += readRegBlocks(filepath);
            if (f.bytes)
                size += readRegBytes(filepath);
            if (f.blockSize)
                size += readRegBlocks(filepath);
            if(!f.all && !f.bytes && !f.dereference && !f.separate && !f.blockSize && !f.depth)
                size += readRegBlocks(filepath);*/
        }
    }
    size += readRegBlocks(path);
    /*stat(path,&currentDir);
    if (f.bytes)
        size += currentDir.st_size;
    //else
        //size += currentDir.st_blocks/2;
    if (f.blockSize) {
        //size = file.st_blocks / 2;
        //printf("%d\n",size);
        double result = currentDir.st_blocks/2 * (1024.0 / args.block_size);
        int interpart = (int) result;
        if (result - interpart > 0)
            interpart = result + 1;
        size += interpart;
        //printTotal(interpart,path);
    }*/
    printTotal(size,path);
    return size;
}

int main(int argc, char* argv[], char* envp[]){
    //int src;
    struct stat stat_buff;
    DIR *dr;
    char path[256];
    //int size;
    if(argc > 8){
        fprintf(stderr, "Usage: %s -l [path] [-a] [-b] [-B size] [-L] [-S] [--max-depth=N]\n", argv[0]);
        exit(1);
    }

    strcpy(path,argv[2]);
    strcpy(initialDir, argv[2]);

    /*if((src = open("output.txt",O_WRONLY | O_CREAT | O_TRUNC, 0600)) == -1){
        printf("Error Number % d\n", errno);
        perror("Error: ");
        exit(2);
    }*/

    checkFlags(argc,argv,&f);
    if(stat(argv[2],&stat_buff) == -1){
        perror("lstat ERROR");
        exit(3);
    }

    if(S_ISDIR(stat_buff.st_mode)){
        dr = opendir(path);
        if(dr){
            readDir(path);
            //size += stat_buff.st_blocks/2;

            //dup2(src,STDOUT_FILENO);
            /*if(f.all || f.dereference || f.separate){
                size = readDir(path);
                size += stat_buff.st_blocks / 2;
                printTotal(size, path);
            }
            else if(f.bytes){
                size = readDir(path);
                size += stat_buff.st_size;
                //printToConsole(size, path);
                printTotal(size, path);
            }
            else if (f.blockSize){
                size = readDir(path);
                size += stat_buff.st_blocks / 2;
                double result = size * (1024.0 / args.block_size);
                int intpart = (int) result;
                if (result - intpart > 0)
                    intpart = result + 1;
                //printToConsole(intpart, path);
                printTotal(size, path);
            }
            else{                                //maybe need to be an if
                size = readDir(path);
                size += stat_buff.st_blocks / 2;
                //printToConsole(size, path);
                printTotal(size, path);
            }
        }*/
        closedir(dr);
    }
    }
    exit(0);
}

