#include "sut.h"
#include "queue.h"

/////////////////// USER LEVEL THREADS DATA ///////////////////

/* function pointer declaration*/
typedef void (*sut_task_f)();

/* queue entry struct declaration*/
struct queue_entry;

/* array of task struct */
sut_task tasks[MAXTHREADS];

/* main context/ parent context */
ucontext_t main_context;

/* current number of user-level threads*/
int numthreads;

/* current thread pointer tool*/
int curthread;

/* ready tasks queue*/
struct queue *task_ready_queue;

/////////////////// KERNEL LEVEL THREADS DATA ///////////////////

/* task ready queue mutex lock */
pthread_mutex_t trq_mutex = PTHREAD_MUTEX_INITIALIZER;

/* kernel threads handles */
pthread_t c_exec_handle;
pthread_t i_exec_handle;

/* execution flag */
bool shutdown;

/* computation execution thread method declaration*/
void *c_exec();

/* wait queue*/
struct queue *wait_queue;

/////////////////// MAIN EXECUTION FUNCTIONS ///////////////////

/**
 * Initialization of required data structures and variables
 */
void sut_init()
{
    // init queues
    task_ready_queue = malloc(sizeof(struct queue));
    *task_ready_queue = queue_create();
    queue_init(task_ready_queue);

    wait_queue = malloc(sizeof(struct queue));
    *wait_queue = queue_create();
    queue_init(wait_queue);

    // init user thread count variables
    numthreads = 0;
    curthread = 0;

    shutdown = false;

    pthread_create(&c_exec_handle, NULL, c_exec, NULL);
    printf("Initialization successful\n");
}

/**
* Terminates program execution. Empty the task queue
*/
void sut_shutdown()
{
    shutdown = true;
    pthread_join(c_exec_handle, NULL);
}

/////////////////// USER LEVEL THREAD LIBRARY FUNCTIONS ///////////////////

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

    /* BEGININNING OF CS */
    pthread_mutex_lock(&trq_mutex);
    queue_insert_tail(task_ready_queue, queue_new_node(task));
    printf("Succesfully created task: %d\n", task->id);
    /* END OF CS */
    pthread_mutex_unlock(&trq_mutex);
    return true;
}

/**
 * pauses the execution of a user thread until it is selected again
 * by the scheduler. Task is put back at the END of the task ready queue. 
 * Add a Task struct to the ready queue.
*/
void sut_yield()
{
    /* BEGININNING OF CS */
    pthread_mutex_lock(&trq_mutex);
    /* currently running task */
    struct sut_task *cur_task = queue_pop_head(task_ready_queue)->data;

    /* END OF CS */
    pthread_mutex_unlock(&trq_mutex);

    // clone current context
    getcontext(&(cur_task->context));
    cur_task->context.uc_stack.ss_sp = cur_task->stack;
    cur_task->context.uc_stack.ss_size = STACKSIZE;
    cur_task->context.uc_link = &main_context;

    // add removed task back to queue
    queue_insert_tail(task_ready_queue, queue_new_node(cur_task));
    /* END OF CS */
    pthread_mutex_unlock(&trq_mutex);

    // swap back to main context
    swapcontext(&(cur_task->context), &main_context);
}

/**
 * Terminates execution of current task.
 * It does not put it back on the task ready queue.
 */
void sut_exit()
{
    /* BEGININNING OF CS */
    pthread_mutex_lock(&trq_mutex);
    /* currently running task */
    struct sut_task *cur_task = queue_pop_head(task_ready_queue)->data;
    /* END OF CS */
    pthread_mutex_unlock(&trq_mutex);

    // clone current context
    getcontext(&(cur_task->context));
    cur_task->context.uc_stack.ss_sp = cur_task->stack;
    cur_task->context.uc_stack.ss_size = STACKSIZE;
    cur_task->context.uc_link = &main_context;
    // swap back to main context
    swapcontext(&(cur_task->context), &main_context);
}

//TODO: IO PART

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

/////////////////// KERNEL LEVEL THREAD FUNCTIONS ///////////////////

void *c_exec()
{

    /* head node of the queue */
    struct queue_entry *head;
    head = malloc(sizeof(struct queue_entry));

    /* head thread task of the queue*/
    struct sut_task *head_task;
    head_task = malloc(sizeof(struct sut_task));

    while (true)
    {
        /* BEGININNING OF CS */
        pthread_mutex_lock(&trq_mutex);

        // queue check
        head = queue_peek_front(task_ready_queue);

        /* END OF CS*/
        pthread_mutex_unlock(&trq_mutex);

        // launch a task
        if (head != NULL)
        {
            head_task = head->data;
            swapcontext(&main_context, &(head_task->context));
        }
        // no tasks left
        else
        {
            printf("Empty task queue\n");
            if (shutdown)
            {
                printf("Shutting down...\n");
                break;
            }
            else
            {
                usleep(1000 * 1000);
                printf("Waiting...\n");
            }
        }
    }
}
