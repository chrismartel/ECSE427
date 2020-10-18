#ifndef COMP310_A2_Q
#define COMP310_A2_Q

#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/queue.h>

struct queue_entry {
    void *data;
    STAILQ_ENTRY(queue_entry) entries;
};

STAILQ_HEAD(queue, queue_entry);

/** Creates a queue 
 * @return: an empty queue struct 
*/
struct queue queue_create() {
    struct queue q = STAILQ_HEAD_INITIALIZER(q);
    return q;
}

/** Initializes a queue to make sure all fields are consistent
*/
void queue_init(struct queue *q) {
    STAILQ_INIT(q);
}

void queue_error() {
    fprintf(stderr, "Fatal error in queue operations\n");
    exit(1);
}

/**
 * Helper function for crating a new node
 * @param data: pointer to the data to store in the node
*/
struct queue_entry *queue_new_node(void *data) {
    struct queue_entry *entry = (struct queue_entry*) malloc(sizeof(struct queue_entry));
    if(!entry) {
        queue_error();
    }
    // set data of entry
    entry->data = data;
    return entry;
}

/**
 * Insert queue entry at head of a queue
 * @param q: pointer to queue struct in which we want to insert an entry
 * @param e: pointer to queue entry struct that we want to insert in the queue
*/
void queue_insert_head(struct queue *q, struct queue_entry *e) {
    STAILQ_INSERT_HEAD(q, e, entries);
}

/**
 * Insert queue entry at end of a queue
 * @param q: pointer to queue struct in which we want to insert an entry
 * @param e: pointer to queue entry struct that we want to insert in the queue
*/
void queue_insert_tail(struct queue *q, struct queue_entry *e) {
    STAILQ_INSERT_TAIL(q, e, entries);
}

/**
 * Peek queue entry at front of a queue
 * @param q: pointer to queue struct in which we want to insert an entry
 * @return: pointer to queue entry at front of queue q
*/
struct queue_entry *queue_peek_front(struct queue *q) {
    return STAILQ_FIRST(q);
}

/**
 * Peek queue entry at end of a queue
 * @param q: pointer to queue struct in which we want to insert an entry
 * @return: pointer to queue entry at end of queue q
*/
struct queue_entry *queue_pop_head(struct queue *q) {
    struct queue_entry *elem = queue_peek_front(q);
    if(elem) {
        STAILQ_REMOVE_HEAD(q, entries);
    }
    return elem;
}

#endif
