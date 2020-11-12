/*
 * =====================================================================================
 *
 *  Filename:  		sma.h
 *
 *  Description:	Header file for SMA.
 *
 *  Version:  		1.0
 *  Created:  		3/11/2020 9:30:00 AM
 *  Revised:  		-
 *  Compiler:  		gcc
 *
 *  Author:  		Mohammad Mushfiqur Rahman
 *      
 *  Instructions:   Please address all the "TODO"s in the code below and modify them
 *                  accordingly. Refer to the Assignment Handout for further info.
 * =====================================================================================
 */

/* Includes */
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

//  Policies definition
#define WORST_FIT	1
#define NEXT_FIT	2

extern char *sma_malloc_error;

//  Public Functions declaration
void *sma_malloc(int size);
void sma_free(void* ptr);
void sma_mallopt(int policy);
void sma_mallinfo();
void *sma_realloc(void *ptr, int size);

//  Private Functions declaration
static void* allocate_pBrk(int size);
static void* allocate_freeList(int size);
static void* allocate_worst_fit(int size);
static void* allocate_next_fit(int size);
static void allocate_block(void* newBlock, int size, int excessSize, int fromFreeList);
static void replace_block_freeList(void* oldBlock, void* newBlock);
static void add_block_freeList(void* block);
static void remove_block_freeList(void* block);
static int get_largest_freeBlock();

//  Getters Setters
static void *getNext(void *block);
static void setNext(void *block, void *next);
static void *getPrevious(void *block);
static void setPrevious(void *block, void *previous);
static int getBlockSize(void *block);
static void setBlockSize(void *block, int size);
static bool getTag(void *block);
static void setTag(void *block, bool tag);

// Helper Functions
static bool isLastBlock(void *block);
static bool isFirstBlock(void *block);
static bool adjacentBlocks(void *leftBlock, void *rightBlock);
static void *findNextFreeBlock(void *start);
static void *findPreviousFreeBlock(void *start);
static void *mergeBlocks(void *leftBlock, void *rightBlock);

// Debugging tools
static void printValue(void *toPrint, int dataType);
static void printBlockInfo(void *block);
