#include "k_task.h"
#include "k_mem.h"
#include "main.h"

bool kernelInitialized = false;
uint32_t* stackPointer = NULL;
TCB* runningTask = NULL;
bool kernelStarted = false;
bool timerResume = false;

bool memoryInitialized = false;
