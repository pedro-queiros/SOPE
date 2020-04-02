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

#include "args.h"

#define MAX_COMMANDS 10

char *possibleFlags[13] = {"-a","--all","-b","--bytes","-B","--block-size","-l","--count-links","-L","--dereference","-S","--separate-dirs","--max-depth"};

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
        else if(strcmp(argv[i],possibleFlags[12]) == 0)
            f->depth = true;
    }
}

/*void writeToFile(int src,struct flags f){
    dup2(src,STDOUT_FILENO);
    if(f.all){
        printf();
}*/

void printToConsole(int size, char* filepath){
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
    int size;
    if (stat(filepath, &file) < 0) {
        printf("Error reading file stat.\n");
        exit(1);
    }
    size = file.st_blocks / 2;
    if (f.all)
        printToConsole(size, filepath);
    return size;
}

int readRegBytes (char* filepath){
    struct stat file;
    int size;
    if (stat(filepath, &file) < 0) {
        printf("Error reading file stat.\n");
        exit(1);
    }
    size = file.st_size;
    return size;
}

int readDir (char* path){
    DIR* dr;
    struct dirent *dir;
    char filepath[256];
    int size = 0;
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
        if (f.all)
            size += readRegBlocks(filepath);
        if (f.bytes)
            size += readRegBytes(filepath);
        if (f.blockSize) {
            size += readRegBlocks(filepath);
        }
    }
    return size;
}

int main(int argc, char* argv[], char* envp[]){
    //int src;
    struct stat stat_buff;
    DIR *dr;
    char path[256];
    int size;
    if(argc > 8){
        fprintf(stderr, "Usage: %s -l [path] [-a] [-b] [-B size] [-L] [-S] [--max-depth=N]\n", argv[0]);
        exit(1);
    }
    strcpy(path,argv[2]);
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
            //dup2(src,STDOUT_FILENO);
            if(f.all){
                size = readDir(path);
                size += stat_buff.st_blocks / 2;
                printToConsole(size, path);
            }
            if(f.bytes){
                size = readDir(path);
                size += stat_buff.st_size;
                printToConsole(size, path);
            }
            if (f.blockSize){
                size = readDir(path);
                size += stat_buff.st_blocks / 2;
                double result = size * (1024.0 / args.block_size);
                int intpart = (int) result;
                if (result - intpart > 0)
                    intpart = result + 1;
                printToConsole(intpart, path);
            }
        }
        closedir(dr);
    }
    exit(0);
}
