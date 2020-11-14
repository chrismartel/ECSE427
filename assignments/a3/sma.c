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
/** Header Format 
 * | - | - - - - | - - - - - - - - | - - - - - - - - |
 *  tag    size         previous           next
 *   1       4             8                8  
 */

/* size constants*/
#define BLOCK_POINTER_SIZE 8
#define INT_SIZE 4
#define BOOL_SIZE 1

/* tag values */
#define FREE 0
#define ALLOCATED 1

/* debugging tools constants */
#define SIZE_TYPE 0
#define ADDRESS_TYPE 1
#define POINTER_TYPE 2
#define TAG_TYPE 3

/* allocation policy */
typedef enum
{
	WORST,
	NEXT
} Policy;

/* global variables */

// error string
char *sma_malloc_error;

// pointer to head of free block list
void *freeListHead = NULL;

// pointer to tail of free block list
void *freeListTail = NULL;

// total allocated size
unsigned long totalAllocatedSize = 0;

// total free size
unsigned long totalFreeSize = 0;

//  allocation policy, NEXT fit by deafult
Policy currentPolicy = NEXT;

// pointer to end of heap
void *heapBreak;

// pointer to start of heap
void *heapStart;

// flag for first heap break
bool isFirstBreak = 1;

// pointer used for the nextFit allocation algorithm
void *lastAllocatedPointer;

/*
 * =====================================================================================
 *	Public Functions for SMA
 * =====================================================================================
 */

/*
 *	Function Name: sma_malloc
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
	else
	{
		// Update SMA
		totalAllocatedSize += size;
		setTag(pMemory, ALLOCATED);

		lastAllocatedPointer = pMemory;

	}
	return pMemory;
}

/*
 *	Function Name: sma_free
 *	Input type:		void*
 * 	Output type:	void
 * 	Description:	Deallocates the memory block pointed by the input pointer
 */
void sma_free(void *ptr)
{
	totalAllocatedSize -= getBlockSize(ptr);

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
		// update the program break
		updateBreak();
	}
	setTag(ptr, FREE);


	totalFreeSize += getBlockSize(ptr);
}

/*
 *	Function Name: sma_mallopt
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
 *	Function Name: sma_mallinfo
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
 *	Function Name: sma_realloc
 *	Input type:		void*, int
 * 	Output type:	void*
 * 	Description:	Reallocates memory pointed to by the input pointer by resizing the
 * 					memory block according to the input size.
 */
void *sma_realloc(void *ptr, int size)
{
	// get tag of block
	bool oldTag = getTag(ptr);

	// block is allocated
	if (oldTag)
	{
		int oldSize = getBlockSize(ptr);
		// new size is smaller
		if (size <= oldSize)
		{
			int excessSize = oldSize - size;
			allocate_block(ptr, size, excessSize, 0);
			totalAllocatedSize -= (oldSize - size);
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
			memcpy(newBlock, oldBlock, oldSize);
			return newBlock;
		}
	}

	// block is not allocated
	else
	{
		sma_malloc_error = "Error: Attempting to reallocate unallocated space!";
	}
}

/*
 * =====================================================================================
 *	Private Functions for SMA
 * =====================================================================================
 */

/*
 *	Function Name: isLastBlock
 *	Input type:		void*
 * 	Output type:	bool
 * 	Description:	checks if block is the last block on the heap
 */
bool isLastBlock(void *block)
{
	if (block == NULL)
		return false;
	int blockSize = getBlockSize(block);
	void *endOfBlock = block + blockSize;

	if (endOfBlock == heapBreak)
		return true;
	else
		return false;
}

/*
 *	Function Name: isFirstBlock
 *	Input type:		void*
 * 	Output type:	bool
 * 	Description:	checks if block is the first block on the heap
 */
bool isFirstBlock(void *block)
{
	if (block == NULL)
		return false;
	void *startOfBlock = block - FREE_BLOCK_HEADER_SIZE;
	if (startOfBlock == heapStart)
		return true;
	else
		return false;
}

/*
 *	Function Name: adjacentBlocks
 *	Input type:		void*,void*
 * 	Output type:	bool
 * 	Description:	checks if lefBlock and rightBlock are adjacent
 */
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
	if (leftBlock == rightBlock)
	{
		return false;
	}
	int leftBlockSize = getBlockSize(leftBlock);
	if ((leftBlock + leftBlockSize + FREE_BLOCK_HEADER_SIZE) == rightBlock)
		return true;
	else
		return false;
}

/*
 *	Function Name: findNextFreeBlock
 *	Input type:		void*
 * 	Output type:	void*
 * 	Description:	finds the next free block from start pointer
 */
void *findNextFreeBlock(void *start)
{
	if (start == NULL)
		return NULL;

	void *block = freeListHead;

	while (true)
	{
		if ((block > start) && (!getTag(block)))
		{

			return block;
		}
		if (block == freeListTail)
			return freeListHead;

		block = getNext(block);
	}
}

/*
 *	Function Name: findPreviousFreeBlock
 *	Input type:		void*
 * 	Output type:	void*
 * 	Description:	finds the previous free block from start pointer
 */
void *findPreviousFreeBlock(void *start)
{
	if (start == NULL)
		return NULL;
	void *block = freeListTail;

	bool blockFound = false;
	while (true)
	{
		if (block <= start && !getTag(block))
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
/*
 *	Function Name: mergeBlocks
 *	Input type:		void*, void*
 * 	Output type:	void*
 * 	Description:	merges the leftblock and the rightblock and returns the merged block pointer
 */
void *mergeBlocks(void *leftBlock, void *rightBlock)
{

	if (leftBlock == NULL)
	{
		return rightBlock;
	}
	if (rightBlock == NULL)
	{
		return leftBlock;
	}
	if (leftBlock == rightBlock)
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

/*
 *	Function Name: updateBreak
 *	Input type:		void
 * 	Output type:	void
 * 	Description:	Updates the break of the program in function 
 * 					of the last free block.
 */
void updateBreak()
{
	if (freeListTail == NULL)
		return;
	else
	{
		// check if last free block is last block on heap
		if (isLastBlock(freeListTail))
		{
			int size = getBlockSize(freeListTail);
			// too much free space
			if (size > MAX_TOP_FREE)
			{

				setBlockSize(freeListTail, MAX_TOP_FREE);

				// reduce program break
				brk(freeListTail + MAX_TOP_FREE);
				heapBreak = sbrk(0);
			}
		}
	}
}

/*
 *	Function Name: allocate_pBrk
 *	Input type:		int
 * 	Output type:	void*
 * 	Description:	Allocates memory by increasing the Program Break. Returns pointer to allocated memory block
 */
void *allocate_pBrk(int size)
{
	// init heap
	if (isFirstBreak)
	{
		heapStart = sbrk(0);
		isFirstBreak = 0;
	}

	int excessSize = MAX_TOP_FREE;
	void *newBlock = NULL;
	int breakIncrement;

	// free block list is empty
	if (freeListHead == NULL)
	{
		breakIncrement = (2 * FREE_BLOCK_HEADER_SIZE) + MAX_TOP_FREE + size;
		newBlock = sbrk(breakIncrement) + FREE_BLOCK_HEADER_SIZE;
		heapBreak = sbrk(0);
	}
	// free block list is not empty
	else
	{
		// get info of last free block
		void *lastFreeBlock = freeListTail;
		int lastFreeBlockSize = getBlockSize(lastFreeBlock);

		// last block on the heap is a free block
		if (isLastBlock(lastFreeBlock))
		{
			breakIncrement = size + FREE_BLOCK_HEADER_SIZE + MAX_TOP_FREE - lastFreeBlockSize;
			sbrk(breakIncrement);
			heapBreak = sbrk(0);
			newBlock = lastFreeBlock;
		}
		// last block on the heap is not a free block
		else
		{
			breakIncrement = 2 * FREE_BLOCK_HEADER_SIZE + MAX_TOP_FREE + size;
			newBlock = sbrk(breakIncrement) + FREE_BLOCK_HEADER_SIZE;
		}
	}

	excessSize = FREE_BLOCK_HEADER_SIZE + MAX_TOP_FREE;

	// Update Free Space Info
	totalFreeSize += breakIncrement;

	//	Allocates the Memory Block
	allocate_block(newBlock, size, excessSize, 0);

	return newBlock;
}

/*
 *	Function Name: allocate_freeList
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
 *	Function Name: allocate_worst_fit
 *	Input type:		int
 * 	Output type:	void*
 * 	Description:	Allocates memory using Worst Fit from the free memory list
 */
void *allocate_worst_fit(int size)
{
	void *worstBlock = NULL;
	int excessSize;
	int blockFound = 0;

	// Start at head
	worstBlock = freeListHead;
	// Largest free block size
	int largestBlockSize = get_largest_freeBlock();

	if (largestBlockSize < size)
		return (void *)-2;

	while (true)
	{
		int blockSize = getBlockSize(worstBlock);
		// Block found
		if (blockSize == largestBlockSize)
		{
			blockFound = 1;
			excessSize = blockSize - size;
			break;
		}
		// Tail is reached --> no block found
		if (worstBlock == freeListTail)
			break;

		// Check next block in list
		worstBlock = getNext(worstBlock);
	}
	//	Checks if block was found.
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
 *	Function Name: allocate_next_fit
 *	Input type:		int
 * 	Output type:	void*
 * 	Description:	Allocates memory using Next Fit from the free memory list
 */
void *allocate_next_fit(int size)
{
	void *nextBlock = NULL;
	int excessSize;
	int blockFound = 0;

	// check if next fit pointer is NULL or was allocated
	if (lastAllocatedPointer == NULL)
	{
		// start from the head
		nextBlock = freeListHead;
	}
	else
	{
		// start from next fit pointer
		nextBlock = findNextFreeBlock(lastAllocatedPointer);
	}

	// keep track of starting point
	void *start = nextBlock;

	while (true)
	{
		int blockSize = getBlockSize(nextBlock);
		// block found
		if (blockSize >= size)
		{
			blockFound = 1;
			excessSize = blockSize - size;
			break;
		}
		// Only one block --> end of loop
		if (nextBlock == freeListHead && nextBlock == freeListTail)
			break;

		// Updates pointer
		else if (nextBlock == freeListTail){
						nextBlock = freeListHead;

		}
		else{
			nextBlock = getNext(nextBlock);
		}

		// Come back to start pointer --> end of loop
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
 *	Function Name: allocate_block
 *	Input type:		void*, int, int, int
 * 	Output type:	void
 * 	Description:	Performs routine operations for allocating a memory block
 * 
 */
void allocate_block(void *newBlock, int size, int excessSize, int fromFreeList)
{
	void *excessFreeBlock;
	int addFreeBlock;

	// set block status to allocated
	//setTag(newBlock, ALLOCATED);

	// 	Checks if excess free size is big enough to be added to the free memory list
	//	Helps to reduce external fragmentation
	addFreeBlock = excessSize > FREE_BLOCK_HEADER_SIZE;

	//	If excess free size is big enough
	if (addFreeBlock)
	{
		setBlockSize(newBlock, size);

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
			// Updates new allocated block parameters

			void *prev = getPrevious(newBlock);
			if (prev != NULL)
			{
				setNext(prev, excessFreeBlock);
				setPrevious(excessFreeBlock, prev);
			}
			freeListTail = excessFreeBlock;
			setPrevious(newBlock, NULL);
			setNext(newBlock, NULL);

			//	Adds excess free block to the free list
			add_block_freeList(excessFreeBlock);
		}
	}
	//	Otherwise add the excess memory to the new block
	else
	{
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
 *	Function Name: replace_block_freeList
 *	Input type:		void*, void*
 * 	Output type:	void
 * 	Description:	Replaces old block with the new block in the free list
 */
void replace_block_freeList(void *oldBlock, void *newBlock)
{
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
	//setTag(oldBlock, ALLOCATED);
	setTag(newBlock, FREE);

	totalFreeSize -= (getBlockSize(oldBlock) + FREE_BLOCK_HEADER_SIZE);
}

/*
 *	Function Name: add_block_freeList
 *	Input type:		void*
 * 	Output type:	void
 * 	Description:	Adds a memory block from the the free memory list
 */
void add_block_freeList(void *block)
{
	int blockSize = getBlockSize(block);
	setTag(block, FREE);

	// list does not exist
	if (freeListHead == NULL)
	{
		// first block of the list
		freeListHead = block;
		setNext(block, NULL);
		setPrevious(block, NULL);
		freeListTail = block;
	}
	// list does exist
	else
	{
		// Checks if last block on the heap
		if (isLastBlock(block))
		{
			// Checks if adjacent with tail
			// |----Tail----|----Block----|
			if (adjacentBlocks(freeListTail, block))
			{
				// Merge left
				setPrevious(block, freeListTail);
				setNext(block, NULL);
				mergeBlocks(freeListTail, block);
			}
		}
		// Checks if first block on the heap
		else if (isFirstBlock(block))
		{
			// Checks if adjacent with head
			// |----Block----|----Head----|
			if (adjacentBlocks(block, freeListHead))
			{
				// Merge right
				setNext(block, freeListHead);
				setPrevious(block, NULL);
				mergeBlocks(block, freeListHead);
			}
		}
		// Block is in middle of the heap
		else
		{
			// previous free block on the heap
			void *prev = findPreviousFreeBlock(block);
			// next free block on theheap
			void *next = findNextFreeBlock(block);

			// set pointers of new block
			setNext(block, next);
			setPrevious(block, prev);

			// Checks if adjacent with previous free block
			// |----Prev----|----Block----|
			if (adjacentBlocks(prev, block))
			{
				// Merge left
				block = mergeBlocks(prev, block);
			}
			// Checks if adjacent with next free block
			// |----Block----|----Next----|
			if (adjacentBlocks(block, next))
			{
				// Merge right
				block = mergeBlocks(block, next);
			}
			// add block to free block list
			// fix pointers
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
}

/*
 *	Function Name: remove_block_freeList
 *	Input type:		void*
 * 	Output type:	void
 * 	Description:	Removes a memory block from the the free memory list
 */
void remove_block_freeList(void *block)
{
	// next free block on the list
	void *nextBlock = getNext(block);
	// previous free block ont he list
	void *previousBlock = getPrevious(block);

	// block to remove is lone block on the list
	if (freeListHead == block && freeListTail == block)
	{
		// no more free list
		freeListHead = NULL;
		freeListTail = NULL;
		lastAllocatedPointer = NULL;
	}
	// block to remove is head block
	else if (freeListHead == block)
	{
		freeListHead = nextBlock;
		if (nextBlock != NULL)
			setPrevious(nextBlock, NULL);
	}
	// block to remove is tail block
	else if (freeListTail == block)
	{
		freeListTail = previousBlock;
		if (previousBlock != NULL)
			setNext(previousBlock, NULL);
	}
	// block to remove is middle block on the list
	else
	{
		setNext(previousBlock, nextBlock);
		setPrevious(nextBlock, previousBlock);
	}

	// set parameters of removed block
	setPrevious(block, NULL);
	setNext(block, NULL);
	//setTag(block, ALLOCATED);

	//	Updates SMA info
	totalFreeSize -= getBlockSize(block);
}

/*
 *	Function Name: get_largest_freeBlock
 *	Input type:		void
 * 	Output type:	int
 * 	Description:	Extracts the largest Block Size
 */
int get_largest_freeBlock()
{
	int largestBlockSize = 0;

	// start at head of free block list
	void *ptr = freeListHead;
	while (true)
	{
		int currentBlockSize = getBlockSize(ptr);
		if (currentBlockSize > largestBlockSize)
		{
			largestBlockSize = currentBlockSize;
		}
		// end of free block list
		if (ptr == freeListTail)
			break;
		ptr = getNext(ptr);
	}

	return largestBlockSize;
}

/*
 * =====================================================================================
 *	Getters Setters for Block Parameters
 * =====================================================================================
 */

/*
 *	Function Name: getBlockSize
 *	Input type:		void*
 * 	Output type:	int
 * 	Description:	returns the blockSize of the block
 */
int getBlockSize(void *block)
{

	void *ptr = block;
	ptr = ptr - 2 * BLOCK_POINTER_SIZE - INT_SIZE;

	//	Returns the deferenced size
	return *(int *)ptr;
}

/*
 *	Function Name: setBlockSize
 *	Input type:		void*,int
 * 	Output type:	void
 * 	Description:	sets the blockSize of the block to size
 */
void setBlockSize(void *block, int size)
{

	void *ptr = block;
	ptr = ptr - 2 * BLOCK_POINTER_SIZE - INT_SIZE;
	*(int *)ptr = size;
}

/*
 *	Function Name: getTag
 *	Input type:		void*
 * 	Output type:	bool
 * 	Description:	returns the tag of the block
 */
bool getTag(void *block)
{
	void *ptr = block;
	ptr = ptr - 2 * BLOCK_POINTER_SIZE - INT_SIZE - BOOL_SIZE;
	return *(bool *)ptr;
}

/*
 *	Function Name: setTag
 *	Input type:		void*,bool
 * 	Output type:	void
 * 	Description:	sets the tag of the block to tag
 */
void setTag(void *block, bool tag)
{
	void *ptr = block;
	ptr = ptr - 2 * BLOCK_POINTER_SIZE - INT_SIZE - BOOL_SIZE;
	*(bool *)ptr = tag;
}

/*
 *	Function Name: getPrevious
 *	Input type:		void*
 * 	Output type:	void*
 * 	Description:	returns the previous block pointer of the block
 */
void *getPrevious(void *block)
{
	void **ptr = block - 2 * BLOCK_POINTER_SIZE;
	return *ptr;
}
/*
 *	Function Name: setPrevious
 *	Input type:		void*,void*
 * 	Output type:	void
 * 	Description:	sets the previous block pointer of the block to previous
 */
void setPrevious(void *block, void *previous)
{
	void **ptr = block - 2 * BLOCK_POINTER_SIZE;
	*ptr = previous;
}

/*
 *	Function Name: getNext
 *	Input type:		void*
 * 	Output type:	void*
 * 	Description:	returns the next block pointer of the block
 */
void *getNext(void *block)
{
	void **ptr = block - BLOCK_POINTER_SIZE;
	return *ptr;
}
/*
 *	Function Name: setNext
 *	Input type:		void*,void*
 * 	Output type:	void
 * 	Description:	sets the next block pointer of the block to next
 */
void setNext(void *block, void *next)
{
	void **ptr = block - BLOCK_POINTER_SIZE;
	*ptr = next;
}

/*
 * =====================================================================================
 *	Debugging Tools
 * =====================================================================================
 */

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


void printFreeList()
{

	void *ptr = freeListHead;
	void *end = freeListTail;
	char str[60];
	if (freeListHead == NULL || freeListTail == NULL)
		return;
	else
	{
		puts("FREE BLOCK LIST:");
		while (true)
		{

			sprintf(str, "Block size: %d   Block address: %p", getBlockSize(ptr), ptr);
			puts(str);
			if (ptr == end)
				break;
			ptr = getNext(ptr);
		}
		sprintf(str, "Head size: %d   Head address: %p", getBlockSize(freeListHead), freeListHead);
		puts(str);
		sprintf(str, "Tail size: %d   Tail address: %p", getBlockSize(freeListTail), freeListTail);
		puts(str);
		sprintf(str, "Is tail last block on heap?: %d", isLastBlock(freeListTail));
		puts(str);
	}
}