/*
 * =====================================================================================
 *
 *	Filename:  		sma.c
 *
 *  Description:	Base code for Assignment 3 for ECSE-427 / COMP-310
 *
 *  Version:  		1.0
 *  Created:  		6/11/2020 9:30:00 AM
 *  Revised:  		-
 *  Compiler:  		gcc
 *
 *  Author:  		Mohammad Mushfiqur Rahman
 *      
 *  Instructions:   Please address all the "TODO"s in the code below and modify 
 * 					them accordingly. Feel free to modify the "PRIVATE" functions.
 * 					Don't modify the "PUBLIC" functions (except the TODO part), unless
 * 					you find a bug! Refer to the Assignment Handout for further info.
 * =====================================================================================
 */

/* Includes */
#include "sma.h" // Please add any libraries you plan to use inside this file

/* Definitions*/
#define MAX_TOP_FREE (128 * 1024)									  // Max top free block size = 128 Kbytes
#define FREE_BLOCK_HEADER_SIZE (2 * sizeof(char *) + 2 * sizeof(int)) // Size of the Header in a free memory block
//	TODO: Add constants here
#define BLOCK_POINTER_SIZE 8
#define INT_SIZE 4
#define BOOL_SIZE 1
#define FREE 0
#define ALLOCATED 1

#define SIZE_TYPE 0
#define ADDRESS_TYPE 1
#define POINTER_TYPE 2
#define TAG_TYPE 3

// BLOCK FORMAT: | - | - - - - | - - | - - | - - - - - - - - - - |
//				  tag     size   prev  next          data
typedef enum
{
	WORST,
	NEXT
} Policy;

char *sma_malloc_error;
void *freeListHead = NULL;
void *freeListTail = NULL;

unsigned long totalAllocatedSize = 0;
unsigned long totalFreeSize = 0;
Policy currentPolicy = NEXT;
//	TODO: Add any global variables here
void *heapBreak;
void *heapStart;
bool isFirstBreak = 1;

void *nextFitPointer;

/*
 * =====================================================================================
 *	Public Functions for SMA
 * =====================================================================================
 */

/*
 *	Funcation Name: sma_malloc
 *	Input type:		int
 * 	Output type:	void*
 * 	Description:	Allocates a memory block of input size from the heap, and returns a 
 * 					pointer pointing to it. Returns NULL if failed and sets a global error.
 */
void *sma_malloc(int size)
{

	void *pMemory = NULL;

	// Checks if the free list is empty
	if (freeListHead == NULL)
	{
		// Allocate memory by increasing the Program Break
		pMemory = allocate_pBrk(size);
	}
	// If free list is not empty
	else
	{

		// Allocate memory from the free memory list
		pMemory = allocate_freeList(size);

		// If a valid memory could NOT be allocated from the free memory list
		if (pMemory == (void *)-2)
		{
			// Allocate memory by increasing the Program Break
			pMemory = allocate_pBrk(size);
		}
	}

	// Validates memory allocation
	if (pMemory < 0 || pMemory == NULL)
	{
		sma_malloc_error = "Error: Memory allocation failed!";
		return NULL;
	}

	// Updates SMA Info
	totalAllocatedSize += size;
	return pMemory;
}

/*
 *	Funcation Name: sma_free
 *	Input type:		void*
 * 	Output type:	void
 * 	Description:	Deallocates the memory block pointed by the input pointer
 */
void sma_free(void *ptr)
{

	//	Checks if the ptr is NULL
	if (ptr == NULL)
	{
		puts("Error: Attempting to free NULL!");
	}
	//	Checks if the ptr is beyond Program Break
	else if (ptr > sbrk(0))
	{
		puts("Error: Attempting to free unallocated space!");
	}
	else
	{
		//	Adds the block to the free memory list
		add_block_freeList(ptr);
	}
}

/*
 *	Funcation Name: sma_mallopt
 *	Input type:		int
 * 	Output type:	void
 * 	Description:	Specifies the memory allocation policy
 */
void sma_mallopt(int policy)
{
	// Assigns the appropriate Policy
	if (policy == 1)
	{
		currentPolicy = WORST;
	}
	else if (policy == 2)
	{
		currentPolicy = NEXT;
	}
}

/*
 *	Funcation Name: sma_mallinfo
 *	Input type:		void
 * 	Output type:	void
 * 	Description:	Prints statistics about the memory allocation by SMA so far.
 */
void sma_mallinfo()
{
	//	Finds the largest Contiguous Free Space (should be the largest free block)
	int largestFreeBlock = get_largest_freeBlock();
	char str[60];

	//	Prints the SMA Stats
	sprintf(str, "Total number of bytes allocated: %lu", totalAllocatedSize);
	puts(str);
	sprintf(str, "Total free space: %lu", totalFreeSize);
	puts(str);
	sprintf(str, "Size of largest contigious free space (in bytes): %d", largestFreeBlock);
	puts(str);
}

/*
 *	Funcation Name: sma_realloc
 *	Input type:		void*, int
 * 	Output type:	void*
 * 	Description:	Reallocates memory pointed to by the input pointer by resizing the
 * 					memory block according to the input size.
 */
void *sma_realloc(void *ptr, int size)
{
	// TODO: 	Should be similar to sma_malloc, except you need to check if the pointer address
	//			had been previously allocated.
	// Hint:	Check if you need to expand or contract the memory. If new size is smaller, then
	//			chop off the current allocated memory and add to the free list. If new size is bigger
	//			then check if there is sufficient adjacent free space to expand, otherwise find a new block
	//			like sma_malloc

	bool oldTag = getTag(ptr);
	// memory was previously allocated
	if (oldTag)
	{
		int oldSize = getBlockSize(ptr);
		// new size is smaller
		if (size <= oldSize)
		{
			int excessSize = oldSize - size;
			allocate_block(ptr, size, excessSize, 0);
			return ptr;
		}
		// new size is larger
		else
		{
			void *oldBlock = ptr;

			// free the memory block
			sma_free(ptr);
			void *newBlock = sma_malloc(size);
			
			// copy data from oldBlock to newBlock
			memcpy(newBlock,oldBlock,oldSize);
			return newBlock;
		}
	}
	// memory not allocated yet
	else
	{
		puts("Error: Attempting to reallocate unallocated space!");
	}
}

/*
 * =====================================================================================
 *	Private Functions for SMA
 * =====================================================================================
 */

//	TODO: Implement all your helper functions here (You need to declare them in helper.h)

/*
 *	Funcation Name: allocate_pBrk
 *	Input type:		int
 * 	Output type:	void*
 * 	Description:	Allocates memory by increasing the Program Break. Returns pointer to allocated memory block
 */
void *allocate_pBrk(int size)
{
	// init start of heap
	if (isFirstBreak)
	{
		puts("FIRST BREAK");
		heapStart = sbrk(0);
		isFirstBreak = 0;
	}

	int excessSize = MAX_TOP_FREE;
	void *newBlock;

	// free blocks list empty
	if (freeListHead == NULL)
	{
		newBlock = sbrk(2 * FREE_BLOCK_HEADER_SIZE + MAX_TOP_FREE + size) + FREE_BLOCK_HEADER_SIZE;
		setTag(newBlock, ALLOCATED);
		heapBreak = sbrk(0);
	}
	// free blocks list not empty
	else
	{

		void *lastFreeBlock = freeListTail;
		int lastFreeBlockSize = getBlockSize(lastFreeBlock);
		int breakIncrement;
		// last block on the heap is a free block
		if (isLastBlock(lastFreeBlock))
		{
			breakIncrement = size + FREE_BLOCK_HEADER_SIZE + MAX_TOP_FREE - lastFreeBlockSize;
			sbrk(breakIncrement);
			heapBreak = sbrk(0);
			newBlock = lastFreeBlock;
			setTag(newBlock, ALLOCATED);
		}
		// last block on the heap is not a free block
		else
		{
			breakIncrement = 2 * FREE_BLOCK_HEADER_SIZE + MAX_TOP_FREE + size;
			newBlock = sbrk(breakIncrement) + FREE_BLOCK_HEADER_SIZE;
			setTag(newBlock, ALLOCATED);
		}
	}

	excessSize = FREE_BLOCK_HEADER_SIZE + MAX_TOP_FREE;

	//	TODO: 	Allocate memory by incrementing the Program Break by calling sbrk() or brk()
	//	Hint:	Getting an exact "size" of memory might not be the best idea. Why?
	//			Also, if you are getting a larger memory, you need to put the excess in the free list

	//	Allocates the Memory Block
	allocate_block(newBlock, size, excessSize, 0);

	return newBlock;
}

/*
 *	Funcation Name: allocate_freeList
 *	Input type:		int
 * 	Output type:	void*
 * 	Description:	Allocates memory from the free memory list
 */
void *allocate_freeList(int size)
{
	void *pMemory = NULL;

	if (currentPolicy == WORST)
	{
		// Allocates memory using Worst Fit Policy
		pMemory = allocate_worst_fit(size);
	}
	else if (currentPolicy == NEXT)
	{
		// Allocates memory using Next Fit Policy
		pMemory = allocate_next_fit(size);
	}
	else
	{
		pMemory = NULL;
	}
	return pMemory;
}

/*
 *	Funcation Name: allocate_worst_fit
 *	Input type:		int
 * 	Output type:	void*
 * 	Description:	Allocates memory using Worst Fit from the free memory list
 */
void *allocate_worst_fit(int size)
{
	void *worstBlock = NULL;
	int excessSize;
	int blockFound = 0;

	//	DONE: 	Allocate memory by using Worst Fit Policy
	//	Hint:	Start off with the freeListHead and iterate through the entire list to get the largest block
	worstBlock = freeListHead;
	// block size we are looking for
	int largestBlockSize = get_largest_freeBlock();

	while (true)
	{
		int blockSize = getBlockSize(worstBlock);
		// block found
		if (blockSize == largestBlockSize)
		{
			blockFound = 1;
			excessSize = blockSize - size;
			break;
		}
		// no block found
		if (worstBlock == freeListTail)
			break;
		worstBlock = getNext(worstBlock);
	}
	//	Checks if appropriate found is found.
	if (blockFound)
	{
		//	Allocates the Memory Block
		allocate_block(worstBlock, size, excessSize, 1);
	}
	else
	{
		//	Assigns invalid valid
		worstBlock = (void *)-2;
	}

	return worstBlock;
}

/*
 *	Funcation Name: allocate_next_fit
 *	Input type:		int
 * 	Output type:	void*
 * 	Description:	Allocates memory using Next Fit from the free memory list
 */
void *allocate_next_fit(int size)
{
	void *nextBlock;
	int excessSize;
	int blockFound = 0;

	//	DONE: 	Allocate memory by using Next Fit Policy
	//	Hint:	Start off with the freeListHead, and keep track of the current position in the free memory list.
	//			The next time you allocate, it should start from the current position.
	if (nextFitPointer == NULL || getTag(nextFitPointer) == 1)
	{
		puts("reset to head");
		nextBlock = freeListHead;
	}
	else
		nextBlock = nextFitPointer;

	void *start = nextBlock;
	void *returnBlock;

	while (true)
	{
		int blockSize = getBlockSize(nextBlock);
		// block found
		if (blockSize >= size)
		{
			blockFound = 1;
			excessSize = blockSize - size;
			nextFitPointer = getNext(nextBlock);
			if (nextFitPointer == NULL)
				nextFitPointer = freeListHead;
			break;
		}
		// no block found
		if (nextBlock == freeListHead && nextBlock == freeListTail)
			break;
		else if (nextBlock == freeListTail)
			nextBlock = freeListHead;
		else
			nextBlock = getNext(nextBlock);

		if (nextBlock == start)
			break;
	}
	//	Checks if appropriate found is found.
	if (blockFound)
	{
		//	Allocates the Memory Block
		allocate_block(nextBlock, size, excessSize, 1);
	}
	else
	{
		//	Assigns invalid valid
		nextBlock = (void *)-2;
	}

	return nextBlock;
}

/*
 *	Funcation Name: allocate_block
 *	Input type:		void*, int, int, int
 * 	Output type:	void
 * 	Description:	Performs routine operations for allocating a memory block
 * 
 */
void allocate_block(void *newBlock, int size, int excessSize, int fromFreeList)
{
	// puts("ALLOCATE BLOCK\n");

	void *excessFreeBlock;
	int addFreeBlock;

	// 	Checks if excess free size is big enough to be added to the free memory list
	//	Helps to reduce external fragmentation
	addFreeBlock = excessSize > FREE_BLOCK_HEADER_SIZE;

	//	If excess free size is big enough
	if (addFreeBlock)
	{
		setBlockSize(newBlock, size);
		// puts("ALLOCATE + ADD FREE BLOCK\n");

		//	DONE: Create a free block using the excess memory size, then assign it to the newBlock
		excessFreeBlock = newBlock + size + FREE_BLOCK_HEADER_SIZE;
		setBlockSize(excessFreeBlock, excessSize - FREE_BLOCK_HEADER_SIZE);

		//	Checks if the new block was allocated from the free memory list
		if (fromFreeList)
		{
			//	Removes new block and adds excess free block to the free list
			replace_block_freeList(newBlock, excessFreeBlock);
		}
		else
		{
			setPrevious(newBlock, NULL);
			setNext(newBlock, NULL);
			void *prev = getPrevious(newBlock);
			void *next = getNext(newBlock);
			if (prev != NULL)
				setNext(prev, next);
			if (next != NULL)
				setPrevious(next, prev);

			//	Adds excess free block to the free list
			add_block_freeList(excessFreeBlock);
		}
	}
	//	Otherwise add the excess memory to the new block
	else
	{
		//	DONE: Add excessSize to size and assign it to the newBlock
		size += excessSize;
		setBlockSize(newBlock, size);

		//	Checks if the new block was allocated from the free memory list
		if (fromFreeList)
		{
			//	Removes that block from the free list
			remove_block_freeList(newBlock);
		}
	}
}

/*
 *	Funcation Name: replace_block_freeList
 *	Input type:		void*, void*
 * 	Output type:	void
 * 	Description:	Replaces old block with the new block in the free list
 */
void replace_block_freeList(void *oldBlock, void *newBlock)
{

	//	DONE: Replace the old block with the new block

	void *previousBlock = getPrevious(oldBlock);
	void *nextBlock = getNext(oldBlock);

	// Update previous and next pointers
	if (previousBlock != NULL)
	{
		setNext(previousBlock, newBlock);
		setPrevious(newBlock, previousBlock);
	}
	else
	{
		//puts("CHECK");
		setPrevious(newBlock, NULL);
		freeListHead = newBlock;
	}

	if (nextBlock != NULL)
	{
		setPrevious(nextBlock, newBlock);
		setNext(newBlock, nextBlock);
	}
	else
	{
		setNext(newBlock, NULL);
		freeListTail = newBlock;
	}

	setPrevious(oldBlock, NULL);
	setNext(oldBlock, NULL);
	setTag(oldBlock, ALLOCATED);
	setTag(newBlock, FREE);

	//	Updates SMA info
	totalAllocatedSize += getBlockSize(oldBlock);
	totalFreeSize -= (getBlockSize(oldBlock) + FREE_BLOCK_HEADER_SIZE);
}

/*
 *	Funcation Name: add_block_freeList
 *	Input type:		void*
 * 	Output type:	void
 * 	Description:	Adds a memory block from the the free memory list
 */
void add_block_freeList(void *block)
{
	int blockSize = getBlockSize(block);
	setTag(block, FREE);

	//	DONE: 	Add the block to the free list
	//	Hint: 	You could add the free block at the end of the list, but need to check if there
	//			exits a list. You need to add the TAG to the list.
	//			Also, you would need to check if merging with the "adjacent" blocks is possible or not.
	//			Merging would be tideous. Check adjacent blocks, then also check if the merged
	//			block is at the top and is bigger than the largest free block allowed (128kB).

	// list does not exist
	if (freeListHead == NULL)
	{
		freeListHead = block;
		setNext(block, NULL);
		setPrevious(block, NULL);
		freeListTail = block;
	}
	// list does exist
	else
	{
		if (isLastBlock(block))
		{
			if (adjacentBlocks(freeListTail, block))
			{
				// Merge left
				setPrevious(block, freeListTail);
				setNext(block, NULL);
				mergeBlocks(freeListTail, block);
			}
		}
		else if (isFirstBlock(block))
		{
			if (adjacentBlocks(block, freeListHead))
			{
				// Merge right
				setNext(block, freeListHead);
				setPrevious(block, NULL);
				mergeBlocks(block, freeListHead);
			}
		}
		else
		{
			void *prev = findPreviousFreeBlock(block);
			void *next = findNextFreeBlock(block);

			setNext(block, next);
			setPrevious(block, prev);

			if (adjacentBlocks(prev, block))
			{

				// Merge left
				block = mergeBlocks(prev, block);
			}
			if (adjacentBlocks(block, next))
			{
				// Merge right
				block = mergeBlocks(block, next);
			}
			// add block to free block list
			prev = getPrevious(block);
			next = getNext(block);
			if (prev != NULL)
			{
				setNext(prev, block);
				setPrevious(block, prev);
			}
			else
			{
				setPrevious(block, NULL);
				freeListHead = block;
			}

			if (next != NULL)
			{
				setNext(block, next);
				setPrevious(next, block);
			}
			else
			{
				setNext(block, NULL);
				freeListTail = block;
			}
		}
	}
	setTag(block, FREE);
	//	Updates SMA info
	totalAllocatedSize -= blockSize;
	totalFreeSize += blockSize;
}

/*
 *	Funcation Name: remove_block_freeList
 *	Input type:		void*
 * 	Output type:	void
 * 	Description:	Removes a memory block from the the free memory list
 */
void remove_block_freeList(void *block)
{

	//	DONE: 	Remove the block from the free list
	//	Hint: 	You need to update the pointers in the free blocks before and after this block.
	//			You also need to remove any TAG in the free block.

	// puts("REMOVE FREE BLOCK\n");
	void *nextBlock = getNext(block);
	void *previousBlock = getPrevious(block);
	if (freeListHead == block && freeListTail == block)
	{
		// no more free list
		freeListHead = NULL;
		freeListTail = NULL;
		nextFitPointer = NULL;
	}
	else if (freeListHead == block)
	{
		freeListHead = nextBlock;
		if (nextBlock != NULL)
			setPrevious(nextBlock, NULL);
	}
	else if (freeListTail == block)
	{
		freeListTail = previousBlock;
		if (previousBlock != NULL)
			setNext(previousBlock, NULL);
	}

	else
	{
		setNext(previousBlock, nextBlock);
		setPrevious(nextBlock, previousBlock);
	}

	setPrevious(block, NULL);
	setNext(block, NULL);
	setTag(block, ALLOCATED);

	//	Updates SMA info
	totalAllocatedSize += getBlockSize(block);
	totalFreeSize -= getBlockSize(block);
}

/*
 *	Funcation Name: getBlockSize
 *	Input type:		void*
 * 	Output type:	int
 * 	Description:	Extracts the Block Size
 */
int getBlockSize(void *block)
{
	void *ptr = block;
	ptr = ptr - 2 * BLOCK_POINTER_SIZE - INT_SIZE;

	//	Returns the deferenced size
	return *(int *)ptr;
}

void setBlockSize(void *block, int size)
{

	void *ptr = block;
	ptr = ptr - 2 * BLOCK_POINTER_SIZE - INT_SIZE;
	*(int *)ptr = size;
}

bool getTag(void *block)
{
	void *ptr = block;
	ptr = ptr - 2 * BLOCK_POINTER_SIZE - INT_SIZE - BOOL_SIZE;

	//	Returns the deferenced size
	return *(bool *)ptr;
}

void setTag(void *block, bool tag)
{
	void *ptr = block;
	ptr = ptr - 2 * BLOCK_POINTER_SIZE - INT_SIZE - BOOL_SIZE;
	*(bool *)ptr = tag;
}

/* Get previous pointer of a block
 * @param block: address of block
 */
void *getPrevious(void *block)
{
	void **ptr = block - 2 * BLOCK_POINTER_SIZE;
	return *ptr;
}
void setPrevious(void *block, void *previous)
{
	void **ptr = block - 2 * BLOCK_POINTER_SIZE;
	*ptr = previous;
}

/* Get previous pointer of a block
 * @param block: address of block
 */
void *getNext(void *block)
{
	void **ptr = block - BLOCK_POINTER_SIZE;
	return *ptr;
}
void setNext(void *block, void *next)
{
	void **ptr = block - BLOCK_POINTER_SIZE;
	*ptr = next;
}

/*
 *	Funcation Name: get_largest_freeBlock
 *	Input type:		void
 * 	Output type:	int
 * 	Description:	Extracts the largest Block Size
 */
int get_largest_freeBlock()
{
	int largestBlockSize = 0;

	//	DONE: Iterate through the Free Block List to find the largest free block and return its size
	void *ptr = freeListHead;
	while (true)
	{
		int currentBlockSize = getBlockSize(ptr);
		if (currentBlockSize > largestBlockSize)
		{
			largestBlockSize = currentBlockSize;
		}
		if (ptr == freeListTail)
			break;
		ptr = getNext(ptr);
	}

	return largestBlockSize;
}

void printValue(void *toPrint, int dataType)
{
	char buffer[50];

	if (dataType == SIZE_TYPE)
	{
		sprintf(buffer, "size: %d\n", *(int *)toPrint);
	}
	else if (dataType == POINTER_TYPE)
	{
		sprintf(buffer, "pointer: %p\n", *(void **)toPrint);
	}
	else if (dataType == ADDRESS_TYPE)
	{
		sprintf(buffer, "address: %p\n", toPrint);
	}
	else
	{
		sprintf(buffer, "tag: %d\n", *(bool *)toPrint);
	}
	puts(buffer);
}

bool isLastBlock(void *block)
{
	int blockSize = getBlockSize(block);
	void *endOfBlock = block + blockSize;

	if (endOfBlock == heapBreak)
		return true;
	else
		return false;
}

bool isFirstBlock(void *block)
{
	void *startOfBlock = block - FREE_BLOCK_HEADER_SIZE;
	if (startOfBlock == heapStart)
		return true;
	else
		return false;
}

bool adjacentBlocks(void *leftBlock, void *rightBlock)
{
	if (leftBlock == NULL)
	{
		return false;
	}
	if (rightBlock == NULL)
	{
		return false;
	}
	int leftBlockSize = getBlockSize(leftBlock);
	if ((leftBlock + leftBlockSize + FREE_BLOCK_HEADER_SIZE) == rightBlock)
		return true;
	else
		return false;
}

void *findNextFreeBlock(void *start)
{
	if (start == NULL)
		return NULL;
	void *block = freeListHead;
	bool blockFound = false;
	while (true)
	{
		if (block >= start)
		{
			blockFound = true;
			break;
		}
		if (block == freeListTail)
			break;

		block = getNext(block);
	}
	if (blockFound)
	{
		return block;
	}
	else
	{
		return NULL;
	}
}

void *findPreviousFreeBlock(void *start)
{
	if (start == NULL)
		return NULL;
	void *block = freeListTail;

	bool blockFound = false;
	while (true)
	{
		if (block <= start)
		{
			blockFound = true;
			break;
		}
		if (block == freeListHead)
		{
			break;
		}

		block = getPrevious(block);
	}
	if (blockFound)
	{
		return block;
	}
	else
	{
		return NULL;
	}
}

void *
mergeBlocks(void *leftBlock, void *rightBlock)
{

	if (leftBlock == NULL)
	{
		return rightBlock;
	}
	if (rightBlock == NULL)
	{
		return leftBlock;
	}
	void *prev = getPrevious(leftBlock);
	void *next = getNext(rightBlock);

	int leftBlockSize = getBlockSize(leftBlock);
	int rightBlockSize = getBlockSize(rightBlock);
	if (next != NULL)
	{
		setNext(leftBlock, next);
		setPrevious(next, leftBlock);
	}
	else
	{
		setNext(leftBlock, NULL);
		freeListTail = leftBlock;
	}

	if (prev != NULL)
	{
		setNext(prev, leftBlock);
		setPrevious(leftBlock, prev);
	}
	else
	{
		setPrevious(leftBlock, NULL);
		freeListHead = leftBlock;
	}
	setPrevious(rightBlock, NULL);
	setNext(rightBlock, NULL);
	setBlockSize(leftBlock, (leftBlockSize + FREE_BLOCK_HEADER_SIZE + rightBlockSize));

	return leftBlock;
}

void printBlockInfo(void *block)
{
	puts("BLOCK INFO:");

	int size = getBlockSize(block);
	bool tag = getTag(block);
	void *prev = getPrevious(block);
	void *next = getNext(block);
	puts("size");
	printValue(&size, SIZE_TYPE);
	puts("address");
	printValue(block, ADDRESS_TYPE);
	puts("prev");
	printValue(prev, ADDRESS_TYPE);
	puts("next");
	printValue(next, ADDRESS_TYPE);
	puts("tag");
	printValue(&tag, TAG_TYPE);
}

void updateNextFitPointer()
{
	if (nextFitPointer == NULL)
	{
		nextFitPointer = freeListHead;
	}
	else if (getTag(nextFitPointer) == 1)
	{
		puts("update nextFitPointer");
		nextFitPointer = freeListHead;
	}
}