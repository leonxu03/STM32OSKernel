/*
 * k_task.h
 *
 *  Created on: Jan 5, 2024
 *      Author: nexususer
 *
 *      NOTE: any C functions you write must go into a corresponding c file that you create in the Core->Src folder
 */

#ifndef INC_K_TASK_H_
#define INC_K_TASK_H_

#include "common.h"
#include "main.h"

// define bool manually
// not sure if we're allowed to import "stdbool.h"
#define false 0
#define true 1
typedef int bool;

#define RTX_ERR 0
#define RTX_OK  1

typedef unsigned int U32;
typedef unsigned short U16;
typedef char U8;
typedef unsigned int task_t;

typedef struct task_control_block{
	void (*ptask)(void* args); //entry address
	U32 stack_high; //starting address of stack (high address)
	task_t tid; //task ID
	U8 state; //task's state
	U16 stack_size; //stack size. Must be a multiple of 8
	//your own fields at the end
	uint32_t* thread_stackptr;
	U32 alloc_point;
	int deadline;  // deadline of task, static unless reconfigured
	int remaining_time; // remaining time until reaching deadline
}TCB;

void osKernelInit(void);
int osCreateTask(TCB* task);
int osKernelStart(void);
void osYield(void);
int osTaskInfo(task_t TID, TCB* task_copy);
task_t getTID(void);
int osTaskExit(void);

// Data Structures (queue for TaskScheduler and TIDdispenser)
typedef struct Queue {
	int head;
	int tail;
	int maxSize;
} Queue;

// Data Structures (Heap for TaskScheduler)
typedef struct Heap {
	int currentSize;
	int maxSize;
} Heap;

// Lab 3 interrupt settings.
#define SHPR2 (*((volatile uint32_t*)0xE000ED1C))//SVC is bits 31-28
#define SHPR3 (*((volatile uint32_t*)0xE000ED20))//SysTick is bits 31-28, and PendSV is bits 23-20

// Global Variables
extern bool kernelInitialized;
extern uint32_t* stackPointer; // set stackptr to MSP Initial Value. This is the top of the main stack
extern TCB* runningTask;
extern bool kernelStarted;
extern bool timerResume;

#endif /* INC_K_TASK_H_ */
