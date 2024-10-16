/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    stm32f4xx_it.c
  * @brief   Interrupt Service Routines.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "stm32f4xx_it.h"
#include "k_task.h"
#include "tcb_array.h"
#include "task_scheduler.h"
/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN TD */

/* USER CODE END TD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */
unsigned int svc_number;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/* External variables --------------------------------------------------------*/
//extern int count;
/* USER CODE BEGIN EV */
/* USER CODE END EV */

/******************************************************************************/
/*           Cortex-M4 Processor Interruption and Exception Handlers          */
/******************************************************************************/
/**
  * @brief This function handles Non maskable interrupt.
  */
void NMI_Handler(void)
{
  /* USER CODE BEGIN NonMaskableInt_IRQn 0 */

  /* USER CODE END NonMaskableInt_IRQn 0 */
  /* USER CODE BEGIN NonMaskableInt_IRQn 1 */
  while (1)
  {
  }
  /* USER CODE END NonMaskableInt_IRQn 1 */
}

/**
  * @brief This function handles Hard fault interrupt.
  */
void HardFault_Handler(void)
{
  /* USER CODE BEGIN HardFault_IRQn 0 */

  /* USER CODE END HardFault_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_HardFault_IRQn 0 */
    /* USER CODE END W1_HardFault_IRQn 0 */
  }
}

/**
  * @brief This function handles Memory management fault.
  */
void MemManage_Handler(void)
{
  /* USER CODE BEGIN MemoryManagement_IRQn 0 */

  /* USER CODE END MemoryManagement_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_MemoryManagement_IRQn 0 */
    /* USER CODE END W1_MemoryManagement_IRQn 0 */
  }
}

/**
  * @brief This function handles Pre-fetch fault, memory access fault.
  */
void BusFault_Handler(void)
{
  /* USER CODE BEGIN BusFault_IRQn 0 */

  /* USER CODE END BusFault_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_BusFault_IRQn 0 */
    /* USER CODE END W1_BusFault_IRQn 0 */
  }
}

/**
  * @brief This function handles Undefined instruction or illegal state.
  */
void UsageFault_Handler(void)
{
  /* USER CODE BEGIN UsageFault_IRQn 0 */

  /* USER CODE END UsageFault_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_UsageFault_IRQn 0 */
    /* USER CODE END W1_UsageFault_IRQn 0 */
  }
}

/**
  * @brief This function handles System service call via SWI instruction.
  */
//void SVC_Handler(void)
//{
//  /* USER CODE BEGIN SVCall_IRQn 0 */
//
//  /* USER CODE END SVCall_IRQn 0 */
//  /* USER CODE BEGIN SVCall_IRQn 1 */
//
//  /* USER CODE END SVCall_IRQn 1 */
//}

/**
  * @brief This function handles Debug monitor.
  */
void DebugMon_Handler(void)
{
  /* USER CODE BEGIN DebugMonitor_IRQn 0 */

  /* USER CODE END DebugMonitor_IRQn 0 */
  /* USER CODE BEGIN DebugMonitor_IRQn 1 */

  /* USER CODE END DebugMonitor_IRQn 1 */
}

/**
  * @brief This function handles Pendable request for system service.
  */
//void PendSV_Handler(void)
//{
//  /* USER CODE BEGIN PendSV_IRQn 0 */
//	__asm("MRS R0, PSP");
//	__asm("STMDB R0!, {R4-R11}");
//	__asm("MSR PSP, R0");
//	__asm("BL Context_Switch");
//	__asm("MRS R0, PSP");
//	__asm("LDMIA R0!, {R4-R11}");
//	__asm("MSR PSP, R0");
//	__asm("LDR LR, =0xFFFFFFFD");
//	__asm("BX LR");
//  /* USER CODE END PendSV_IRQn 0 */
//  /* USER CODE BEGIN PendSV_IRQn 1 */
//
//  /* USER CODE END PendSV_IRQn 1 */
//}

/**
  * @brief This function handles System tick timer.
  */
void SysTick_Handler(void)
{
  /* USER CODE BEGIN SysTick_IRQn 0 */

  /* USER CODE END SysTick_IRQn 0 */
  HAL_IncTick();
  /* USER CODE BEGIN SysTick_IRQn 1 */

  // Prelab 3
//   printf("num is %d\r\n", count);

  if (timerResume) {
	  // Decrement Both Sleep and Scheduler Heap Tasks
	  decrementRemainingTime(&kernelSleepTaskScheduler);
	  decrementRemainingTime(&kernelTaskScheduler);
	  // Don't decrement woken up tasks again on the scheduler

	  TCB* minSleepTask = getMinRemainingTimeTask(&kernelSleepTaskScheduler);
	  while (minSleepTask != NULL) {
		  if (minSleepTask->remaining_time <= 0) { // Remove all sleeping tasks that are done sleeping and place back into scheduler
			  TCB* wokenUpTask = removeMin(&kernelSleepTaskScheduler);
			  wokenUpTask->remaining_time = wokenUpTask->deadline;
			  insert(&kernelTaskScheduler, wokenUpTask);
		  } else {
			  break;
		  }

		  minSleepTask = getMinRemainingTimeTask(&kernelSleepTaskScheduler);
	  }

	  TCB* minScheduledTask = getMinRemainingTimeTask(&kernelTaskScheduler);
	  if (runningTask != minScheduledTask || runningTask->remaining_time <= 0) { // if the top of the heap doesn't match the runningTask (that means something new entered that should pre-empt the running task)
		  // Context switch to next task											  // or the currently running task expired
		  timerResume = false;
      if(minScheduledTask->remaining_time <= 0) {
        svc_number = 1; // reset remaining_time if task expires naturally
      } else {
        svc_number = 3; // don't reset remaining_time of running_task if it gets preempted
      }
		  SCB->ICSR |= 1<<28; //TODO: What to do when remaining time expires
		  __asm("isb");
		  timerResume = true;
	  }

  }

  /* USER CODE END SysTick_IRQn 1 */
}

void SVC_Handler_Main( unsigned int *svc_args )
{
//  unsigned int svc_number;
  /*
  * Stack contains:
  * r0, r1, r2, r3, r12, r14, the return address and xPSR
  * First argument (r0) is svc_args[0]
  */
  svc_number = ( ( char * )svc_args[ 6 ] )[ -2 ] ;
  switch( svc_number )
  {
    case 0:  /* EnablePrivilegedMode */
    	 __set_CONTROL( __get_CONTROL( ) & ~CONTROL_nPRIV_Msk ) ;
      	 break;
    case 1: // Context Switch for OsYield: For PendSV_Handler entry
		SCB->ICSR |= 1<<28; // Control register bit for a PendSV interrupt
		__asm("isb");
		break;
    case 2: // Kernel Start
    	__asm("MRS R0, PSP");
		__asm("LDMIA R0!, {R4 - R11}");
    	__asm("MSR PSP, R0");
    	__asm("LDR LR, =0xFFFFFFFD");
    	__asm("BX LR");
    	break;
    case 3: // Context Switch for Pre-emption
    	SCB->ICSR |= 1<<28; // Control register bit for a PendSV interrupt
    	__asm("isb");
    case 4: // Context Switch for osSleep: for PendSV_Handler entry
    	SCB->ICSR |= 1<<28;
    	__asm("isb");
    default:    /* unknown SVC */
    	break;
  }
}


void Context_Switch( void ) {
	// Saves old task
	TCB* oldTask = runningTask; // Check that this actually holds the last running task

	// Case 1: osTaskExit
	if (oldTask->state == DORMANT) {
		removeMin(&kernelTaskScheduler); // Remove from scheduler
	} else {
		// Case 2: osYield
		if (svc_number == 1) {
			oldTask->state = READY;
			oldTask->remaining_time = oldTask->deadline; // Reset remaining_time to original deadline
			changeRemainingTime(&kernelTaskScheduler, oldTask->tid, oldTask->deadline); // Reheapify the min-heap to place new task at root.
		}

		// Case 3: osSleep
		if (svc_number == 4) {
			oldTask->state = SLEEPING;
			oldTask->thread_stackptr = (uint32_t)__get_PSP();
			insert(&kernelSleepTaskScheduler, oldTask);
			removeMin(&kernelTaskScheduler);
		} else { // state should not be set to READY if osSleep function triggered context switch
			oldTask->state = READY;
			oldTask->thread_stackptr = (uint32_t)__get_PSP();
			setTCBState(&kernelTCBarray, READY, oldTask->tid);
			setTCBStackPointer(&kernelTCBarray, oldTask->thread_stackptr, oldTask->tid); // Might be redundant
		}
	}

	// Get the next task
	TCB* newTask = getMinRemainingTimeTask(&kernelTaskScheduler);
	runningTask = newTask;
	// Set new task as running
	setTCBState(&kernelTCBarray, RUNNING, runningTask->tid);
	runningTask->state = RUNNING;
	stackPointer = (uint32_t)runningTask->thread_stackptr;
	__set_PSP((uint32_t)runningTask->thread_stackptr);

	// printf("ALERT--------------\r\n OLD_TASK_TID = %d, TOP OF HEAP TID = %d\r\n", oldTask->tid, runningTask->tid);

	timerResume = true;
}

/******************************************************************************/
/* STM32F4xx Peripheral Interrupt Handlers                                    */
/* Add here the Interrupt Handlers for the used peripherals.                  */
/* For the available peripheral interrupt handler names,                      */
/* please refer to the startup file (startup_stm32f4xx.s).                    */
/******************************************************************************/

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */
