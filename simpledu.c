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

void checkFlags(int size, char* argv[], struct flags *f){
    for (int i = 0; i < size ; ++i) {
        if(strcmp(argv[i],possibleFlags[0]) == 0 || strcmp(argv[i],possibleFlags[1]) == 0)
            f->all = true;
        else if(strcmp(argv[i],possibleFlags[2]) == 0 || strcmp(argv[i],possibleFlags[3]) == 0)
            f->bytes = true;
        else if(strcmp(argv[i],possibleFlags[4]) == 0 || strcmp(argv[i],possibleFlags[5]) == 0)
            f->blockSize = true;
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
    }
}*/




int main(int argc, char* argv[], char* envp[]){
    int src;
    struct stat stat_buff; //file;
    char path[PATH_MAX];
    struct flags f = {false,false,false,false,false,false,false};
    DIR *dr;
    struct dirent *dir;
    //int size;

    if(argc > 8 ){
        fprintf(stderr, "Usage: %s -l [path] [-a] [-b] [-B size] [-L] [-S] [--max-depth=N]\n", argv[0]);
        exit(1);
    }

    strcpy(path,argv[2]);

    if((src = open("output.txt",O_WRONLY | O_CREAT | O_TRUNC, 0600)) == -1){
        printf("Error Number % d\n", errno);
        perror("Error: ");
        exit(2);
    }

    checkFlags(argc,argv,&f);

    if(stat(argv[2],&stat_buff) == -1){
        perror("lstat ERROR");
        exit(3);
    }
    if(S_ISDIR(stat_buff.st_mode)){
        dr = opendir(path);
        if(dr){
            if(f.all){
                dup2(src,STDOUT_FILENO);
                while((dir = readdir(dr)) != NULL){
                    if(strcmp(dir->d_name,".") == 0|| strcmp(dir->d_name,"..") == 0)
                        continue;
                    //stat(dir->d_name,&file);
                    //size = file.st_size;
                    printf("%s/%s\n",path,dir->d_name);
                }
                //size = stat_buff.st_size;
                printf("%s",path);
                closedir(dr);
            }
            if(f.bytes){
                printf("Total size in bytes: %ld\n",stat_buff.st_size);
            }
        }
    }

    exit(0);

}
