#include "main.h"
#include "bit_array.h"
#include "common.h"
#include <stdio.h>

//bitArray buddyBitArray;
Block* freeListArray[MAX_ORDER + 1]; // !! :eve;s

// The bit array might actually be very simple, as an array of just 1s and 0s.

//void initBitArray() {
////;	int k = 0; // Level counter
////	int level_index = 0;
//	for (int i = 0; i < NUM_NODES; i++) {
//
////		buddyBitArray.array[i].size = 1 << (BIT_ARRAY_N - k);
////		buddyBitArray.array[i].level = k;
////		buddyBitArray.array[i].status = 0;
//		buddyBitArray.array[i] = 0;
////		level_index++;
////		if (level_index == (1 << k)) {
////			k++;
////			level_index = 0;
////		}
//	}
//}


void initLinkedList(Block* initial) {
	for (int i = 0; i <= MAX_ORDER; i++) {
		freeListArray[i] = NULL;
	}
	// Level 0 free list must hold the entire memory block initially.
	freeListArray[0] = initial;
}


