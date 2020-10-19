#include "sut.h"
#include "queue.h"

sut_task tasks[MAXTHREADS];
ucontext_t main_context;
int numthreads;
int curthread;

/* function pointer */
typedef void (*sut_task_f)();

/* queue entry struct declaration*/
struct queue_entry;

/* ready tasks queue*/
struct queue *task_ready_queue;

/**
 * Initialization of required data structures and variables
 */
void sut_init()
{
    // init ready task queue
    task_ready_queue = malloc(sizeof(struct queue));
    *task_ready_queue = queue_create();
    queue_init(task_ready_queue);

    // variables
    numthreads = 0;
    curthread = 0;

    printf("Initialization successful\n");
}

/**
 * Create a Task struct and add it to the END of the task ready queue.
 * @param fn: pointer to a C function to execute when task gets to execute
 * @return true if task created successfuly false if not
 */
bool sut_create(sut_task_f fn)
{
    if (numthreads >= MAXTHREADS)
    {
        printf("Error: Maximum thread limit reached... creation failed! \n");
        return false;
    }
    // create and setup new task
    /* new task */
    struct sut_task *task = &(tasks[numthreads]);
    getcontext(&(task->context));
    task->id = numthreads;
    task->stack = (char *)malloc(STACKSIZE);
    task->context.uc_stack.ss_sp = task->stack;
    task->context.uc_stack.ss_size = STACKSIZE;
    task->context.uc_link = 0;
    task->fct = fn;
    // create context for this task
    makecontext(&(task->context), fn, 0);
    numthreads++;
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
    cur_task->context.uc_stack.ss_size = STACKSIZE;
    cur_task->context.uc_link = &main_context;
    // add removed task back to queue
    queue_insert_tail(task_ready_queue, queue_new_node(cur_task));
    // swap back to main context
    swapcontext(&(cur_task->context), &main_context);
}

/**
 * Terminates execution of current task.
 * It does not put it back on the task ready queue.
 */
void sut_exit()
{
    /* currently running task */
    struct sut_task *cur_task = queue_pop_head(task_ready_queue)->data;
    // clone current context
    getcontext(&(cur_task->context));
    cur_task->context.uc_stack.ss_sp = cur_task->stack;
    cur_task->context.uc_stack.ss_size = STACKSIZE;
    cur_task->context.uc_link = &main_context;
    // swap back to main context
    swapcontext(&(cur_task->context), &main_context);
}

/**
* Terminates program execution. Empty the task queue
*/
void sut_shutdown()
{
    /* head node of the queue */
    struct queue_entry *head;
    head = malloc(sizeof(struct queue_entry));

    /* head thread task of the queue*/
    struct sut_task *head_task;
    head_task = malloc(sizeof(struct sut_task));

    while (true)
    {
        // check if task left in the ready queue
        head = queue_peek_front(task_ready_queue);
        // execute task
        if (head != NULL)
        {
            // set head task
            head_task = head->data;
            // run head task context, save current context state in main_context context
            swapcontext(&main_context, &(head_task->context));
        }
        // no tasks left
        else
        {
            printf("Empty task queue\n");
            break;
        }
    }
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
