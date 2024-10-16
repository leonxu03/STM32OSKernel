#ifndef TASK_SCHEDULER_H
#define TASK_SCHEDULER_H

#include "k_task.h"

// Task Scheduler data structure
typedef struct TaskScheduler {
//	Queue* queue;
	TCB* array[MAX_TASKS];
	Heap* heap;
} TaskScheduler;

extern TaskScheduler kernelTaskScheduler;
//extern Queue kernelTaskSchedulerQueue;
extern Heap kernelTaskSchedulerHeap;
extern TaskScheduler kernelSleepTaskScheduler;
extern Heap kernelSleepTaskSchedulerHeap;

// Task Scheduler queue
void initTaskScheduler(TaskScheduler* self);

int size(TaskScheduler* self);

bool isEmpty(TaskScheduler* self);

bool isFull(TaskScheduler* self);

TCB* front(TaskScheduler* self);

TCB* dequeue(TaskScheduler* self);

bool enqueue(TaskScheduler* self, TCB* tcb);

#endif
