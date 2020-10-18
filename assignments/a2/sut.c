#include "sut.h"
#include "queue.h"

#define SS 1024 * 16;

/* TODO: 
    
    1. Implement Task struct to store: 
            - int thread id
            -  ucontext_t thread context
            -  char* thread stack pointer
            -  void * thread function

    2. Implement ready queue of Task struct using queue wrapper
*/

typedef void (*sut_task_f)();

struct queue *task_ready_queue;
ucontext_t *main_context;
int *taskId;

typedef struct sut_task
{
    int id;
    char *stack;
    sut_task_f fct;
    ucontext_t context;

} sut_task;

/**
 * Initialization of required data structures and variables
 */
void sut_init()
{
    // init ready task queue 
    task_ready_queue = malloc(sizeof(struct queue));
    *task_ready_queue = queue_create();
    queue_init(task_ready_queue);

    // init task id count 
    taskId = malloc(sizeof(int));
    *taskId = 0;

    printf("Initialization successful\n");
}

/**
 * Create a Task struct and add it to the END of the task ready queue.
 * @param fn: pointer to a C function to execute when task gets to execute
 * @return true if task created successfuly false if not
 */
bool sut_create(sut_task_f fn)
{
    // create and setup new task
    /* new task */
    struct sut_task *task = (struct sut_task *)malloc(sizeof(struct sut_task));
    task->id = *taskId;
    *taskId = *taskId + 1;
    task->fct = fn;

    // add task to queue 
    queue_insert_tail(task_ready_queue, queue_new_node(task));
    printf("Succesfully created task: %d\n", task->id);
    return true;
}

/**
 * pauses the execution of a user thread until it is selected again
 * by the scheduler. Task is put back at the END of the task ready queue. 
 * Add a Task struct to the ready queue.
*/
void sut_yield()
{
    /* currently running task */
    struct sut_task *cur_task = queue_pop_head(task_ready_queue)->data;
    // clone current context 
    getcontext(&(cur_task->context));
    cur_task->context.uc_stack.ss_sp = cur_task->stack;
    cur_task->context.uc_stack.ss_size = SS;
    cur_task->context.uc_link = main_context;
    // add removed task back to queue 
    queue_insert_tail(task_ready_queue, queue_new_node(cur_task));
    // swap back to main context
    swapcontext(&(cur_task->context),main_context);
}

/**
 * Terminates execution of current task.
 * It does not put it back on the task ready queue.
 */
void sut_exit()
{
}

/**
* Terminates program execution. Empty the task queue
*/
void sut_shutdown()
{

    /* init main context */
    main_context = malloc(sizeof(ucontext_t));
    getcontext(main_context);
}

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
