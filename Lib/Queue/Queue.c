#include "Queue.h"

void createQueue(QUEUE* Q, int maxsize, unsigned char *pBuf) {
    Q->pBase = pBuf;
    Q->front = 0;
    Q->rear = 0;
    Q->maxsize = maxsize;
}

unsigned char isQueueFull(QUEUE* Q) {
    return (Q->front == (Q->rear + 1) % Q->maxsize);
}

unsigned char isQueueEmpty(QUEUE* Q) {
    return (Q->front == Q->rear);
}

unsigned char enqueue(QUEUE* Q, unsigned char val) {
    if (isQueueFull(Q)) {
        return 0;
    } else {
        Q->pBase[Q->rear] = val;
        Q->rear = (Q->rear + 1) % Q->maxsize;
        return 1;
    }
}

unsigned char dequeue(QUEUE* Q, unsigned char *val) {
    if (isQueueEmpty(Q)) {
        return 0;
    } else {
        *val = Q->pBase[Q->front];
        Q->front = (Q->front + 1) % Q->maxsize;
        return 1;
    }
}

void flushQueue(QUEUE* Q) {
    Q->front = 0;
    Q->rear = 0;
    return;
}

void deleteQueue(QUEUE* Q) {
    if (Q->pBase != 0) {
        free(Q->pBase);
    }

    Q->pBase = 0;
    Q->front = 0;
    Q->rear = 0;
    Q->maxsize = 0;
}