#include <stdlib.h>

typedef struct queue{
    int front, rear, placesAvailable, maxCapacity;
    int *array;
} queue;

queue createQueue(int maxCapacity){
    queue q;
    q.maxCapacity = maxCapacity;
    q.front = 0;
    q.rear = maxCapacity-1;
    q.placesAvailable = 0;
    q.array = (int*) malloc(maxCapacity*sizeof(int));
    return q;
};

int isFull(queue *q){
    return q->placesAvailable == q->maxCapacity;
}

int isEmpty(queue *q){
    return q->placesAvailable == 0;
}

int occupyPlace(queue *q){
    if(isEmpty(q))
        return 0;
    int placeId = q->array[q->front];
    q->front = (q->front+1) % q->maxCapacity;
    q->placesAvailable--;
    return placeId;
}

void releasePlace(queue *q,int placeId){
    if(isFull(q))
        return;
    q->rear = (q->rear+1)%q->maxCapacity;
    q->array[q->rear] = placeId;
    q->placesAvailable++;
}

void createPlaces(queue *q){
    for (int i = 1; i < q->maxCapacity+1; i++) {
        releasePlace(q,i);
    }
}

