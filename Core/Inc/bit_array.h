#ifndef BIT_ARRAY_H
#define BIT_ARRAY_H

#include "k_mem.h"

//// BitArray
//typedef struct bitArray{
//	uint32_t array[NUM_NODES];
//} bitArray;

//extern bitArray buddyBitArray;
extern Block* freeListArray[MAX_ORDER + 1];

//void initBitArray();
void initLinkedList(Block* initial);

#endif
