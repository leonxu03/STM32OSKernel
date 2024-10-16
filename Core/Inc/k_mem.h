/*
 * k_mem.h
 *
 *  Created on: Jan 5, 2024
 *      Author: nexususer
 *
 *      NOTE: any C functions you write must go into a corresponding c file that you create in the Core->Src folder
 */

#ifndef INC_K_MEM_H_
#define INC_K_MEM_H_

#include "k_task.h"
#include "main.h"

// config for buddy memory manager
#define BIT_ARRAY_N 15 // 15 for now, may need to increase for bonus
#define MEMORY_POOL (1 << BIT_ARRAY_N) // 32 KibiBytes
#define MIN_BLOCK_ORDER 5
#define MIN_BLOCK_SIZE (1 << MIN_BLOCK_ORDER) // 32 Bytes
#define MAX_ORDER 10 /* change it to >= 11 for bonus */
#define BIT_ARRAY_HEIGHT (BIT_ARRAY_N - MIN_BLOCK_ORDER + 1)
#define NUM_NODES (1 << BIT_ARRAY_HEIGHT) - 1

#define FREE 0
#define ALLOCATED 1

typedef struct Block { //MetaData
	uint32_t size; // Size of block
	uint32_t tid; // TID of task that requested memory
	uint32_t magicNumber; // TID of task that owns allocated block
	uint32_t status; // Might not be needed
	struct Block* next; // Pointer to next free node in list.
	struct Block* prev; // Pointer to previous free node in list.
//	uint32_t bitArrayIndex; // Index in the bit array.
} Block;


//typedef struct listNode {
//	Block* memoryBlock; // Pointer to the block of memory in the heap that is available for the free node.
//	struct listNode* next; // Pointer to next free node in list.
//	struct listNode* prev; // Pointer to previous free node in list.
//} listNode;

//typedef struct bitArrayBlock {
//	uint32_t level;
//	uint32_t size;
//	uint32_t status;
//} bitArrayBlock;

int k_mem_init();
void* k_mem_alloc(size_t size);
int k_mem_dealloc(void* ptr);
int k_mem_count_extfrag(size_t size);
void k_mem_init_UnitTest();
size_t sizeOfAllocation(size_t requestedBytes);
void sizeOfAllocation_UnitTest();
int findStartingLevel(size_t size);
void k_mem_alloc_UnitTest();
uint32_t getBuddyBlockAddress(Block* block);
Block* getLowerAddressMemoryBlock(Block* memoryBlock1, Block* memoryBlock2);
void k_mem_dealloc_UnitTest();
void k_mem_count_extfrag_UnitTest();
int coalesce(Block* block, uint32_t level);

extern bool memoryInitialized;
extern uint32_t _img_end;
extern uint32_t _estack;
extern uint32_t _Min_Stack_Size;
#endif /* INC_K_MEM_H_ */
