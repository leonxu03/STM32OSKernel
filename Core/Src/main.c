/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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
#include <stdio.h> //You are permitted to use this library, but currently only printf is implemented. Anything else is up to you!


/* Declare the constants as external variables */
int count = 0;

/**
  * @brief  The application entry point.
  * @retval int
  */
//int main_prelab(void)
//{
//
//	  /* MCU Configuration: Don't change this or the whole chip won't work!*/
//
//	  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
//	  HAL_Init();
//	  /* Configure the system clock */
//	  SystemClock_Config();
//
//	  /* Initialize all configured peripherals */
//	  MX_GPIO_Init();
//	  MX_USART2_UART_Init();
//	  /* MCU Configuration is now complete. Start writing your code below this line */
//	  uint32_t main_stack_size = (uint32_t)MAIN_STACK_SIZE;
//	  stackptr = *(uint32_t**)0x0;	// Set stackptr to MSP Initial Value
//	  stackptr = stackptr - main_stack_size; // Go to the top of the stack. (Going downward since descending addresses)
//
//	  *(--stackptr) = 1<<24; // This is xPSR, setting the chip to Thumb mode
//	  *(--stackptr) = (uint32_t)print_continuously; // Entry point to the function call
//	  for(; i < 14; i++){
//		  *(--stackptr) = 0xA;
//		  printf("Stackptr is at %p, stack value is %x \r\n", stackptr, *stackptr);
//	  }
//	  __set_PSP(stackptr);
//	  __asm("SVC #17");
//
//	  /* Infinite loop */
//	  /* USER CODE BEGIN WHILE */
//	  while (1)
//	  {
//		/* USER CODE END WHILE */
//		/* USER CODE BEGIN 3 */
//	  }
//	  /* USER CODE END 3 */
//}


//int main_lab1(void) {
//  /* MCU Configuration: Don't change this or the whole chip won't work!*/
//
//  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
//  HAL_Init();
//  /* Configure the system clock */
//  SystemClock_Config();
//
//  /* Initialize all configured peripherals */
//  MX_GPIO_Init();
//  MX_USART2_UART_Init();
//
//  // Kernel Init function
//  // osKernelInit();
//  osKernelInit_UnitTests();
//
//  // Get TID function
//  // getTID();
//  getTID_UnitTest();
//
//  // Get OS Task info function
//  // osTaskInfo();
//  osTaskInfo_UnitTest();
//
//  // OS Create Task function
//  // osCreateTask();
//  osCreateTask_UnitTest();
//
//  // OS Kernel Start Function
//  // osKernelStart();
//  osKernelStart_UnitTest();
//
//  // OS Yield Function
//  // osYield();
//  osYield_SingleTask_UnitTest();
//   osYield_DualTask_UnitTest();
//
//  // OS Task Exit Function
//  // osTaskExit();
//  osTaskExit_NoNewTasks_UnitTest();
//  osTaskExit_YieldToNextTask_UnitTest();
//}


//int main(void) {
//  /* MCU Configuration: Don't change this or the whole chip won't work!*/
//
//  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
//  HAL_Init();
//  count = 0;
//
//  /* Configure the system clock */
//  SystemClock_Config();
//
//  /* Initialize all configured peripherals */
//  MX_GPIO_Init();
//  MX_USART2_UART_Init();
//
////  osKernelInit();
////  osKernelStart_UnitTest();
//  // Prelab3_UnitTest();
//
////  k_mem_init Function
////  k_mem_init();
////  k_mem_init_UnitTest();
//
//
////  k_mem_alloc_UnitTest();
//// 	k_mem_dealloc_UnitTest();
////  k_mem_count_extfrag_UnitTest();
//  //while (1) {
////
//  //}
//
//  // Test MinHeap
////  min_heap_UnitTest();
//
//  // osSetDeadline_UnitTest();
//  //  osCreateDeadlineTask_UnitTest();
//
////  osPeriodYieldTest();
//  return 0;
//
//
////  Miscellaneous Function Tests
////  sizeOfAllocation_UnitTest();
////  findLevelFromBitArray_UnitTest();
////  getXthNodeAtLevel_UnitTest();
////  getChildBitArrayIndex_UnitTest();
//
//}
