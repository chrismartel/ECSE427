#include "a2_lib.h"
#include "SUT.h"
#include "queue.h"

// Function declaration to be able to use in file
char *trim(char *s);

// Data for server
int sockfd;
char *read_buf;
const char *host_ip = "127.0.0.0";

// Define a type name for function pointer
typedef void (*sut_task_f)();


/*** Queue data ***/

// Ready task queue
struct queue *task_queue;

// IO wait queue
struct queue *wait_queue;

// Initialize a struct for a task which is an entry in the queue
struct queue_entry;


/*** Kernel-level thread data ***/

// we can use pthreads for implementing kernel-level threading

// Initializing posix thread handles
pthread_t c_thread_handle;
pthread_t i_thread_handle;

// Initializing mutexts
pthread_mutex_t c_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t i_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t nbt_mutex = PTHREAD_MUTEX_INITIALIZER;

// Initializing flags
bool is_shutdown;

// Thread data
int threadCounter, currentThread;

// Initialize array to keep track of tasks, based on YAUThreads
task threadarr[MAX_THREADS];

// Main context
ucontext_t m; 

/*** C-EXEC function ***/

void *thread_c(){

    // Initialize a task and its corresponding node in the queue
    struct task *task_queue_node = malloc(sizeof(struct task));
    struct queue_entry *head_queue_node = malloc(sizeof(struct queue_entry));

    while (true){

        // Peek the first node in the task queue (return its value)
        pthread_mutex_lock(&c_mutex);
        head_queue_node = queue_peek_front(task_queue);
        pthread_mutex_unlock(&c_mutex);

        // Something in the first node
        if (head_queue_node != NULL){
            // Assign the task into proper struct
            task_queue_node = head_queue_node->data;
            // Schedule the next task
            swapcontext(&m, &(task_queue_node->task_context));
        }
        // Empty
        else{
            if ((threadCounter == 0) && is_shutdown){
                printf("Shutdown_c_thread\n");
                break; //exits if shutdown issued
            }
        }
        // Decrease the CPU utilization, 100 ms
        usleep(100); 
    }
}

/*** I-EXEC function (second-kernel thread) ***/

void *thread_i(){

    // Again, initialize an IO message and its corresponding node in the wait queue
    struct messages *message_queue_node = malloc(sizeof(struct messages));
    struct queue_entry *head_queue_node = malloc(sizeof(struct queue_entry));

    // Boolean to check if command is complete
    bool is_complete = true;

    while (true){

        // Peek the first node in the wait queue (return its value)
        pthread_mutex_lock(&i_mutex);
        head_queue_node = queue_peek_front(wait_queue);
        pthread_mutex_unlock(&i_mutex);

        // There is something
        if (head_queue_node != NULL){
            // Assign the node into proper struct io message struct
            message_queue_node = head_queue_node->data;
            // Initialize a char for the command type
            char *command_type;
            command_type = message_queue_node->message_type;

            // Open is called
            if (!strcmp(trim(command_type), "OPEN")){
                printf("Connecting to server\n");
                if (connect_to_server(message_queue_node->message_buffer, message_queue_node->message_port, &sockfd) < 0){
                    fflush(stdout);
                    fprintf(stderr, "Cannot connect to the server\n");
                    is_complete = false;
                }

                read_buf = message_queue_node->message_buffer;
            }

            // Close is called
            else if (!strcmp(trim(command_type), "CLOSE")){
                close(sockfd);
            }

            // Read function called
            else if (!strcmp(trim(command_type), "READ")){
                printf("Reading to data\n");
                char readData[BUFSIZE];
                
                ssize_t byte_count = recv_message(sockfd, readData, sizeof(readData));
                message_queue_node -> message_task -> content = readData;
                if (byte_count <= 0){
                    fflush(stdout);
                    fprintf(stderr, "Error in reading data\n");
                    is_complete = false;
                }
            }

            // Write function is called
            else if (!strcmp(trim(command_type), "WRITE")){
                printf("Writing to data\n");
                if (send_message(sockfd, message_queue_node->message_buffer, message_queue_node->message_size) == -1){
                    fprintf(stderr, "error in sending data\n");
                    is_complete = false;
                }
            }
            else{
                printf("Error: invalid command in IO message queue\n");
                is_complete = false;
            }

            queue_pop_head(wait_queue);
            if (is_complete){
                pthread_mutex_lock(&c_mutex);
                queue_insert_tail(task_queue, queue_new_node(message_queue_node->message_task));
                pthread_mutex_unlock(&c_mutex);
            }
            else{
                pthread_mutex_lock(&nbt_mutex);
                threadCounter = threadCounter - 1;
                pthread_mutex_unlock(&nbt_mutex);

            }
        }

        // Empty queue
        else{
            if ((threadCounter == 0) && is_shutdown){
                printf("Shutdown_i\n");
                break; //exits if shutdown issued
            }
        }
        usleep(100); //100 microseconds
    }
}

/*** User-level threads ***/

// Context switching can be used for user-level threads


/*** SUT Functions ***/

/**
 *  Initialize everything before calling functions.
 */
void sut_init(){

    // Thread counters
    threadCounter = 0;
    currentThread = 0;
    is_shutdown = false;

    // Create queue
    task_queue = malloc(sizeof(struct queue));
    *task_queue = queue_create();

    // Initialize queue
    queue_init(task_queue);

    // Same for the wait queue of io
    wait_queue = malloc(sizeof(struct queue));
    *wait_queue = queue_create();
    queue_init(wait_queue);

    // Creating kernel-level threads
    pthread_create(&c_thread_handle, NULL, thread_c, NULL);
    pthread_create(&i_thread_handle, NULL, thread_i, NULL);
    //printf("init success\n");

    ///what does function does not block its caller mean?
}

/**
 *  Takes in a C function as argument and creates new task added 
 *  to the end of the ready queue. This should be run on the C-exec
 *  thread. 
 */
bool sut_create(sut_task_f fn){

    if (currentThread > MAX_THREADS){
        printf("FATAL: Maximum thread limit reached... creation failed! \n");
        return false;
    }
    else{
        // Create new task by initializing struct type
        printf("Thread create started\n");
        struct task *new_task = &(threadarr[currentThread]);
        getcontext(&(new_task->task_context));

        // Define the parameters of the new task that is created
        new_task->task_id = currentThread;
        // Function passed as argument to the new task
        new_task->task_func = fn;
        new_task->task_stack = (char *)malloc(THREAD_STACK_SIZE);
        new_task->task_context.uc_stack.ss_sp = new_task->task_stack;
        new_task->task_context.uc_stack.ss_size = THREAD_STACK_SIZE;
        new_task->task_context.uc_link = 0;

        // Create the context for the new task
        makecontext(&(new_task->task_context), fn, 0);

        // Increase the thread number counter by 1
        currentThread = currentThread + 1;

        pthread_mutex_lock(&nbt_mutex);

        threadCounter = threadCounter + 1;
        pthread_mutex_unlock(&nbt_mutex);

        pthread_mutex_lock(&c_mutex);

        /* Add task to back of ready queue as this is run on C-exec*/
        queue_insert_tail(task_queue, queue_new_node(new_task));

        //printf("Created new task is a success\n");

        pthread_mutex_unlock(&c_mutex);
        return true;
    }
}

/**
 * Pauses execution of a thread until scheduler selects it again. 
 * The task is placed at back of ready queue for further execution.
 * Yield is called within a user task. 
 *   
 */

void sut_yield(){
    pthread_mutex_lock(&c_mutex);

    // Initialize thread control block
    struct task *tcb;
    // Assign it the first task in queue of tasks
    tcb = queue_pop_head(task_queue)->data;

    // When we swap tasks, the context must be saved before in TCB
    getcontext(&(tcb->task_context));
    tcb->task_context.uc_stack.ss_sp = tcb->task_stack;
    tcb->task_context.uc_stack.ss_size = THREAD_STACK_SIZE;
    tcb->task_context.uc_link = &m;

    // Add back to ready queue at back
    queue_insert_tail(task_queue, queue_new_node(tcb));

    //printf("yielded task is a success\n");
    pthread_mutex_unlock(&c_mutex);

    // Swap contexts with the main context
    swapcontext(&(tcb->task_context), &m);

    // the C-exec task context is properly saved
    //the c-exec thread schedules the next available task
}

/**
 *  The sut_exit() function will
 *  Called within user task.
 */
void sut_exit(){
    pthread_mutex_lock(&c_mutex);

    // Initialize thread control block that will hold task to destroy
    struct task *tcb;
    // Get currently running task, which is the head of queue (first task)
    tcb = queue_pop_head(task_queue)->data;
    //printf("Exited successfully!\n");
    pthread_mutex_unlock(&c_mutex);

    // Get the context of the tcb to be able to swap context
    getcontext(&(tcb->task_context));
    
    pthread_mutex_lock(&nbt_mutex);
    threadCounter = threadCounter - 1;
    pthread_mutex_unlock(&nbt_mutex);

    // Does not resume the task later as it is not placed in the ready queue

    // Instead simply swap to the main context who will be able to select next task in queue
    swapcontext(&(tcb->task_context), &m);
    //current task context is removed from any state
}

/**
 * 
 */
void sut_open(char *dest, int port){

    pthread_mutex_lock(&i_mutex);
    // Instantiate a task struct that will represents the task being runned
    struct task *tcb;
    tcb = queue_pop_head(task_queue)->data;
    pthread_mutex_unlock(&i_mutex);

    // Save the context of the
    getcontext(&(tcb->task_context));
    tcb->task_context.uc_stack.ss_sp = tcb->task_stack;
    tcb->task_context.uc_stack.ss_size = THREAD_STACK_SIZE;
    tcb->task_context.uc_link = &m;

    struct messages message;
    strncpy(message.message_type, "OPEN", 10);
    // Associate the socket to the task to run
    message.message_task = tcb;
    // Assign the destination and the port
    message.message_buffer = dest;
    message.message_port = port;

    pthread_mutex_lock(&i_mutex);
    // Add task to the wait queue of IO at the back (tail)
    queue_insert_tail(wait_queue, queue_new_node(&message));
    printf("Succesfully created OPEN IO task\n");
    pthread_mutex_unlock(&i_mutex);

    swapcontext(&(tcb->task_context), &m);
}

/**
 * 
 */
void sut_write(char *buf, int size){

    // Add error check for sut_open() before performing write

    // Since accessing shared data, use mutex lock for critical section
    pthread_mutex_lock(&i_mutex);
    // Initialize a tcb representing calling task
    struct task *tcb;
    // Task calling is the first task in queue, assign to tcb
    tcb = queue_pop_head(task_queue)->data;
    pthread_mutex_unlock(&i_mutex);

    getcontext(&(tcb->task_context));
    tcb->task_context.uc_stack.ss_sp = tcb->task_stack;
    tcb->task_context.uc_stack.ss_size = THREAD_STACK_SIZE;
    tcb->task_context.uc_link = &m;

    // Initialize the message struct for IO communication
    struct messages message;
    strncpy(message.message_type, "WRITE", 10);
    // Assign the calling task to the task field of the message
    message.message_task = tcb;
    // Assign the size byte argument to the defined field in the struct
    message.message_size = size;
    // Assign the buf argument to the message struct
    message.message_buffer = buf;

    pthread_mutex_lock(&i_mutex);
    // Add the new IO write task to the wait queue in IO
    queue_insert_tail(wait_queue, queue_new_node(&message));

    printf("Succesfully created WRITE task\n");
    pthread_mutex_unlock(&i_mutex);

    //Kross:
    // swap context when calling task is still continuing on c-exec
    swapcontext(&(tcb->task_context), &m);
}

/**
 * 
 */
void sut_close(){
    //implement open verification check

    pthread_mutex_lock(&i_mutex);
    struct task *tcb;
    tcb = queue_pop_head(task_queue)->data;
    pthread_mutex_unlock(&i_mutex);

    getcontext(&(tcb->task_context));
    tcb->task_context.uc_stack.ss_sp = tcb->task_stack;
    tcb->task_context.uc_stack.ss_size = THREAD_STACK_SIZE;
    tcb->task_context.uc_link = &m;

    struct messages message;
    strncpy(message.message_type, "CLOSE", 10);
    message.message_task = tcb;

    pthread_mutex_lock(&i_mutex);
    queue_insert_tail(wait_queue, queue_new_node(&message));
    printf("Succesfully created CLOSE task\n");

    pthread_mutex_unlock(&i_mutex);

    // why swap contexts when task should not be interrupted for close
    swapcontext(&(tcb->task_context), &m);
}

char *sut_read(){

    pthread_mutex_lock(&i_mutex);
    struct task *tcb;
    tcb = queue_pop_head(task_queue)->data;
    pthread_mutex_unlock(&i_mutex);

    getcontext(&(tcb->task_context));
    tcb->task_context.uc_stack.ss_sp = tcb->task_stack;
    tcb->task_context.uc_stack.ss_size = THREAD_STACK_SIZE;
    tcb->task_context.uc_link = &m;

    struct messages message;
    strncpy(message.message_type, "READ", 10);
    message.message_task = tcb;

    pthread_mutex_lock(&i_mutex);
    queue_insert_tail(wait_queue, queue_new_node(&message));
    printf("Succesfully created READ task\n");
    pthread_mutex_unlock(&i_mutex);

    swapcontext(&(tcb->task_context), &m);

    return tcb->content;
}

/**
 *  Function used to finish currently running tasks when no more
 *  tasks to add. (Join threads)
 */
void sut_shutdown(){
    // Flag to know that shutdown function has been called
    is_shutdown = true;
    pthread_join(i_thread_handle, NULL);
    pthread_join(c_thread_handle, NULL);

    //printf("Shutdown successfully\n");
    
    // Reset variables
    threadCounter = 0;
    currentThread =0;
    is_shutdown = false;
    
}

/* Trim function used to have correct spacing*/

char *trim(char *s)
{
    char *original = s;
    size_t len = 0;

    while (isspace((unsigned char)*s))
    {
        s++;
    }
    if (*s)
    {
        char *p = s;
        while (*p)
            p++;
        while (isspace((unsigned char)*(--p)))
            ;
        p[1] = '\0';
        len = (size_t)(p - s + 1);
    }
    return (s == original) ? s : memmove(original, s, len + 1);
}