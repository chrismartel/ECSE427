#include "sut.h"
#include "queue.h"
#include "rpc.h"
#include "mystringlib.h"

/** connection mode 
 * true --> mock connection
 * false --> server connection */

bool io_mock = false;

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

/* task ready queue pointer */
struct queue *cpu_queue;

/////////////////// KERNEL LEVEL THREADS DATA ///////////////////

/* computation queue mutex lock */
pthread_mutex_t cpu_mutex = PTHREAD_MUTEX_INITIALIZER;

/* IO queue mutex lock */
pthread_mutex_t io_mutex = PTHREAD_MUTEX_INITIALIZER;

/* kernel threads handles */
pthread_t c_exec_handle;
pthread_t i_exec_handle;

/* shutdown flag */
bool sd = false;

/* io queue empty flag */
bool io_empty = true;

/* computation queue empty flag */
bool cpu_empty = true;

/* computation execution thread method declaration*/
void *c_exec();

/* IO execution thread method declaration*/
void *i_exec();

/////////////////// IO DATA ///////////////////

/* IO wait queue pointer */
struct queue *io_queue;

/* client socket fd */
int sockfd;

/* IO message struct declaration */
struct io_msg;

/* IO Command Table */

int openCmd = 0;

int readCmd = 1;

int writeCmd = 2;

int closeCmd = 3;

/////////////////// MAIN EXECUTION FUNCTIONS ///////////////////

/**
 * Initialization of queues and variables
 * & creation of kernel threads
 */
void sut_init()
{
    // init queues
    cpu_queue = malloc(sizeof(struct queue));
    *cpu_queue = queue_create();
    queue_init(cpu_queue);

    io_queue = malloc(sizeof(struct queue));
    *io_queue = queue_create();
    queue_init(io_queue);

    // init user thread count variables
    numthreads = 0;

    pthread_create(&c_exec_handle, NULL, c_exec, NULL);
    pthread_create(&i_exec_handle, NULL, i_exec, NULL);
    //printf("Initialization successful\n");
}

/**
 * Shutting down by waiting for kernel threads to terminate all current tasks 
 * and by cleaning up internal library state.
*/
void sut_shutdown()
{
    sd = true;
    // wait for IO queue to finish processing IO tasks
    pthread_join(i_exec_handle, NULL);
    // wait for computation queue to finish processing cpu tasks
    pthread_join(c_exec_handle, NULL);

    // reset number of user threads
    numthreads = 0;

    // reset shutdown flag
    sd = false;
}

/////////////////// USER LEVEL THREAD LIBRARY FUNCTIONS ///////////////////

/**
 * Create a task and add it to the end of the task ready queue.
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

    /* BEGININNING OF CPU QUEUE CS */
    pthread_mutex_lock(&cpu_mutex);
    queue_insert_tail(cpu_queue, queue_new_node(task));
    cpu_empty = false;
    // printf("Succesfully created task: %d\n", task->id);
    /* END OF CPU QUEUE CS */
    pthread_mutex_unlock(&cpu_mutex);
    return true;
}

/**
 * Pauses the execution of a task until it is selected again
 * by the FCFS scheduler. The paused task is put back at the end of the ready task queue. 
*/
void sut_yield()
{
    /* BEGININNING OF CPU QUEUE CS */
    pthread_mutex_lock(&cpu_mutex);
    /* currently running task */
    struct sut_task *cur_task = queue_pop_head(cpu_queue)->data;

    // clone current context
    getcontext(&(cur_task->context));
    cur_task->context.uc_stack.ss_sp = cur_task->stack;
    cur_task->context.uc_stack.ss_size = STACKSIZE;
    cur_task->context.uc_link = &main_context;

    // add removed task back to queue
    queue_insert_tail(cpu_queue, queue_new_node(cur_task));
    cpu_empty = false;
    /* END OF CPU QUEUE CS */
    pthread_mutex_unlock(&cpu_mutex);

    // swap back to main context
    swapcontext(&(cur_task->context), &main_context);
}

/**
 * Terminates execution of current task.
 * It does not put it back on the task ready queue.
 */
void sut_exit()
{
    /* BEGININNING OF CPU QUEUE CS */
    pthread_mutex_lock(&cpu_mutex);
    /* currently running task */
    struct sut_task *cur_task = queue_pop_head(cpu_queue)->data;
    /* END OF CPU QUEUE CS */
    pthread_mutex_unlock(&cpu_mutex);

    // clone current context
    getcontext(&(cur_task->context));
    cur_task->context.uc_stack.ss_sp = cur_task->stack;
    cur_task->context.uc_stack.ss_size = STACKSIZE;
    cur_task->context.uc_link = &main_context;
    // swap back to main context
    swapcontext(&(cur_task->context), &main_context);
}

/**
 * Creates an OPEN task message and enqueues it in the IO wait queue
 * @param dest: destination buffer to read data from
 * @param port: port to establish remote process connection
 */
void sut_open(char *dest, int port)
{
    /* BEGININNING OF CPU QUEUE CS */
    pthread_mutex_lock(&cpu_mutex);
    /* currently running task */
    struct sut_task *cur_task = queue_pop_head(cpu_queue)->data;
    /* END OF CPU QUEUE CS */
    pthread_mutex_unlock(&cpu_mutex);

    // clone current context
    getcontext(&(cur_task->context));
    cur_task->context.uc_stack.ss_sp = cur_task->stack;
    cur_task->context.uc_stack.ss_size = STACKSIZE;
    cur_task->context.uc_link = &main_context;

    /* IO Message struct */
    struct io_msg msg;
    msg.cmd = openCmd;
    msg.buf = dest;
    msg.port = port;
    msg.task = cur_task;

    /* BEGININNING OF IO QUEUE CS */
    pthread_mutex_lock(&io_mutex);
    queue_insert_tail(io_queue, queue_new_node(&msg));
    io_empty = false;
    /* END OF IO QUEUE CS */
    pthread_mutex_unlock(&io_mutex);
    // swap back to main context
    swapcontext(&(cur_task->context), &main_context);
}

/**
 * Creates a WRITE task and enqueues it in the IO wait queue
 * @param buf: pointer to data to send to the remote process
 * @param size: size of data to send to remote process
 */
void sut_write(char *buf, int size)
{
    /* BEGININNING OF CPU QUEUE CS */
    pthread_mutex_lock(&cpu_mutex);
    /* currently running task */
    struct sut_task *cur_task = queue_pop_head(cpu_queue)->data;
    /* END OF CPU QUEUE CS */
    pthread_mutex_unlock(&cpu_mutex);

    // clone current context
    getcontext(&(cur_task->context));
    cur_task->context.uc_stack.ss_sp = cur_task->stack;
    cur_task->context.uc_stack.ss_size = STACKSIZE;
    cur_task->context.uc_link = &main_context;

    /* IO Message struct */
    struct io_msg msg;
    msg.cmd = writeCmd;
    msg.buf = buf;
    msg.size = size;
    msg.task = cur_task;

    /* BEGININNING OF IO QUEUE CS */
    pthread_mutex_lock(&io_mutex);
    queue_insert_tail(io_queue, queue_new_node(&msg));
    io_empty = false;
    /* END OF IO QUEUE CS */
    pthread_mutex_unlock(&io_mutex);
    // swap back to main context
    swapcontext(&(cur_task->context), &main_context);
}

/**
 * Creates a CLOSE task and enqueues it in the IO wait queue.
 */
void sut_close()
{
    /* BEGININNING OF CPU QUEUE CS */
    pthread_mutex_lock(&cpu_mutex);
    /* currently running task */
    struct sut_task *cur_task = queue_pop_head(cpu_queue)->data;
    /* END OF CPU QUEUE CS */
    pthread_mutex_unlock(&cpu_mutex);

    // clone current context
    getcontext(&(cur_task->context));
    cur_task->context.uc_stack.ss_sp = cur_task->stack;
    cur_task->context.uc_stack.ss_size = STACKSIZE;
    cur_task->context.uc_link = &main_context;

    /* IO Message struct */
    struct io_msg msg;
    msg.cmd = closeCmd;
    msg.task = cur_task;

    /* BEGININNING OF IO QUEUE CS */
    pthread_mutex_lock(&io_mutex);
    queue_insert_tail(io_queue, queue_new_node(&msg));
    io_empty = false;
    /* END OF IO QUEUE CS */
    pthread_mutex_unlock(&io_mutex);

    // swap back to main context
    swapcontext(&(cur_task->context), &main_context);
}

/**
 * Creates a READ task and enqueues it in the IO wait queue
 */
char *sut_read()
{
    /* BEGININNING OF CPU QUEUE CS */
    pthread_mutex_lock(&cpu_mutex);
    /* currently running task */
    struct sut_task *cur_task = queue_pop_head(cpu_queue)->data;
    /* END OF CPU QUEUE CS */
    pthread_mutex_unlock(&cpu_mutex);

    // clone current context
    getcontext(&(cur_task->context));
    cur_task->context.uc_stack.ss_sp = cur_task->stack;
    cur_task->context.uc_stack.ss_size = STACKSIZE;
    cur_task->context.uc_link = &main_context;

    /* IO Message struct */
    struct io_msg msg;
    msg.cmd = readCmd;
    msg.task = cur_task;

    /* BEGININNING OF IO QUEUE CS */
    pthread_mutex_lock(&io_mutex);
    queue_insert_tail(io_queue, queue_new_node(&msg));
    io_empty = false;
    /* END OF IO QUEUE CS */
    pthread_mutex_unlock(&io_mutex);
    // swap back to main context
    swapcontext(&(cur_task->context), &main_context);

    return cur_task->return_val;
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
        /* BEGININNING OF CPU QUEUE CS */
        pthread_mutex_lock(&cpu_mutex);

        // queue check
        head = queue_peek_front(cpu_queue);
        /* END OF CPU QUEUE CS*/
        pthread_mutex_unlock(&cpu_mutex);
        // launch a task
        if (head != NULL)
        {
            /* BEGININNING OF CPU QUEUE CS */
            pthread_mutex_lock(&io_mutex);
            cpu_empty = false;
            /* END OF CPU QUEUE CS */
            pthread_mutex_unlock(&io_mutex);

            head_task = head->data;
            swapcontext(&main_context, &(head_task->context));
        }
        // no tasks left
        else
        {
            /* BEGININNING OF CPU QUEUE CS */
            pthread_mutex_lock(&cpu_mutex);

            cpu_empty = true;
            /* END OF CPU QUEUE CS*/
            pthread_mutex_unlock(&cpu_mutex);
            if (sd && io_empty)
            {
                printf("Shutting down...\n");
                break;
            }
        }

        usleep(THREADSLEEP);
    }
}

void *i_exec()
{

    /* head node of the queue */
    struct queue_entry *head;
    head = malloc(sizeof(struct queue_entry));

    /* head thread task of the queue*/
    struct io_msg *head_msg;
    head_msg = malloc(sizeof(struct io_msg));

    /* head thread task of the queue*/
    struct sut_task *head_task;
    head_task = malloc(sizeof(struct sut_task));

    bool error = false;
    while (true)
    {
        error = false;
        /* BEGININNING OF IO QUEUE CS */
        pthread_mutex_lock(&io_mutex);

        // queue check
        head = queue_pop_head(io_queue);

        /* END OF IO QUEUE CS*/
        pthread_mutex_unlock(&io_mutex);

        // launch a task
        if (head != NULL)
        {
            /* BEGININNING OF IO QUEUE CS */
            pthread_mutex_lock(&io_mutex);
            io_empty = false;
            /* END OF IO QUEUE CS */
            pthread_mutex_unlock(&io_mutex);


            /* BEGININNING OF CPU QUEUE CS */
            pthread_mutex_lock(&cpu_mutex);
            cpu_empty = false;
            /* END OF CPU QUEUE CS */
            pthread_mutex_unlock(&cpu_mutex);

            head_msg = head->data;
            int cmd = head_msg->cmd;

            /* OPEN command */
            if (cmd == openCmd)
            {
                //printf("CONNECTING TO SERVER...\n");

                /* mock connection mode*/
                if (io_mock)
                {
                    sleep(1);
                }
                /* server connection mode */
                else
                {
                    // connect to server

                    if (connect_to_server(head_msg->buf, head_msg->port, &sockfd) < 0)
                    {
                        fflush(stdout);
                        fprintf(stderr, "Error connecting to server\n");
                        error = true;
                    }
                }
                //printf("connected to host IP: %s at port %d\n", head_msg->buf, head_msg->port);
            }
            /* CLOSE command */
            else if (cmd == closeCmd)
            {
                // close remote process connection
                close(sockfd);
            }
            /* READ COMMAND */
            else if (cmd == readCmd)
            {

                //printf("READING DATA...\n");

                /* mock connection mode */
                if (io_mock)
                {

                    head_msg->task->return_val = "test";
                    sleep(1);
                }
                /* server connection mode */
                else
                {
                    // read data from server and set task return val
                    ssize_t byte_count = recv_message(sockfd, head_msg->task->return_val, sizeof(BUFSIZE));
                    if (byte_count <= 0)
                    {
                        fflush(stdout);
                        fprintf(stderr, "Error in reading data from server...\n");
                        error = true;
                    }
                }
            }
            /* WRITE COMMAND */
            else if (cmd == writeCmd)
            {
                //printf("WRITING TO SERVER...\n");

                /* mock connection mode */
                if (io_mock)
                {
                    printf("%s\n", head_msg->buf);
                }
                /* server connection mode */
                else
                {
                    if (send_message(sockfd, head_msg->buf, head_msg->size) == -1)
                    {
                        fprintf(stderr, "Error in sending message to server...\n");
                        error = true;
                    }
                }
            }
            /* INVALID COMMAND */
            else
            {
                printf("Error: invalid command in IO message queue\n");
                error = true;
            }

            // if IO task executed with no error --> add IO task to C_EXEC task queue
            if (!error)
            {
                /* BEGININNING OF CPU QUEUE CS */
                pthread_mutex_lock(&cpu_mutex);

                // add completed IO task to ready task queue
                queue_insert_tail(cpu_queue, queue_new_node(head_msg->task));
                /* END OF CPU QUEUE CS */
                pthread_mutex_unlock(&cpu_mutex);
            }
        }
        // no IO tasks left
        else
        {
            /* BEGININNING OF IO QUEUE CS */
            pthread_mutex_lock(&io_mutex);
            io_empty = true;
            /* END OF IO QUEUE CS */
            pthread_mutex_unlock(&io_mutex);
            if (sd && cpu_empty)
            {
                printf("IO Shutting down...\n");
                // leave while loop
                break;
            }
        }
        usleep(THREADSLEEP);
    }
}
