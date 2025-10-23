#ifndef QUEUE_H_
#define QUEUE_H_

#include <stdlib.h>
#include <stdint.h>

typedef struct queue 
{
	unsigned char *pBase;
	int front;
	int rear;
	int maxsize;
	uint8_t lock;
}QUEUE;

void createQueue(QUEUE* Q,int maxsize, unsigned char *pBuf);
unsigned char isQueueFull(QUEUE* Q);
unsigned char isQueueEmpty(QUEUE* Q);
unsigned char enqueue(QUEUE* Q, unsigned char val);
unsigned char dequeue(QUEUE* Q, unsigned char *val);
void flushQueue(QUEUE* Q);
void deleteQueue(QUEUE* Q);
#endif

