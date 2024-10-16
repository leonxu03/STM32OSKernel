#include "k_mem.h"
#include "main.h"
#include "bit_array.h"
#include <stdio.h>

int k_mem_init() {
//	printf("End of Image:%x\r\n", (uint32_t)&_img_end); // Start of heap
//	printf("End of heap:%x\r\n", (uint32_t)&_estack - (uint32_t)&_Min_Stack_Size); // End of heap
	if (kernelInitialized == false) {
		return RTX_ERR;
	}

	if (memoryInitialized) {
//		printf("Memory already initialized\r\n");
		return RTX_ERR;
	}

//	initBitArray();

	uint32_t initial_block_address = (uint32_t)&_img_end; // + (uint32_t)MEMORY_POOL;
//	printf("initial_block_address is %x\r\n", initial_block_address);
	uint32_t* heap_ptr = initial_block_address;
	Block* initial_block = (Block*)heap_ptr;
	// Set up initial memory block of 32KB
	initial_block->size = (int32_t)MEMORY_POOL;
	initial_block->tid = 0;
	initial_block->magicNumber = -1;
	initial_block->status = FREE;
	initial_block->next = NULL;
	initial_block->prev = NULL;

	freeListArray[0] = initial_block;
	memoryInitialized = 1;

	return RTX_OK;
}

void k_mem_init_UnitTest() {
	uint32_t ptr = freeListArray[0];
	assert(ptr > (uint32_t)&_img_end);
	assert(ptr < ((uint32_t)&_estack - (uint32_t)&_Min_Stack_Size));
}

void* k_mem_alloc(size_t size) {
	if (memoryInitialized == false) {
		return RTX_ERR;
	}

	if (size == 0) {
		return RTX_ERR;
	}

	if(size + sizeof(Block) > 32768){
		return RTX_ERR;
	}

	// level k
	int level = findStartingLevel(size);
	// should check if the requested size is bigger than the max size we have which is 32KB
	// also should check if we have a free node for the requested size
	int currentLevel = -1;
	// go up until it has the free list node
	for(int i = level; i >= 0; i--) {
		if (freeListArray[i] != NULL){
			currentLevel = i;
			break;
		}
	}

	if (currentLevel == -1) {
		return RTX_ERR;
	}

	// have to split until current level matches the desired level (only runs if no free node in original level)
	for(int i = currentLevel; i < level; i++) {
		//2. set bit array index to 1 (for the deleted head node)
		Block* splitBlock = freeListArray[i];

		//1. delete the head node of free list
		if (freeListArray[i]->next != NULL){
			freeListArray[i]->next->prev = NULL;
		}
		freeListArray[i] = splitBlock->next;

		//3. create 2 child nodes (head of the doubly linked list)
		uint32_t newSize = splitBlock->size / 2;

		Block* block1 = (Block*)((uint32_t)splitBlock);
		block1->size = newSize;
		block1->tid = 0;
		block1->magicNumber = -1;
		block1->status = FREE;
		block1->next = NULL;
		block1->prev = NULL;

		Block* block2 = (Block*)((uint32_t)splitBlock + newSize);
		block2->size = newSize;
		block2->tid = 0;
		block2->magicNumber = -1;
		block2->status = FREE;
		block2->next = NULL;
		block2->prev = NULL;

		// Pointer orientation
		block1->next = block2;
		block2->prev = block1;
		block2->next = freeListArray[i + 1];

		if (freeListArray[i + 1] != NULL) {
			freeListArray[i + 1]->prev = block2;
		}
		freeListArray[i + 1] = block1;
		// recursive (by going down the level)

	}
	//4. if i equals to the desired level, set bit array index to 1, remove the node in the free list
	Block* allocationBlock = freeListArray[level];
	allocationBlock->status = ALLOCATED;
	allocationBlock->tid = runningTask ? runningTask->tid : 0;
	allocationBlock->magicNumber = -1;
	freeListArray[level] = freeListArray[level]->next;
	if (freeListArray[level] != NULL) {
		freeListArray[level]->prev = NULL;
	}

	return (void*)((uint32_t)allocationBlock + sizeof(Block));
}

// TODO: More thorough testing of alloc would be nice
void k_mem_alloc_UnitTest() {
	assert(k_mem_init() == RTX_OK);
	void* ptr1 = k_mem_alloc((uint32_t)MEMORY_POOL / 2 - sizeof(Block));
	assert(ptr1 == (uint32_t)&_img_end + sizeof(Block)); // Location of first block, without metadata
	void* ptr2 = k_mem_alloc((uint32_t)MEMORY_POOL / 4 - sizeof(Block));
	assert(ptr2 == (uint32_t)&_img_end + (uint32_t)MEMORY_POOL / 2 + sizeof(Block)); // Location of first block, without metadata
	printf("Completed k_mem_alloc unit test successfully \r\n");
}


int k_mem_dealloc(void* ptr) {
	// Step 0: Check for invalid inputs
	if (memoryInitialized == false) return RTX_ERR;
	if (ptr == NULL) return RTX_ERR;  // if provided ptr to memory region is NULL, return error

	// Step 1: Obtain and check block metadata
	Block *metadata = (Block *)ptr - 1;
	if (runningTask != NULL) { // if dealloc requested by user task, make sure running user task owns the memory region
		if ((uint32_t)(runningTask->tid) != (uint32_t)(metadata->tid)) return RTX_ERR;
	}
	if (metadata->status == FREE) return RTX_ERR; // if memory region is not allocated, return error
	if (metadata->magicNumber != -1) return RTX_ERR; // check if memory region has been corrupted

	// Step 2: Clear metadata of cleared memory region
	uint32_t size = metadata->size;
	int level = findStartingLevel(size - sizeof(Block));
	metadata->status = FREE;
	metadata->tid = 0;

	// Step 3: Add a free list node since this memory region is now cleared
	metadata->prev = NULL;
	metadata->next = freeListArray[level];
	if (metadata->next != NULL) {
		metadata->next->prev = metadata;
	}
	freeListArray[level] = metadata;

	return coalesce(metadata, level);
}

// Coalesces memory blocks if applicable
// Block* block should be a pointer to a free memory block which is also the head of the free list at that level
int coalesce(Block* block, uint32_t level) {

	// if root node, cannot coalesce so return early
	if (block->size == MEMORY_POOL) return RTX_OK;

	for(int lvl = level; lvl > 0; lvl--) {
		// Searches for and captures a pair of free buddy memory regions if it exists
		Block* buddyBlockAddress = getBuddyBlockAddress(block);
		uint32_t foundFreeBuddyBlock = 0;

		Block* freeList = freeListArray[lvl];
		while(freeList != NULL) {
			uint32_t status = freeList->status;
			uint32_t magicNumber = freeList->magicNumber;

			if ((uint32_t)freeList == (uint32_t)buddyBlockAddress) {
				if (status != FREE) return RTX_ERR; // why is buddy on free list if it's not free
				if (magicNumber != -1) return RTX_ERR; // free buddy is corrupted
				foundFreeBuddyBlock = 1; // successfully found free buddy block
			}

			if (foundFreeBuddyBlock) break;

			freeList = freeList->next;
		}

		// If we did NOT find any free buddy memory regions, we return early as we CANNOT coalesce
		// This is a valid case if we don't have a free buddy. Only return RTX_ERR if our free buddy was corrupted
		if (!foundFreeBuddyBlock) return RTX_OK;

		// START COALESCE FLOW
		// Step 1: remove 2 free nodes from the current level
		block->next->prev = NULL;
		freeListArray[lvl] = block->next;
		block->next = NULL;

		if (buddyBlockAddress->prev == NULL) { // if free list node is head of the free list (need to update the pointer to head of free list)
			buddyBlockAddress->next->prev = NULL;
			freeListArray[lvl] = buddyBlockAddress->next;
			buddyBlockAddress->next = NULL;
		} else { // if free list node is in the middle of the free list (don't need to update the pointer to head of free list)
			Block* prevFreeNode = buddyBlockAddress->prev;
			Block* nextNode = buddyBlockAddress->next;
			prevFreeNode->next = nextNode;
			nextNode->prev = prevFreeNode;

			buddyBlockAddress->prev = NULL;
			buddyBlockAddress->next = NULL;
			buddyBlockAddress->status = FREE;
			buddyBlockAddress->tid = 0;
		}

		// Step 2: add a free node to the level above, as we have coalesced two buddies and now we have a new combined memory region at the parent
		Block *parentMemoryBlock = getLowerAddressMemoryBlock(block, buddyBlockAddress);
		parentMemoryBlock->status = FREE;
		parentMemoryBlock->tid = 0;
		parentMemoryBlock->size = parentMemoryBlock->size * 2;

		parentMemoryBlock->prev = NULL;
		parentMemoryBlock->next = freeListArray[lvl - 1];
		if (parentMemoryBlock->next != NULL) {
			parentMemoryBlock->next->prev = parentMemoryBlock;
		}
		freeListArray[lvl - 1] = parentMemoryBlock;

		// recurse to see if we can coalesce at the level above now
		block = parentMemoryBlock;
	}

	return RTX_OK;
}

uint32_t getBuddyBlockAddress(Block* block) {
	uint32_t bottom_address = (uint32_t)&_img_end;
	uint32_t xNode = ((uint32_t)block - bottom_address)/(block->size);

	if(xNode % 2 == 0) {
		return (uint32_t)block + (uint32_t)(block->size);
	}

	return (uint32_t)block - (uint32_t)(block->size);
}

Block* getLowerAddressMemoryBlock(Block* memoryBlock1, Block* memoryBlock2) {
	if ((uint32_t)memoryBlock1 < (uint32_t)memoryBlock2) {
		return memoryBlock1;
	}

	return memoryBlock2;
}

void k_mem_dealloc_UnitTest() {
	k_mem_init();

	TCB tcb;
	tcb.tid = 10;
	runningTask = &tcb;

	// Unit test 1 for freeing the root
	int memRegion = k_mem_alloc(32000);
	assert(freeListArray[0] == NULL);

	int res = k_mem_dealloc(memRegion);
	assert(res == RTX_OK);
	assert(freeListArray[0] != NULL);
	assert((uint32_t)freeListArray[0] == (uint32_t)&_img_end + (uint32_t)MEMORY_POOL);

	// Unit test 2 for deallocation adapted from slide 34 of Lecture 7. Tests the following:
	// 1. freeing A = 4KB at index 7 -> level 3 (tests the freeing one memory block without coalescing case)
	// 2. freeing B = 2KB at index 17 -> level 4 (tests freeing a memory block with 2 levels of coalescing case)
	// 3. freeing C = 8KB at index 4 -> level 2 (tests freeing a memory block with 1 level coalescing case)
	// 4. freeing D = 8KB at index 5 -> level 2 (tests freeing a memory block with coalescing all the way back to the root case)

	int memRegionA = k_mem_alloc(4000);
	assert(res == RTX_OK);

	int memRegionB = k_mem_alloc(2000);
	assert(res == RTX_OK);

	int memRegionC = k_mem_alloc(8000);
	assert(res == RTX_OK);

	int memRegionD = k_mem_alloc(8000);
	assert(res == RTX_OK);

	int resFreeA = k_mem_dealloc(memRegionA);
	assert(res == RTX_OK);
	assert(freeListArray[3] != NULL);

	int resFreeB = k_mem_dealloc(memRegionB);
	assert(res == RTX_OK);
	assert(freeListArray[4] == NULL);
	assert(freeListArray[3] == NULL);
	assert(freeListArray[2] != NULL);

	int resFreeC = k_mem_dealloc(memRegionC);
	assert(res == RTX_OK);

	int resFreeD = k_mem_dealloc(memRegionD);
	assert(res == RTX_OK);
	assert(freeListArray[0] != NULL);

	printf("Completed k_mem_dealloc unit test successfully \r\n");
}

int k_mem_count_extfrag(size_t size) {
	// If memory has not yet been initialized, return 0
	if(memoryInitialized == 0){
		return 0;
	}

	// Early return if size is less than or equal to minimum block size
	if (size <= 32) return 0;

	int level = findStartingLevel(size - sizeof(Block)); // Need to subtract by block size since block size is added in function.

	int fragments = 0;
	for (int i = MAX_ORDER; i >= level; i--) {
		if(freeListArray[i] != NULL){
			Block* current = freeListArray[i];
			while(current != NULL){
				if(current->status == FREE){
					if (current->size < size) {
						fragments++;
					}
				}
				current = current->next;
			}
		}
	}

	return fragments;
}

void k_mem_count_extfrag_UnitTest() {
	assert(k_mem_init() == RTX_OK);

	void* ptr1 = k_mem_alloc((uint32_t)MEMORY_POOL / 2 - sizeof(Block));

	assert(ptr1 == (uint32_t)&_img_end + (uint32_t)MEMORY_POOL - sizeof(Block)); // Location of first block, without metadata
	assert(k_mem_count_extfrag((uint32_t)MEMORY_POOL / 2 + 1) == 1);

	void* ptr2 = k_mem_alloc((uint32_t)MEMORY_POOL / 4 - sizeof(Block));

	assert(ptr2 == (uint32_t)&_img_end + (uint32_t)MEMORY_POOL / 2 - sizeof(Block)); // Location of first block, without metadata
	assert(k_mem_count_extfrag((uint32_t)MEMORY_POOL / 4 + 1) == 1);

	printf("Completed k_mem_count_extfrag unit test successfully \r\n");
}

//could optimize maybe later
size_t sizeOfAllocation(size_t requestedBytes) {
	int bytes = sizeof(Block) + requestedBytes;
	for (int i = 32; i <= (uint32_t)MEMORY_POOL; i *= 2) {
		if (bytes <= i) {
			return i; // Find size that can fit request from 32B, 64B, 128B, and so on.
		}
	}
	// Maximum return-able size is 32KB
	return (uint32_t)MEMORY_POOL;
}

void sizeOfAllocation_UnitTest() {
	assert(sizeOfAllocation(8) == 32);
	assert(sizeOfAllocation(32-sizeof(Block)) == 32);
	assert(sizeOfAllocation(60-sizeof(Block)) == 64);
	assert(sizeOfAllocation(160-sizeof(Block)) == 256);
	assert(sizeOfAllocation((uint32_t)MEMORY_POOL) == (uint32_t)MEMORY_POOL);
	printf("Completed sizeOfAllocation unit test successfully \r\n");
}

// Find level to start checking for free blocks during allocation
int findStartingLevel(size_t size) {
	int bytes = sizeof(Block) + size;
	int level = MAX_ORDER;
	for (int i = 32; i <= (uint32_t)MEMORY_POOL; i *= 2) {
		if (bytes <= i) {
			return level;
		}
		level--;
	}
	return 0;
}
