#include "main.h"
#include "k_task.h"
#include "task_scheduler.h"
#include "tcb_array.h"
#include "tid_dispenser.h"
#include "common.h"
#include "k_mem.h"
#include <stdio.h>

extern int count;

void print_continuously(void) {
	while (1) {
//		printf("Thread one\r\n");
		osYield();
	}
}

void print_continuously_two(void) {
	while (1) {
//		printf("Thread two two two two\r\n");
		osYield();// Hard faulting after osYield
	}
}

void print_until_exit(void) {
//	for (int i = 3; i >= 0; i--) {
//		printf("Exiting in %d.\r\n", i);
//	}
	osTaskExit();
}

// from osSleep function definition in lab manual
// "if no tasks are available to run (that is, all tasks are sleeping), the expected behaviour"
// "is for the NULL task to run until a task emerges from sleep"
void do_nothing(void) {
	while(1) {
//		osYield();
//		count = 0;
	}
}

void no_yield(void) {
	while(1) {
		count = 1;
	}
}

void no_yield_with_sleep(void) {
	while(1) {
		count = 1;
		osSleep(1000);
	}
}

void no_yield2(void) {
	while(1) {
		count = 2;
	}
}

void no_yield2_with_sleep(void) {
	while(1) {
		count = 2;
		osSleep(5000);
	}
}

void period_yield(void) {
	while(1) {
		count += 1;
		osPeriodYield();
	}
}

// osKernelInit function initializes the following kernel-level data structures
// TaskScheduler (queue) -> holds tasks ready to be run
// TCBarray (array) -> holds created tasks that exist in our main stack. Includes both active and dormant tasks
// TIDdispenser (queue) -> helps assign TIDs when tasks are created
void osKernelInit(void) {
	//Interrupt setting
	SHPR3 = (SHPR3 & ~(0xFFU << 24)) | (0xF0U << 24);//SysTick is lowest priority (highest number)
	SHPR3 = (SHPR3 & ~(0xFFU << 16)) | (0xE0U << 16);//PendSV is in the middle

	SHPR2 = (SHPR2 & ~(0xFFU << 24)) | (0xD0U << 24);//SVC is highest priority (lowest number)

	initTaskScheduler(&kernelTaskScheduler);
	initSleepTaskScheduler(&kernelSleepTaskScheduler);
	initTCBarray(&kernelTCBarray);
	initTIDdispenser(&kernelTIDdispenser);

	// Setup the null task
	TCB nullTask;
	nullTask.ptask = (uint32_t)do_nothing;
	nullTask.stack_size = STACK_SIZE;
	osCreateTask(&nullTask); //Maximum integer size deadline

	stackPointer = *(uint32_t**)0x0;
	stackPointer = stackPointer - (uint32_t)MAIN_STACK_SIZE;
	runningTask = NULL;
	kernelInitialized = true;

	k_mem_init();
}

// osTaskInfo function
// looks for a task with the associated TID, and populates the task_copy with the found values
int osTaskInfo(task_t TID, TCB* task_copy) {
	if (TID < 0) {
		return RTX_ERR;
	}

	if (TID >= MAX_TASKS) {
		return RTX_ERR;
	}

	if (task_copy == NULL) {
		return RTX_ERR;
	}

	for (int i = 0; i < 30; i++) {
		if (kernelTCBarray.array[i] == NULL) {
			continue;
		}

		if (kernelTCBarray.array[i]->tid == TID) {
			if (kernelTCBarray.array[i]->state == DORMANT) { // Dormant tasks should not return data
				return RTX_ERR;
			}
			task_copy->ptask = kernelTCBarray.array[i]->ptask;
			task_copy->stack_high = kernelTCBarray.array[i]->stack_high;
			task_copy->tid = kernelTCBarray.array[i]->tid;
			task_copy->state = kernelTCBarray.array[i]->state;
			task_copy->stack_size = kernelTCBarray.array[i]->stack_size;
			task_copy->thread_stackptr = kernelTCBarray.array[i]->thread_stackptr;
			task_copy->deadline = kernelTCBarray.array[i]->deadline;
			task_copy->remaining_time = kernelTCBarray.array[i]->remaining_time;
			return RTX_OK;
		}
	}

	return RTX_ERR;
}

// getTID function
// gets TID of currently running task
task_t osGetTID(void) {
	if (kernelInitialized == false) {
		return 0;
	}

	if (runningTask == NULL) {
		return 0;
	}

	return runningTask->tid;
}

// osCreateTask
int osCreateTask(TCB* task) {
	timerResume = false;
	if (task == NULL) {
		if (kernelStarted) {
			timerResume = true;
		}
		return RTX_ERR;
	}

	if (hasReachedMaxActiveTasks(&kernelTCBarray)) {
		if (kernelStarted) {
			timerResume = true;
		}
		return RTX_ERR;
	}

	if (outOfTIDs(&kernelTIDdispenser)) {
		if (kernelStarted) {
			timerResume = true;
		}
		return RTX_ERR;
	}

	if (isFull(&kernelTaskScheduler)) {
		if (kernelStarted) {
			timerResume = true;
		}
		return RTX_ERR;
	}

	if (task->stack_size < STACK_SIZE) {
		if (kernelStarted) {
			timerResume = true;
		}
		return RTX_ERR;
	}

	if (task->ptask == NULL) {
		if (kernelStarted) {
			timerResume = true;
		}
		return RTX_ERR;
	}

	// prioritize dormant replacements first
	int dormantTaskIndex = canReplaceDormantTaskInStack(&kernelTCBarray, task);
	if (dormantTaskIndex >= 0) {
		task->alloc_point = k_mem_alloc(task->stack_size);
		if (task->alloc_point == NULL) {
			if (kernelStarted) {
				timerResume = true;
			}
			return RTX_ERR;
		}

		Block *metadata = (Block *)(task->alloc_point) - 1;
		task->stack_high = task->alloc_point + metadata->size - sizeof(Block);
		int TID = dequeueTID(&kernelTIDdispenser);
		task->tid = TID;
		metadata->tid = TID;
		task->deadline = 5; // should be 5ms
		task->remaining_time = 5; // should be 5ms
		TCB* replacedTCB = replaceDormantTaskInStack(&kernelTCBarray, task, dormantTaskIndex);
		insert(&kernelTaskScheduler, replacedTCB);
//		enqueue(&kernelTaskScheduler, replacedTCB);
		if (kernelStarted) {
			__asm("SVC #3"); // Context Switch

			// enable interrupt
			timerResume = true;
		}
		return RTX_OK;
	}

	if (canAppendToStack(&kernelTCBarray, task)) {
		int TID = dequeueTID(&kernelTIDdispenser);
		task->tid = TID;
		if (TID == 0) {
			task->deadline = 2147483647;
			task->remaining_time = 2147483647;
			task->stack_high = kernelTCBarray.p_currentStackSize;
			task->alloc_point = kernelTCBarray.p_currentStackSize;
		} else {
			task->alloc_point = k_mem_alloc(task->stack_size);
			if (task->alloc_point == NULL) {
				if (kernelStarted) {
					timerResume = true;
				}
				return RTX_ERR;
			}

			Block *metadata = (Block *)(task->alloc_point) - 1;
			task->stack_high = task->alloc_point + metadata->size - sizeof(Block);

			metadata->tid = TID;
			task->deadline = 5;
			task->remaining_time = 5;
		}
		TCB* appendedTCB = appendToStack(&kernelTCBarray, task);
		insert(&kernelTaskScheduler, appendedTCB);
//		enqueue(&kernelTaskScheduler, appendedTCB);

		if (kernelStarted) {
			__asm("SVC #3"); // Context Switch

			// enable interrupt
			timerResume = true;
		}
		return RTX_OK;
	}

	timerResume = true;
	return RTX_ERR;
}

int osKernelStart(void) {

	if (kernelInitialized == false) {
		return RTX_ERR;
	}

	if (kernelStarted == true) {
		return RTX_ERR;
	}

	if (isEmpty(&kernelTaskScheduler)) {
		return RTX_ERR;
	}

	runningTask = getMinRemainingTimeTask(&kernelTaskScheduler);
	setTCBState(&kernelTCBarray, RUNNING, runningTask->tid);
	runningTask->state = RUNNING; // This is redundant

	// Since the timer begins before osKernelStart(), the first 1ms timer interrupt is somewhat inaccurate
	// For example, if the kernel is started at 0.8ms and the timer is triggered at 1ms,
	// then the remainingTime will decrement by 1ms despite only 0.2ms having elapsed.
	// to compensate for this, add a small delay to the tasks that are ready at the start of osKernelStart()
	for(int i = 0; i<kernelTaskScheduler.heap->currentSize; i++) {
		if (kernelTaskScheduler.array[i]->tid != 0) {
			kernelTaskScheduler.array[i]->remaining_time++;
		}
	}

	stackPointer = (uint32_t)runningTask->thread_stackptr;
	__set_PSP(stackPointer);
	kernelStarted = true;
	timerResume = true;

	__asm("SVC #2");

	return RTX_OK;
}

// osYield
void osYield(void) {
	if (runningTask == NULL) {
			return;
	}

	timerResume = false;
	// PendSV_Handler saves to PSP the register values of the old task, then grabs new PSP from new task
	// and then loads registers from the new task's state.

	__asm("SVC #1");
	timerResume = true;
}

// osTaskExit
int osTaskExit(void) {
	if (runningTask == NULL) {
		return RTX_ERR;
	}
	timerResume = false;
	// Find TCB copy in TCB array and set to dormant.
	runningTask->state = DORMANT;
	setTCBState(&kernelTCBarray, DORMANT, runningTask->tid);
	enqueueTID(&kernelTIDdispenser, runningTask->tid);

	// deallocate memory
	k_mem_dealloc(runningTask->alloc_point);
//	__set_PSP((uint32_t)(runningTask->stack_high));
	__asm("SVC #1");
	timerResume = true;
	return RTX_OK;
}

int osSetDeadline(int deadline, task_t TID) {
	timerResume = false;
	// disable interrupt
	if (deadline <= 0) {

		// enable interrupt
		timerResume = true;
		return RTX_ERR;
	}

	if (runningTask->tid == TID) {

		// enable interrupt
		timerResume = true;
		return RTX_ERR;
	}

	if (TID <= 0) {
		// enable interrupt
		timerResume = true;
		return RTX_ERR;
	}

	TCB* oldTask = runningTask; // Keep track of runningTask so if the task with shortest remaining time is different, context switch.
	if (changeRemainingTime(&kernelTaskScheduler, TID, deadline)) {
		TCB* newTask = getMinRemainingTimeTask(&kernelTaskScheduler);
		if (oldTask->tid != newTask->tid) {
			__asm("SVC #3"); // Context Switch
		}

		// enable interrupt
		timerResume = true;
		return RTX_OK;
	}
	// No task with matching TID found

	// enable interrupt
	timerResume = true;
	return RTX_ERR;
}

int osCreateDeadlineTask(int deadline, TCB* task) {
	timerResume = false;	// disable interrupt

	if (task == NULL) {
		if (kernelStarted) {
			timerResume = true;
		}
		return RTX_ERR;
	}

	if (hasReachedMaxActiveTasks(&kernelTCBarray)) {
		if (kernelStarted) {
			timerResume = true;
		}
		return RTX_ERR;
	}

	if (outOfTIDs(&kernelTIDdispenser)) {
		if (kernelStarted) {
			timerResume = true;
		}
		return RTX_ERR;
	}

	if (isFull(&kernelTaskScheduler)) {
		if (kernelStarted) {
			timerResume = true;
		}
		return RTX_ERR;
	}

	if (task->stack_size < STACK_SIZE) {
		if (kernelStarted) {
			timerResume = true;
		}
		return RTX_ERR;
	}

	if (task->ptask == NULL) {
		if (kernelStarted) {
			timerResume = true;
		}
		return RTX_ERR;
	}

	if (deadline <= 0) {
		if (kernelStarted) {
			timerResume = true;
		}
		return RTX_ERR;
	}

	int dormantTaskIndex = canReplaceDormantTaskInStack(&kernelTCBarray, task);
	if (dormantTaskIndex >= 0) {
		task->alloc_point = k_mem_alloc(task->stack_size);
		if (task->alloc_point == NULL) {
			if (kernelStarted) {
				timerResume = true;
			}
			return RTX_ERR;
		}

		Block *metadata = (Block *)(task->alloc_point) - 1;
		task->stack_high = task->alloc_point + metadata->size - sizeof(Block);
		int TID = dequeueTID(&kernelTIDdispenser);
		task->tid = TID;
		metadata->tid = TID;
		task->deadline = deadline;
		task->remaining_time = deadline;
		TCB* replacedTCB = replaceDormantTaskInStack(&kernelTCBarray, task, dormantTaskIndex);
		insert(&kernelTaskScheduler, replacedTCB);
		if (kernelStarted) {
			__asm("SVC #3");	// Context Switch
			timerResume = true;	// enable interrupt
		}
		return RTX_OK;
	}

	if (canAppendToStack(&kernelTCBarray, task)) {
		task->alloc_point = k_mem_alloc(task->stack_size);
		if (task->alloc_point == NULL) {
			if (kernelStarted) {
				timerResume = true;
			}
			return RTX_ERR;
		}

		Block *metadata = (Block *)(task->alloc_point) - 1;
		task->stack_high = task->alloc_point + metadata->size - sizeof(Block);
		int TID = dequeueTID(&kernelTIDdispenser);
		task->tid = TID;
		metadata->tid = TID;
		task->deadline = deadline;
		task->remaining_time = deadline;
		TCB* appendedTCB = appendToStack(&kernelTCBarray, task);
		insert(&kernelTaskScheduler, appendedTCB);
		if (kernelStarted) {
			__asm("SVC #3");	// Context Switch
			timerResume = true;	// enable interrupt
		}
		return RTX_OK;
	}

	timerResume = true;		// enable interrupt
	return RTX_ERR;
}

// Important
// "A key distinction is that in the EDF, while one task is RUNNING, all the READY tasks's deadlines are also approaching.
// And all the SLEEPING tasks's wakeup times are also approaching. Hence time ticks down for *all* tasks"
void osSleep(int timeInMs) {
	// code should be similar to osYield
//	timerResume = 0;
	
	if (kernelInitialized == false || kernelStarted == false) {
		return;
	}

	// Pseudocode flow
	// 1. runningTask -> remainingTime = timeInMs
//	TCB* runningTask = getMinRemainingTimeTask(&kernelTaskScheduler);
	runningTask->remaining_time = timeInMs; // set sleep time

	timerResume = 0;
	__asm("SVC #4");

	// 2. save context (find out how to do this)
	// 3. add runningTask node to sleep min heap
	//    sleep MinHeap: remainingTime should decrement in the sys timer interrupt handler
	//    once reminaingTime expires:
	//    EDF scheduler MinHeap: add runningTask in sleep MinHeap to EDF Scheduler and reheapify
	//    Sleep MinHeap: move last node to the top and reheapify
	// 4. trigger context switch
	//    EDF scheduler MinHeap, move last node to the top and reheapify
	// 5. load context of new task
	// 6. new next task should be running now

	//

	// 5 .
//	timerResume = 1;
}

void osPeriodYield() {

	if (kernelInitialized == false || kernelStarted == false) {
		return;
	}

	// Needs to wait till remaining time elapses before being scheduled
	// Switches to SLEEP state
//	TCB* runningTask = getMinRemainingTimeTask(&kernelTaskScheduler);
	osSleep(runningTask->remaining_time);

}
