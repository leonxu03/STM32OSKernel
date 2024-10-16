#include "tcb_array.h"
#include "k_task.h"

TCBarray kernelTCBarray;
TCB tcbs[30];

void initTCBarray() {
//	kernelTCBarray.p_stackLimit = (uint32_t)*(uint32_t**)0x0 - (uint32_t)0x4000;	// Set stackptr to lower stack bound
	kernelTCBarray.p_currentStackSize = *(uint32_t**)0x0 - (uint32_t)MAIN_STACK_SIZE; // pointer to the bottom of our stack so far

	kernelTCBarray.index = 0; // Reserve index 0 for the null task.
	kernelTCBarray.numOfActiveTasks = 0;

	for(int i = 0; i < 30; i++) {
		kernelTCBarray.array[i] = NULL;
	}
}

//uint32_t* currentStackSize(TCBarray* self) {
//	self->p_currentStackSize;
//}
//
//uint32_t* stackLimit(TCBarray* self) {
//	self->p_stackLimit;
//}

bool isArrayFull(TCBarray* self) {
	return self->index == 30;
}

bool hasReachedMaxActiveTasks(TCBarray* self) {
	return self->numOfActiveTasks == MAX_TASKS;
}

int numOfActiveTasks(TCBarray* self) {
	return self->numOfActiveTasks;
}

bool canAppendToStack(TCBarray* self, TCB* incomingTask) {
	if (isArrayFull(self)) {
		return false;
	}

	if (hasReachedMaxActiveTasks(self)) {
		return false;
	}

//	uint32_t* p_currentStackSize = self->p_currentStackSize;
//	uint32_t currentStackSize = (uint32_t)p_currentStackSize;
//
//	uint32_t* p_stackLimit = self->p_stackLimit;
//	uint32_t stackLimit = (uint32_t)p_stackLimit;
//
//	uint32_t incomingTaskSize = (uint16_t)(incomingTask->stack_size);

	return true;
}

TCB* appendToStack(TCBarray* self, TCB *incomingTask) {
	int currentIndex = self->index;

	TCB* newTCB = &tcbs[currentIndex];

	newTCB->ptask = incomingTask->ptask;
	newTCB->stack_high = incomingTask->stack_high;
	newTCB->tid = incomingTask->tid;
	newTCB->state = READY;
	newTCB->stack_size = incomingTask->stack_size;
	newTCB->alloc_point = incomingTask->alloc_point;
	newTCB->deadline = incomingTask->deadline;
	newTCB->remaining_time = incomingTask->remaining_time;

	// Push registers onto the stack - dummy values for now
	uint32_t* p_stackptr = newTCB->stack_high;
	*(--p_stackptr) = 1<<24; // this is xPSR, setting the chip to Thumb mode
	*(--p_stackptr) = (uint32_t)newTCB->ptask;
	for(int i = 0; i < 14; i++) {
		*(--p_stackptr) = 0xA;
	}
	newTCB->thread_stackptr = p_stackptr;

	// update currentStackSize to the starting address of the NEXT thread stack
//	uint32_t* p_currentStackSize = self->p_currentStackSize;
//	self->p_currentStackSize = (uint32_t)p_currentStackSize - (incomingTask->stack_size);

	self->index++;
	self->numOfActiveTasks++;

	// insert the new TCB into the TCBarray
	self->array[currentIndex] = newTCB;

	// return the TCB we just added
	return self->array[currentIndex];
}

int canReplaceDormantTaskInStack(TCBarray* self, TCB* incomingTask) {
	if (hasReachedMaxActiveTasks(self)) {
		return -1;
	}

	for(int i = 0; i < self->index; i++) {
		if (self->array[i]->state == DORMANT && self->array[i]->stack_size >= incomingTask->stack_size) {
			return i;
		}
	}

	return -1;
}

TCB* getTCBAtCurrentIndex(TCBarray* self) {
	return self->array[self->index];
}

TCB* replaceDormantTaskInStack(TCBarray* self, TCB *incomingTask, int dormantTaskIndex) {
	self->array[dormantTaskIndex]->ptask = incomingTask->ptask;
	self->array[dormantTaskIndex]->tid = incomingTask->tid;
	self->array[dormantTaskIndex]->state = READY;
	self->array[dormantTaskIndex]->stack_size = incomingTask->stack_size;
	self->array[dormantTaskIndex]->alloc_point = incomingTask->alloc_point;
	self->array[dormantTaskIndex]->deadline = incomingTask->deadline;
	self->array[dormantTaskIndex]->remaining_time = incomingTask->remaining_time;
	self->array[dormantTaskIndex]->stack_high = incomingTask->stack_high;

	// Push registers onto the stack - dummy values for now
	uint32_t* p_stackptr = self->array[dormantTaskIndex]->stack_high;
	*(--p_stackptr) = 1<<24; // this is xPSR, setting the chip to Thumb mode
	*(--p_stackptr) = (uint32_t)self->array[dormantTaskIndex]->ptask;
	for(int i = 0; i < 14; i++) {
		*(--p_stackptr) = 0xA;
	}
	self->array[dormantTaskIndex]->thread_stackptr = p_stackptr; //save a thread stack pointer for each thread

	self->numOfActiveTasks++;

	return self->array[dormantTaskIndex];
}

int setTCBState(TCBarray* self, int newState, task_t TID) {
    if (newState != RUNNING && newState != READY && newState != DORMANT) return -1;

    if (TID < 0 || TID > MAX_TASKS - 1) return -1;

    for(int i = 0; i < self->index; i++) {
        if (self->array[i]->tid == TID) {
            self->array[i]->state = newState;
            if (newState == DORMANT) {
            	self->numOfActiveTasks--;
            }
            return 0;
        }
    }

    // No matching TID found.
    return -1;
}

int setTCBStackPointer(TCBarray* self, uint32_t ptr, task_t TID) {
    if (TID < 0 || TID > MAX_TASKS - 1) return -1;

    for(int i = 0; i < self->index; i++) {
        if (self->array[i]->tid == TID) {
            self->array[i]->thread_stackptr = ptr;
            return 0;
        }
    }

    // No matching TID found.
    return -1;
}
