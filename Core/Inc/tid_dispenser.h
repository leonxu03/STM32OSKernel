#ifndef TID_DISPENSER_H
#define TID_DISPENSER_H

#include "k_task.h"

// Stores available TIDs for task creation
typedef struct TIDdispenser {
	Queue* queue;
	int array[MAX_TASKS + 1]; // 17 total slots, 1 extra to differentiate between empty and full queue. 16 TIDs can be stored (0-15)
} TIDdispenser;

TIDdispenser kernelTIDdispenser;
Queue kernelTIDdispenserQueue;

int numOfAvailableTIDs(TIDdispenser* self){
	int maxSize = self->queue->maxSize;
	int head = self->queue->head;
	int tail = self->queue->tail;

	int size = (maxSize - head + tail) % maxSize;
	return size;
}

bool outOfTIDs(TIDdispenser* self) {
	return numOfAvailableTIDs(self) == 0;
}

int dequeueTID(TIDdispenser* self) {
	if (outOfTIDs(self)) {
		return NULL;
	}

	int head = self->queue->head;
	int maxSize = self->queue->maxSize;

	int TID = self->array[head];
	self->array[head] = -1;
	self->queue->head = (head + 1) % maxSize;

	return TID;
}

bool enqueueTID(TIDdispenser* self, int tid) {
	int maxSize = self->queue->maxSize;

	if (numOfAvailableTIDs(self) == maxSize - 1) {
		return false;
	}

	int tail = self->queue->tail;

	self->array[tail] = tid;
	self->queue->tail = (tail + 1) % maxSize;
	return true;
}

// Initialize TID dispenser data structure
void initTIDdispenser(TIDdispenser* self) {
	kernelTIDdispenserQueue.head = 0;
	kernelTIDdispenserQueue.tail = 0;
	kernelTIDdispenserQueue.maxSize = MAX_TASKS + 1; // in a circular queue, we sacrifice one extra space to differentiate between full and empty
												  // 16 TIDs total
	self->queue = &kernelTIDdispenserQueue;

	// initialize queue to NULL values
	for(int i = 0; i < MAX_TASKS; i++) {
		enqueueTID(&kernelTIDdispenser, i);
	}
}

#endif
