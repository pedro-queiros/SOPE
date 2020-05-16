#include <pthread.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdbool.h>
#include <errno.h>
#include "timer.h"
#include "logs.h"

int id = 1;
int fd;
int opened = true;
char fifo_name[MAX_LEN] = {0};
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void *thread_handler(void *arg){
    if(pthread_detach(pthread_self()) != 0){
        perror("Error Detaching Thread\n");
        //pthread_exit(NULL);
        return NULL;
    }

    //int fd;

    char fifo_priv[MAX_LEN] = {0};
    sprintf(fifo_priv, "/tmp/%d.%ld", getpid(), pthread_self());

    if(mkfifo(fifo_priv, 0660) < 0){
        printToConsole(id,getpid(),pthread_self(),-1,-1,"FAILD");
        perror("Error Creating Private Fifo");
        return NULL;
        //pthread_exit(NULL);
    }
    char msg[MAX_LEN] = {0};
    long int dur = rand() % 500 + 1;

    pthread_mutex_lock(&mutex);
    int clientId = id;
    id++;
    pthread_mutex_unlock(&mutex);

    sprintf(msg,"[ %d, %d, %ld, %ld, -1]",clientId,(int)getpid(),(long)pthread_self(),dur);

    if(write(fd, &msg, MAX_LEN) < 0){
        printToConsole(id,getpid(),pthread_self(),dur,-1,"FAILD");
        perror("Error Writing to Public Fifo\n");
        if(close(fd)<0) fprintf(stderr, "Cannot close public FIFO");
        if (unlink(fifo_priv) < 0) fprintf(stderr, "Cannot delete private FIFO");
        opened = false;
        return NULL;
        //pthread_exit(NULL);
    }
    printToConsole(clientId,getpid(),pthread_self(),dur,-1,"IWANT");


    int fd2;

    if ((fd2 = open(fifo_priv,O_RDONLY)) < 0){
        printToConsole(id,getpid(),pthread_self(),dur,-1,"FAILD");
        //unlink(fifo_priv);
        perror("Error Opening File");
        return NULL;
        //pthread_exit(NULL);
    }

    char answer[MAX_LEN];

    /*int tries = 0;
    while(read(fd2,answer,MAX_LEN) <= 0 && tries < 5){
        fprintf(stderr, "Cant read. Try again");
        usleep(200);
        tries++;
    }
    if (tries > 0 && tries < 5) {
        fprintf(stderr,"READ!");
    }

    if(tries == 5){
        fprintf(stderr, "Can't read from private FIFO\n");
        //writeRegister(mynum,getpid(),pthread_self(),duration,-1,FAILED);
        if (close(fd2) < 0)
            fprintf(stderr, "Error closing FIFO %s file descriptor.\n", fifo_priv);
        if (unlink(fifo_priv)<0)
            fprintf(stderr, "Error when destroying FIFO '%s'\n",fifo_priv);
        return NULL;
        //pthread_exit(NULL);
    }*/

    if(read(fd2, &answer, MAX_LEN) <= 0){
        printToConsole(id,getpid(),pthread_self(),dur,-1,"FAILD");
        perror("Error Reading Private Fifo\n");
        pthread_exit(NULL);
    }

    int num1, pid, place;
    long tid;
    sscanf(answer,"[%d, %d, %ld, %ld, %d]",&num1,&pid,&tid,&dur, &place);
    if(place==-1 && dur==-1){
        printToConsole(num1,getpid(),pthread_self(),dur,-1,"CLOSD");
        opened = false;
    }
    else
        printToConsole(num1, getpid(),pthread_self(), dur, place, "IAMIN");

    if(close(fd2) < 0){
        perror("Error Closing Private Fifo\n");
        return NULL;
        //pthread_exit(NULL);
    }

    if(unlink(fifo_priv) < 0){
        perror("Error Deleting Fifo");
    }
    return NULL;
    //pthread_exit(NULL);
}

void checkArgs(int argc, char* argv[],int * workingTime, char * fifoName){
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
                exit(1);
            }
        }
        else{
            strcpy(fifoName,aux);
        }
    }
}


int main(int argc, char* argv[], char* envp[]){
    if (argc != 4){
        fprintf(stderr, "Usage: Un <-t nsecs> fifoname\n");
    }

    startTime();

    int workingTime;
    checkArgs(argc,argv,&workingTime,fifo_name);
    //sscanf(argv[2], "%d", &workingTime);

    //strcat(fifo_name, argv[3]);

    if((fd = open(fifo_name, O_WRONLY/*|O_NONBLOCK,0660*/)) < 0){
        printToConsole(id,getpid(),pthread_self(),-1,-1,"FAILD");
        opened = false;
        fprintf(stderr, "Oops !!! Service is closed !!!\n");
        //pthread_exit(NULL);
        //exit()
    }

    while(getElapsedTime() < workingTime && opened){
        pthread_t thread;
        if(pthread_create(&thread,NULL,thread_handler,NULL) != 0){
            perror("Error Creating Thread\n");
        }
        usleep(5*1000);
    }

    if(close(fd) < 0){
        perror("Error Closing Public Fifo\n");
        //pthread_exit(1);
    }

    pthread_exit(0);
}
