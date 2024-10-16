#include "task_scheduler.h"
#include "k_task.h"
#include <stdio.h>

TaskScheduler kernelTaskScheduler;
Heap kernelTaskSchedulerHeap;
TaskScheduler kernelSleepTaskScheduler;
Heap kernelSleepTaskSchedulerHeap;

//// Task Scheduler queue
//void initTaskScheduler(TaskScheduler* self) {
//	kernelTaskSchedulerQueue.head = 0;
//	kernelTaskSchedulerQueue.tail = 0;
//	kernelTaskSchedulerQueue.maxSize = MAX_TASKS; // in a circular queue, we sacrifice one extra space to differentiate between full and empty
//												  // 15 tasks can be scheduled at once.
//												  // Total tasks in Kernel = 15 scheduled + 1 running.
//	self->queue = &kernelTaskSchedulerQueue;
//
//	// initialize queue to NULL values
//	for(int i = 0; i < self->queue->maxSize; i++) {
//		self->array[i] = NULL;
//	}
//}
//
//int size(TaskScheduler* self){
//	int maxSize = self->queue->maxSize;
//	int head = self->queue->head;
//	int tail = self->queue->tail;
//
//	int size = (maxSize - head + tail) % maxSize;
//	return size;
//}
//
//bool isEmpty(TaskScheduler* self) {
//	return size(self) == 0;
//}
//
//bool isFull(TaskScheduler* self) {
//	return size(self) == MAX_TASKS;
//}
//
//TCB* front(TaskScheduler* self) {
//	if (isEmpty(self)) {
//		return NULL;
//	}
//	return self->array[self->queue->head];
//}
//
//TCB* dequeue(TaskScheduler* self) {
//	if (isEmpty(self)) {
//		return NULL;
//	}
//
//	int head = self->queue->head;
//	int maxSize = self->queue->maxSize;
//
//	TCB* poppedTCB = self->array[head];
//	self->array[head] = NULL;
//	self->queue->head = (head + 1) % maxSize;
//	return poppedTCB;
//}
//
//bool enqueue(TaskScheduler* self, TCB* tcb) {
//	int maxSize = self->queue->maxSize;
//
//	if (size(self) == maxSize - 1) { //TODO: Change to maxSize to pass test 7 of lab 1
//		return false;
//	}
//
//	int tail = self->queue->tail;
//
//	self->array[tail] = tcb;
//	self->queue->tail = (tail + 1) % maxSize;
//	return true;
//}

// MIN-HEAP SCHEDULER (Root node should be the task with the shortest remaining time, with ties being broken by lower tid)
void initTaskScheduler(TaskScheduler* self) {
	kernelTaskSchedulerHeap.currentSize = 0;
	kernelTaskSchedulerHeap.maxSize = MAX_TASKS;

	self->heap = &kernelTaskSchedulerHeap;

	// initialize heap to NULL values
	for(int i = 0; i < self->heap->maxSize; i++) {
		self->array[i] = NULL;
	}
}

void initSleepTaskScheduler(TaskScheduler* sleepScheduler) {
	kernelSleepTaskSchedulerHeap.currentSize = 0;
	kernelSleepTaskSchedulerHeap.maxSize = MAX_TASKS;

	sleepScheduler->heap = &kernelSleepTaskSchedulerHeap;

	// initialize heap to NULL values
	for(int i = 0; i < sleepScheduler->heap->maxSize; i++) {
		sleepScheduler->array[i] = NULL;
	}
}

bool isEmpty(TaskScheduler* self) {
	return self->heap->currentSize == 0;
}

bool isFull(TaskScheduler* self) {
	return self->heap->currentSize == MAX_TASKS;
}

void heapify(TaskScheduler* self, int currentIndex) {
	int leftChildIndex = left(currentIndex);
	int rightChildIndex = right(currentIndex);
	int smallestIndex = currentIndex;

	TCB* currentNode = self->array[currentIndex];
	TCB* leftChildNode = self->array[leftChildIndex];
	TCB* rightChildNode = self->array[rightChildIndex];

	// Check current node against left child node
	if (leftChildIndex < self->heap->currentSize) {
		if ((leftChildNode->remaining_time == currentNode->remaining_time) && (leftChildNode->tid < currentNode->tid)) { // case 1: same deadline priority, break tie with lower TID
			smallestIndex = leftChildIndex;
			currentNode = leftChildNode;
		}
		else if (leftChildNode->remaining_time < currentNode->remaining_time) { // case 2: lower deadline priority
			smallestIndex = leftChildIndex;
			currentNode = leftChildNode;
		}
	}

	// Check current node against right child node
	if (rightChildIndex < self->heap->currentSize) {
		if ((rightChildNode->remaining_time == currentNode->remaining_time) && (rightChildNode->tid < currentNode->tid)) { // case 1: same deadline priority, break tie with lower TID
			smallestIndex = rightChildIndex;
		}
		else if (rightChildNode->remaining_time < currentNode->remaining_time) { // case 2: lower deadline priority
			smallestIndex = rightChildIndex;
		}
	}

	// swap nodes to satisfy min heap priority
	if (smallestIndex != currentIndex) {
		swap(self, smallestIndex, currentIndex);
		heapify(self, smallestIndex);
	}
}

void swap(TaskScheduler* self, int index1, int index2) {
	TCB* temp = self->array[index1];
	self->array[index1] = self->array[index2];
	self->array[index2] = temp;
}

int left(int index) {
	return 2 * index + 1;
}

int right(int index) {
	return 2 * index + 2;
}

int parent(int index) {
	return (index - 1) / 2;
}

bool insert(TaskScheduler* self, TCB* tcb) {
	if (isFull(self)) {
		return false;
	}

	self->array[self->heap->currentSize] = tcb;
	self->heap->currentSize++;

	int index = self->heap->currentSize - 1; // get the index of the inserted node
	TCB* parentNode = self->array[parent(index)];
	TCB* currentNode = self->array[index];

	while (index != 0 && (currentNode->remaining_time <= parentNode->remaining_time)) { //heapify upwards starting with the added node
		if (parentNode->remaining_time == currentNode->remaining_time) { // Check 1: same remaining_time
			if (currentNode->tid < parentNode->tid) { 					 // Check 2: break ties with TID
				swap(self, parent(index), index);
				index = parent(index);
			} else {
				break;
			}
		} else {														 // Check 1: currentNode's remaining_time < parentNode's remaining_time
			swap(self, parent(index), index);
			index = parent(index);
		}
		currentNode = self->array[index];
		parentNode = self->array[parent(index)];
	}

	return true;
}

TCB* removeMin(TaskScheduler* self) {
	if (isEmpty(self)) {
		return NULL;
	}

	TCB* toRemove = self->array[0];
	self->array[0] = self->array[self->heap->currentSize - 1];
	self->array[self->heap->currentSize - 1] = NULL;
	self->heap->currentSize--;

	heapify(self, 0);

	return toRemove;
}

TCB* getMinRemainingTimeTask(TaskScheduler* self) {
	if (isEmpty(self)) {
		return NULL;
	}

	if (self->array[0]->tid == 0 && self->heap->currentSize > 1) {
		self->array[0]->remaining_time = self->array[0]->deadline;
	}
	reheapify(self);

	return self->array[0]; //get root of MinHeap
}

void reheapify(TaskScheduler* self) {
	for (int i = (self->heap->currentSize) / 2 - 1; i >= 0; i--) {
		heapify(self, i);
	}
}

void printHeap(TaskScheduler* self) {
	printf("\n\r~~~BEGIN PRINT ITERATION~~~\n\r");
	for (int i = 0; i < self->heap->currentSize; i++) {
		printf("%d ", self->array[i]->remaining_time); // Alter to also include tid incase of ties.
		if (i == 0 || i == 2 || i == 6 || i == 14) {
			printf("\n\r");
		}
	}
	printf("~~~END PRINT ITERATION~~~\n\r");
}

bool changeRemainingTime(TaskScheduler* self, int tid, int newDeadline) {
	if (isEmpty(self)) return false;
//	if (tid == 0) return false;

	for(int i = 0; i < self->heap->currentSize; i++) {
		if (self->array[i]->tid == tid) {
			self->array[i]->deadline = newDeadline;
			self->array[i]->remaining_time = newDeadline;
			reheapify(self);
			return true;
		}
	}

	return false;
}

void decrementRemainingTime(TaskScheduler* self) {
	for (int i = 0; i < self->heap->currentSize; i++) {
		self->array[i]->remaining_time -= 1;
	}

	return;
}

void min_heap_UnitTest() {
	initTaskScheduler(&kernelTaskScheduler);
	assert(isEmpty(&kernelTaskScheduler));
	assert(!isFull(&kernelTaskScheduler));

	TCB tcbs_test[MAX_TASKS];
	for(int i = 0; i < MAX_TASKS; i++) {
		tcbs_test[i].tid = i;
		tcbs_test[i].deadline = i;
		tcbs_test[i].remaining_time = i;
	}

	// Test insertion
	insert(&kernelTaskScheduler, &tcbs_test[3]);
	insert(&kernelTaskScheduler, &tcbs_test[4]);
	insert(&kernelTaskScheduler, &tcbs_test[5]);
	insert(&kernelTaskScheduler, &tcbs_test[1]);
	insert(&kernelTaskScheduler, &tcbs_test[2]);
	insert(&kernelTaskScheduler, &tcbs_test[0]);
	insert(&kernelTaskScheduler, &tcbs_test[7]);
	insert(&kernelTaskScheduler, &tcbs_test[8]);
	insert(&kernelTaskScheduler, &tcbs_test[9]);
	insert(&kernelTaskScheduler, &tcbs_test[10]);
	insert(&kernelTaskScheduler, &tcbs_test[11]);
	insert(&kernelTaskScheduler, &tcbs_test[12]);
	insert(&kernelTaskScheduler, &tcbs_test[13]);
	insert(&kernelTaskScheduler, &tcbs_test[14]);
	insert(&kernelTaskScheduler, &tcbs_test[15]);
	insert(&kernelTaskScheduler, &tcbs_test[6]);

	assert(insert(&kernelTaskScheduler, &tcbs_test[0]) == false); // should reject insertion as heap is full w/ 16 tasks

	// Test remove min
	removeMin(&kernelTaskScheduler); // remove [TID=0,remaining_time=0] at the top, replace with 8 and then heapify downwards
	removeMin(&kernelTaskScheduler); //remove [TID=1,remaining_time=1] at the top, replace with 15 then heapify downwards
	printHeap(&kernelTaskScheduler);

	insert(&kernelTaskScheduler, &tcbs_test[0]); // insert [TID=0,remaining_time=0] at bottom to test functionality of insert after removal
	printHeap(&kernelTaskScheduler);

	// Test change remaining time
	changeRemainingTime(&kernelTaskScheduler, 10, -1); // change [TID=10,remaining_time=10] -> [TID=10,remaining_time=-1]
													   // bubble up interior node case
	printHeap(&kernelTaskScheduler);

	changeRemainingTime(&kernelTaskScheduler, 15, -2); // change [TID=15,remaining_time=15] -> [TID=15,remaining_time=-2]
													   // bubble up leaf node to root case
	printHeap(&kernelTaskScheduler);

	changeRemainingTime(&kernelTaskScheduler, 15, 100); // change [TID=15,remaining_time=-2] -> [TID=15,remaining_time=100]
														// bubble down root node case
	printHeap(&kernelTaskScheduler);

	changeRemainingTime(&kernelTaskScheduler, 2, 50); // change [TID=2,remaining_time=2] -> [TID=2,remaining_time=50]
														// bubble down interior node case
	printHeap(&kernelTaskScheduler);
}

