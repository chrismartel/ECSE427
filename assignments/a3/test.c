#include <stdio.h>
#include <stdlib.h>
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

    printf("block address: %p\n", block);
    printf("previous block address: %p\n", previousBlock);

    printf("next block address: %p\n", nextBlock);

    void **doublepointer = block - 2;
    *doublepointer = previousBlock;
    printf("double pointer: %p\n",*doublepointer);
}