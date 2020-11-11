#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

int main()
{

    void printValue(void *toPrint, int dataType)
    {
        char buffer[50];

        if (dataType == 0)
        {
            sprintf(buffer, "size: %d\n", *(int *)toPrint);
        }
        else if (dataType == 1)
        {
            sprintf(buffer, "pointer: %p\n", *(void **)toPrint);
        }
        else if (dataType == 2)
        {
            sprintf(buffer, "address: %p\n", toPrint);
        }
        else
        {
            sprintf(buffer, "tag: %d\n", *(bool *)toPrint);
        }
        puts(buffer);
    }

    void *previousBlock = malloc(128);
    void *nextBlock = malloc(128);
    void *block = malloc(128);
    void *ptr = block;
    bool tag = 1;
    *(bool *)ptr = tag;

    ptr = block + 1;
    int size = 8;
    *(int *)ptr = size;

    void **ptr2 = block + 1 + 4;
    *ptr2 = previousBlock;

    ptr2 = block + 1 + 4 + 8;
    *ptr2 = nextBlock;

    printf("block address: %p\n", block );
    printf("previous block address: %p\n", previousBlock);
    printf("next block address: %p\n", nextBlock);

    printf("tag: %d\n", *(bool *)block);
    printf("size: %d\n", *(int *)(block + 1));
    printf("prev: %p\n", *(void **)(block + 1 + 4));
    printf("next: %p\n", *(void **)(block + 1 + 4 + 8));

    printValue(block, 2);
}