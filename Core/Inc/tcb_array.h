#ifndef TCB_ARRAY_H
#define TCB_ARRAY_H

#include "k_task.h"
//extern uint32_t MAIN_STACK_SIZE[];

// TCBarray
typedef struct TCBarray{
	TCB* array[30]; // holds active and dormant TCBs.
							  // at a total thread stack size of 0x3C00 and a minimum thread stack of 0x200, a maximum of 30 threads could be put on to our stack.
							  // note: 30 threads could be on our stack, but only 16 can be active/scheduled at once
	uint32_t* p_currentStackSize; //current stack size. Points to the address of the potential next thread. See end of prelab page 2.
//	uint32_t* p_stackLimit; // stack size limit of main stack. This is MSP - 0x4000.
							// Anything past this bound and we'd have to start seeing if we can replace dormant TCBs.
	int index;		// index of next free TCB space
	int numOfActiveTasks; // current number of active tasks (includes scheduled + running task)
} TCBarray;

extern TCBarray kernelTCBarray;
extern TCB tcbs[30];

void initTCBarray();
void initNULLTCB(TCB* tcb);
uint32_t* currentStackSize(TCBarray* self);
uint32_t* stackLimit(TCBarray* self);
bool isArrayFull(TCBarray* self);
bool hasReachedMaxActiveTasks(TCBarray* self);
int numOfActiveTasks(TCBarray* self);
bool canAppendToStack(TCBarray* self, TCB* incomingTask);
TCB* appendToStack(TCBarray* self, TCB *incomingTask);
int canReplaceDormantTaskInStack(TCBarray* self, TCB* incomingTask);
TCB* getTCBAtCurrentIndex(TCBarray* self);
TCB* replaceDormantTaskInStack(TCBarray* self, TCB *incomingTask, int dormantTaskIndex);
int setTCBState(TCBarray* self, int newState, task_t TID);
int setTCBStackPointer(TCBarray* self, uint32_t ptr, task_t TID);

#endif
