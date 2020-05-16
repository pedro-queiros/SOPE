#include "utils.h"

void checkServerArgs(int argc, char* argv[],int *nSecs, char* fifoName, int *nPlaces, int *nThreads){
    if(argc < 4 || argc > 8){
        printf("Usage: Qn <-t nsecs> [-l nplaces] [-n nthreads] fifoname\n");
        exit(1);
    }
    for (int i = 1; i < argc; i++){
        char * aux = argv[i];
        if(aux[0] == '-'){
            if(strcmp(aux,"-t") == 0){
                *nSecs = atoi(argv[i+1]);
                i++;
            }
            else if(strcmp(aux,"-l") == 0){
                *nPlaces = atoi(argv[i+1]);
                i++;
            }
            else if(strcmp(aux,"-n") == 0){
                *nThreads = atoi(argv[i+1]);
                i++;
            }
            else{
                printf("Usage: Qn <-t nsecs> [-l nplaces] [-n nthreads] fifoname\n");
                exit(2);
            }
        }
        else{
            strcpy(fifoName,aux);
        }
    }
}

void checkClientArgs(int argc, char* argv[],int * workingTime, char * fifoName){
    if(argc != 4){
        fprintf(stderr, "Usage: Un <-t nsecs> fifoname\n");
        exit(1);
    }
    for (int i = 1; i < argc ; i++) {
        char * aux = argv[i];
        if(aux[0] == '-'){
            if(strcmp(aux,"-t") == 0){
                *workingTime = atoi(argv[i+1]);
                i++;
            }
            else{
                fprintf(stderr, "Usage: Un <-t nsecs> fifoname\n");
                exit(2);
            }
        }
        else{
            strcpy(fifoName,aux);
        }
    }
}