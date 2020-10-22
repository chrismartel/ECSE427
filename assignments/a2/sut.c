#include "sut.h"
#include "queue.h"
#include "rpc.h"
#include "mystringlib.h"

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

/* IO wait queue mutex lock */
pthread_mutex_t io_mutex = PTHREAD_MUTEX_INITIALIZER;

/* kernel threads handles */
pthread_t c_exec_handle;
pthread_t i_exec_handle;

/* execution flag */
bool sd = false;

/* io queue empty flag */
bool io_empty = true;

/* task queue empty flag */
bool trq_empty = true;

/* computation execution thread method declaration*/
void *c_exec();

/* IO execution thread method declaration*/
void *i_exec();

/////////////////// IO DATA ///////////////////

/* IO wait queue*/
struct queue *io_queue;

/* client socket fd */
int sockfd;

/* host ip address */
const char *host_ip = "127.0.0.0";

/* Read IO buffer pointer */
char *read_buf;

/* IO message struct */
struct io_msg;

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

    io_queue = malloc(sizeof(struct queue));
    *io_queue = queue_create();
    queue_init(io_queue);

    // init user thread count variables
    numthreads = 0;
    curthread = 0;

    pthread_create(&c_exec_handle, NULL, c_exec, NULL);
    pthread_create(&i_exec_handle, NULL, i_exec, NULL);
    printf("Initialization successful\n");
}

/**
* Terminates program execution. Empty the task queue
*/
void sut_shutdown()
{
    sd = true;
    // wait for IO queue to finish processing first
    pthread_join(i_exec_handle, NULL);
    // wait for computation queue to finish last
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

/**
 * Creates an OPEN task message and enqueues it in the IO wait queue
 * @param dest: destination buffer to read data from
 * @param port: port to establish remote process connection
 */
void sut_open(char *dest, int port)
{
    /* BEGININNING OF CEXEC QUEUE CS */
    pthread_mutex_lock(&trq_mutex);
    /* currently running task */
    struct sut_task *cur_task = queue_pop_head(task_ready_queue)->data;
    /* END OF CEXEC QUEUE CS */
    pthread_mutex_unlock(&trq_mutex);

    // clone current context
    getcontext(&(cur_task->context));
    cur_task->context.uc_stack.ss_sp = cur_task->stack;
    cur_task->context.uc_stack.ss_size = STACKSIZE;
    cur_task->context.uc_link = &main_context;

    /* IO Message struct */
    struct io_msg msg;
    strncpy(msg.cmd, "OPEN", CMDSIZE);
    msg.buf = dest;
    msg.port = port;
    msg.task = cur_task;

    /* BEGININNING OF IO QUEUE CS */
    pthread_mutex_lock(&io_mutex);
    queue_insert_tail(io_queue, queue_new_node(&msg));
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
    /* BEGININNING OF CEXEC QUEUE CS */
    pthread_mutex_lock(&trq_mutex);
    /* currently running task */
    struct sut_task *cur_task = queue_pop_head(task_ready_queue)->data;
    /* END OF CEXEC QUEUE CS */
    pthread_mutex_unlock(&trq_mutex);

    // clone current context
    getcontext(&(cur_task->context));
    cur_task->context.uc_stack.ss_sp = cur_task->stack;
    cur_task->context.uc_stack.ss_size = STACKSIZE;
    cur_task->context.uc_link = &main_context;

    /* IO Message struct */
    struct io_msg msg;
    strncpy(msg.cmd, "WRITE", CMDSIZE);
    msg.buf = buf;
    msg.size = size;
    msg.task = cur_task;

    /* BEGININNING OF IO QUEUE CS */
    pthread_mutex_lock(&io_mutex);
    queue_insert_tail(io_queue, queue_new_node(&msg));
    struct io_msg *test = queue_peek_front(io_queue)->data;
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
    /* BEGININNING OF CEXEC QUEUE CS */
    pthread_mutex_lock(&trq_mutex);
    /* currently running task */
    struct sut_task *cur_task = queue_pop_head(task_ready_queue)->data;
    /* END OF CEXEC QUEUE CS */
    pthread_mutex_unlock(&trq_mutex);

    // clone current context
    getcontext(&(cur_task->context));
    cur_task->context.uc_stack.ss_sp = cur_task->stack;
    cur_task->context.uc_stack.ss_size = STACKSIZE;
    cur_task->context.uc_link = &main_context;

    /* IO Message struct */
    struct io_msg msg;
    strncpy(msg.cmd, "CLOSE", CMDSIZE);
    msg.task = cur_task;

    /* BEGININNING OF IO QUEUE CS */
    pthread_mutex_lock(&io_mutex);
    queue_insert_tail(io_queue, queue_new_node(&msg));
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
    /* BEGININNING OF CEXEC QUEUE CS */
    pthread_mutex_lock(&trq_mutex);
    /* currently running task */
    struct sut_task *cur_task = queue_pop_head(task_ready_queue)->data;
    /* END OF CEXEC QUEUE CS */
    pthread_mutex_unlock(&trq_mutex);

    // clone current context
    getcontext(&(cur_task->context));
    cur_task->context.uc_stack.ss_sp = cur_task->stack;
    cur_task->context.uc_stack.ss_size = STACKSIZE;
    cur_task->context.uc_link = &main_context;

    /* IO Message struct */
    struct io_msg msg;
    strncpy(msg.cmd, "READ", CMDSIZE);
    msg.task = cur_task;

    /* BEGININNING OF IO QUEUE CS */
    pthread_mutex_lock(&io_mutex);
    queue_insert_tail(io_queue, queue_new_node(&msg));
    /* END OF IO QUEUE CS */
    pthread_mutex_unlock(&io_mutex);
    // swap back to main context
    swapcontext(&(cur_task->context), &main_context);
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
            trq_empty = false;
            head_task = head->data;
            swapcontext(&main_context, &(head_task->context));
        }
        // no tasks left
        else
        {
            trq_empty = true;
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
        /* BEGININNING OF CS */
        pthread_mutex_lock(&io_mutex);

        // queue check
        head = queue_pop_head(io_queue);

        /* END OF CS*/
        pthread_mutex_unlock(&io_mutex);

        // launch a task
        if (head != NULL)
        {
            io_empty = false;
            head_msg = head->data;
            char *cmd = head_msg->cmd;

            /* OPEN command */
            if (!strcmp(trim(cmd), "OPEN"))
            {
                // connect to server
                printf("CONNECTING TO SERVER...\n");
                if (connect_to_server(host_ip, head_msg->port, &sockfd) < 0)
                {
                    fflush(stdout);
                    fprintf(stderr, "Error connecting to server\n");
                    error = true;
                }
                // set read destination buffer
                read_buf = head_msg->buf;
            }
            /* CLOSE command */
            else if (!strcmp(trim(cmd), "CLOSE"))
            {
                // close remote process connection
                close(sockfd);
            }
            /* READ COMMAND */
            else if (!strcmp(trim(cmd), "READ"))
            {
                /* receive server response*/
                printf("READING DATA...\n");
                ssize_t byte_count = recv_message(sockfd, read_buf, sizeof(read_buf));
                if (byte_count <= 0)
                {
                    fflush(stdout);
                    fprintf(stderr, "Error in reading data from server...\n");
                    error = true;
                }
            }
            /* WRITE COMMAND */
            else if (!strcmp(trim(cmd), "WRITE"))
            {
                printf("WRITING TO SERVER...\n");
                /* send data to server*/
                if (send_message(sockfd, head_msg->buf, head_msg->size) == -1)
                {
                    fprintf(stderr, "Error in sending message to server...\n");
                    error = true;
                }
            }
            /* INVALID COMMAND */
            else
            {
                printf("Error: invalid command in IO message queue\n");
                error = true;
            }

            // if IO task executed with no error
            if (!error)
            {
                /* BEGININNING OF CS */
                pthread_mutex_lock(&trq_mutex);

                // add completed IO task to ready task queue
                queue_insert_tail(task_ready_queue, queue_new_node(head_msg->task));
                /* END OF CS */
                pthread_mutex_unlock(&trq_mutex);
            }
        }
        // no IO tasks left
        else
        {
            io_empty = true;
            if (sd && trq_empty)
            {
                printf("IO Shutting down...\n");
                // leave while loop
                break;
            }
        }
        usleep(THREADSLEEP);
    }
}
