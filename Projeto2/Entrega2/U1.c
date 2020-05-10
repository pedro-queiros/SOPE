#include <pthread.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdbool.h>
#include "timer.h"
#include "logs.h"

int id = 1;
int opened = true;

void *thread_handler(void *fifo){
    char msg[MAX_LEN] = {0};
    //srand(time(NULL));
    long int dur = rand() % 200 + 1;
    sprintf(msg,"[ %d, %d, %ld, %ld, -1]",id,(int)getpid(),(long)pthread_self(),dur);

    char fifo_priv[MAX_LEN] = "/tmp/", pidInString[MAX_LEN] = {0}, tidInString[MAX_LEN] = {0};
    sprintf(pidInString, "%d", getpid());
    strcat(fifo_priv, pidInString);
    strcat(fifo_priv, ".");
    sprintf(tidInString, "%ld", pthread_self());
    strcat(fifo_priv, tidInString);

    if(mkfifo(fifo_priv, 0660) < 0){
        printToConsole(id,getpid(),pthread_self(),dur,-1,"FAILD");
        perror("Error Creating Private Fifo");
        return NULL;
    }

    char *fifo_name = (char *)fifo;
    int fd;
    if((fd = open(fifo_name, O_WRONLY)) < 0){
        printToConsole(id,getpid(),pthread_self(),dur,-1,"FAILD");
        opened = false;
        return NULL;
    }
    printToConsole(id,getpid(),pthread_self(),dur,-1,"IWANT");

    if(write(fd, &msg, MAX_LEN) < 0){
        printToConsole(id,getpid(),pthread_self(),dur,-1,"FAILD");
        perror("Error Writing to Public Fifo\n");
        return NULL;
    }

    if(close(fd) < 0){
        perror("Error Closing Public Fifo\n");
        return NULL;
    }

    int fd2;

    if ((fd2 = open(fifo_priv,O_RDONLY)) < 0){
        perror("Error Opening File");
        return NULL;
    }

    char answer[MAX_LEN];
    if(read(fd2, &answer, MAX_LEN) < 0){
        perror("Error Reading Private Fifo\n");
        return NULL;
    }

    int num1, pid, place;
    long tid;
    sscanf(answer,"[%d, %d, %ld, %ld, %d]",&num1,&pid,&tid,&dur, &place);
    if(place==-1 && dur==-1)
        printToConsole(id,getpid(),pthread_self(),dur,-1,"CLOSD");
    else
        printToConsole(num1, getpid(),pthread_self(), dur, place, "IAMIN");

    if(close(fd2) < 0){
        perror("Error Closing Private Fifo\n");
        return NULL;
    }

    if(unlink(fifo_priv) < 0){
        perror("Error Deleting Fifo");
    }

    return NULL;
}

void checkArgs(int argc, char* argv[],int *nSecs, char* fifoName, int *nPlaces, int *nThreads){
    if(argc > 4 || argc > 8){
        printf("Usage: Qn <-t nsecs> [-l nplaces] [-n nthreads] fifoname");
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
                printf("Usage: Qn <-t nsecs> [-l nplaces] [-n nthreads] fifoname");
                exit(1);
            }
        }
        else{
            strcpy(fifoName,aux);
        }
    }
}

int main(int argc, char* argv[], char* envp[]){
    /*if (argc != 4){
        fprintf(stderr, "Usage: Un <-t nsecs> fifoname\n");
    }*/

    startTime();

    int workingTime;
    int nPlaces, nThreads;
    //sscanf(argv[2], "%lf", &workingTime);

    pthread_t thread;

    char fifo[MAX_LEN] = {0};
    //strcat(fifo, argv[3]);
    checkArgs(argc,argv,&workingTime,fifo,&nPlaces,&nThreads);

    while(getElapsedTime() <= workingTime && opened){
        if(pthread_create(&thread,NULL,thread_handler,&fifo) != 0){
            perror("Error Creating Thread\n");
            break;
        }
        if(pthread_detach(thread) != 0){
            perror("Error Detaching Thread\n");
            break;
        }
        usleep(20*1000);
        id++;
    }

    exit(0);
}
