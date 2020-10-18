#include "sut.h"
#include "queue.h"

/* TODO: 
    
    1. Implement Task struct to store: 
            - int thread id
            -  ucontext_t thread context
            -  char* thread stack pointer
            -  void * thread function

    2. Implement ready queue of Task struct using queue wrapper
*/

typedef void (*sut_task_f)();

struct queue task_ready_queue;

typedef struct suts {
    int id;
    char *stack;
    sut_task_f fct;
    ucontext_t context;

} sut;

void sut_init()
{
    // create ready task queue
    task_ready_queue = queue_create();
    // initialize queue
    queue_init(&task_ready_queue);
}

/**
 * Create a Task struct and add it to the END of the task ready queue.
 * @param fn: pointer to a C function to execute when task gets to execute
 * @return true if task created successfuly false if not
 */
bool sut_create(sut_task_f fn)
{
}

/**
 * pauses the execution of a user thread until it is selected again
 * by the scheduler. Task is put back at the END of the task ready queue. 
 * Add a Task struct to the ready queue.
*/
void sut_yield()
{
}

/**
 * Terminates execution of current task.
 * It does not put it back on the task ready queue.
 */
void sut_exit()
{
}

void sut_shutdown(){}


// IO PART

void sut_open(char *dest, int port)
{
}

void sut_write(char *buf, int size)
{
}

void sut_close()
{
}

char *sut_read()
{
}
