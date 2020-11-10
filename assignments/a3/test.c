#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

int main()
{

    // struct block
    // {
    //     int size; // front of block
    //     struct block *previous;
    //     struct block *next;
    // };

    // struct block *a = malloc(sizeof(struct block));
    // printf("struct address: %p\n",&a);
    // printf("struct address: %p\n",&(a->size));
    // printf("struct address: %p\n",&(a->previous));
    // printf("struct address: %p\n",&(a->next));
    // printf("size of struct: %zu",sizeof(struct block *));

    void *previousBlock = malloc(128);
    void *nextBlock = malloc(128);
    void *block = malloc(128);
    void *ptr = block;
    bool tag = 1;
    *(bool *)ptr = tag;

    ptr = block +1;
    int size = 8;
    *(int *)ptr = size;

    void **ptr2 = block + 1 + 4;
    *ptr2 = previousBlock;

    ptr2 = block + 1 + 4 + 8;
    *ptr2 = nextBlock;
    

    printf("block address: %p\n", block + 1 + 4 + 8 + 8);
    printf("previous block address: %p\n", previousBlock);
    printf("next block address: %p\n", nextBlock);

    printf("tag: %d\n",*(bool*)block);
    printf("size: %d\n",*(int*)(block+1));
    printf("prev: %p\n",*(void **)(block+1+4));
    printf("next: %p\n",*(void **)(block+1+4+8));





}