#include "a2_lib.h"
#include "SUT.h"
#include "queue.h"

/* Trim function */

char *trim(char *s) {
  char *original = s;
  size_t len = 0;

  while (isspace((unsigned char) *s)) {
    s++;
  } 
  if (*s) {
    char *p = s;
    while (*p) p++;
    while (isspace((unsigned char) *(--p)));
    p[1] = '\0';
    len = (size_t) (p - s + 1);  
  }
  return (s == original) ? s : memmove(original, s, len + 1);
}

//Data for server
int sockfd;

// Define a type name for function pointer
typedef void (*sut_task_f)();

/* Queue data */

// Ready task queue
struct queue* task_queue;

// IO wait queue
struct queue* wait_queue;

// Initialize a struct for a task which is an entry in the queue
struct queue_entry;

/* Kernel-level threads */

// we can use pthreads for implementing kernel-level threading

// Initializing posix thread handles
pthread_t c_thread_handle;
pthread_t i_thread_handle;

// Initializing mutext 
pthread_mutex_t c_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t i_mutex = PTHREAD_MUTEX_INITIALIZER;

// Flags
bool is_shutdown = false;

ucontext_t m; //main context

// C-EXEC function
void *thread_c(){
    //pthread_mutex_t *c_mutex = arg;

    struct queue_entry *head_queue_node = malloc(sizeof(struct queue_entry));
    struct task *task_queue_node = malloc(sizeof(struct task));
    while(true){
        pthread_mutex_lock(&c_mutex);
        head_queue_node = queue_peek_front(task_queue);
        pthread_mutex_unlock(&c_mutex);

        if(head_queue_node!=NULL){
            task_queue_node = head_queue_node -> data;
            swapcontext(&m,&(task_queue_node -> task_context));
        }
        else{
            if(is_shutdown){
                printf("Shutdown\n");
                break;
            }
        }
        usleep(100); //100 microseconds
        printf("waiting\n");
    }
}

// I-EXEC function (second-kernel thread)
void *thread_i(){
    struct queue_entry *head_queue_node = malloc(sizeof(struct queue_entry));
    struct messages *message_queue_node = malloc(sizeof(struct messages));

    while(true){
        pthread_mutex_lock(&i_mutex);
        head_queue_node = queue_peek_front(wait_queue);
        pthread_mutex_unlock(&i_mutex);

        if(head_queue_node!=NULL){
           message_queue_node = head_queue_node -> data;
           char *command_type = message_queue_node -> message_type;
           if (!strcmp(trim(command_type), "OPEN")){ 

           }
            
           else if (!strcmp(trim(command_type), "CLOSE")){
           }
            
           else if (!strcmp(trim(command_type), "READ")){
           }
            
           else if (!strcmp(trim(command_type), "WRITE")){
           }
           else{
            printf("Error: invalid command in IO message queue\n");
           }
        }
        else{
            if(is_shutdown){
                printf("Shutdown\n");
                break;
            }
        }
        usleep(100); //100 microseconds
        printf("waiting\n");
    }
}

/* User-level threads */

// Context switching can be used for user-level threads

// Thread data
int numberThreads;
int currentThread;
task threadarr[MAX_THREADS];

/* SUT Functions */

// Initializing the SUT library (creating 2 kernel-level threads)

void sut_init(){

    // Thread counter
    numberThreads = 0;
    currentThread = 0;

    // Create queue
    task_queue = malloc(sizeof(struct queue));
    *task_queue = queue_create();
    // Initialize queue
    queue_init(task_queue);

    wait_queue = malloc(sizeof(struct queue));
    *wait_queue = queue_create();
    queue_init(wait_queue);

    // Creating kernel-level threads
    pthread_create(&c_thread_handle,NULL, thread_c, NULL);
    pthread_create(&i_thread_handle,NULL, thread_i, NULL);
    printf("init success\n");
}

/**
 *  Takes in a C function as argument and creates new task added 
 *  to the end of the ready queue.
 */ 
bool sut_create(sut_task_f fn){

    if(numberThreads > MAX_THREADS){
        printf("FATAL: Maximum thread limit reached... creation failed! \n");
		return -false;
    }else{
        // Create new task by initializing struct type
        printf("Thread create started\n");
        struct task *new_task = &(threadarr[numberThreads]);
        getcontext(&(new_task -> task_context));

        new_task -> task_id = numberThreads;
        new_task -> task_func = fn;
        new_task->task_stack = (char *)malloc(THREAD_STACK_SIZE);
        new_task -> task_context.uc_stack.ss_sp = new_task -> task_stack;
        new_task -> task_context.uc_stack.ss_size = THREAD_STACK_SIZE;
        new_task -> task_context.uc_link = 0;
        makecontext(&(new_task -> task_context), fn, 0);
        numberThreads++;

        pthread_mutex_lock(&c_mutex);

        /* Add task to back of ready queue*/
        queue_insert_tail(task_queue, queue_new_node(new_task));

        printf("created new task is a success\n");
        pthread_mutex_unlock(&c_mutex);
        return true;
    }
}

/**
 * Pauses execution of a thread until scheduler selects it again. 
 * The task is placed at back of ready queue for further execution.  
 */

void sut_yield(){
    pthread_mutex_lock(&c_mutex);
    struct task* current_task;
    current_task = queue_pop_head(task_queue) ->data;

    // When we swap tasks, the context must be saved before in TCB
    getcontext(&(current_task->task_context));
    current_task -> task_context.uc_stack.ss_sp = current_task -> task_stack;
    current_task -> task_context.uc_stack.ss_size = THREAD_STACK_SIZE;
    current_task -> task_context.uc_link = &m;

    // Add back to ready queue at back
    queue_insert_tail(task_queue, queue_new_node(current_task));
    printf("yielded task is a success\n");
    pthread_mutex_unlock(&c_mutex);
    // Swap contexts
    swapcontext(&(current_task->task_context),&m);

}

void sut_exit(){

    pthread_mutex_lock(&c_mutex);
    struct task* current_task;
    current_task = queue_pop_head(task_queue) ->data;
    printf("Exited successfully!\n");
    pthread_mutex_unlock(&c_mutex);

    getcontext(&(current_task->task_context));
    current_task -> task_context.uc_stack.ss_sp = current_task -> task_stack;
    current_task -> task_context.uc_stack.ss_size = THREAD_STACK_SIZE;
    current_task -> task_context.uc_link = &m;
    

    swapcontext(&(current_task->task_context),&m);
}

void sut_open(char *dest, int port){

    pthread_mutex_lock(&i_mutex);
    struct task *task_to_run;
    task_to_run = queue_pop_head(task_queue) -> data;
    pthread_mutex_unlock(&i_mutex);

    getcontext(&(task_to_run->task_context));
    task_to_run->task_context.uc_stack.ss_sp = task_to_run->task_stack;
    task_to_run->task_context.uc_stack.ss_size = THREAD_STACK_SIZE;
    task_to_run->task_context.uc_link = &m;

    struct messages message;
    strncpy(message.message_type, "OPEN", 10);
    message.message_task = task_to_run;
    message.message_buffer = dest;
    message.message_port = port;

    
    pthread_mutex_lock(&i_mutex);
    queue_insert_tail(wait_queue, queue_new_node(&message));
    printf("Succesfully created OPEN IO task\n");
    
    pthread_mutex_unlock(&i_mutex);

    swapcontext(&(task_to_run -> task_context),&m);
}

void sut_write(char *buf, int size){
    pthread_mutex_lock(&i_mutex);
    struct task *task_to_run;
    task_to_run = queue_pop_head(task_queue) -> data;
    pthread_mutex_unlock(&i_mutex);

    getcontext(&(task_to_run->task_context));
    task_to_run->task_context.uc_stack.ss_sp = task_to_run->task_stack;
    task_to_run->task_context.uc_stack.ss_size = THREAD_STACK_SIZE;
    task_to_run->task_context.uc_link = &m;

    struct messages message;
    strncpy(message.message_type, "WRITE", 10);
    message.message_task = task_to_run;
    message.message_size = size;
    message.message_buffer = buf;

    pthread_mutex_lock(&i_mutex);
    queue_insert_tail(wait_queue, queue_new_node(&message));
    printf("Succesfully created WRITE task\n");
    
    pthread_mutex_unlock(&i_mutex);

    swapcontext(&(task_to_run -> task_context),&m);
}

void sut_close(){
    pthread_mutex_lock(&i_mutex);
    struct task *task_to_run;
    task_to_run = queue_pop_head(task_queue) -> data;
    pthread_mutex_unlock(&i_mutex);

    getcontext(&(task_to_run->task_context));
    task_to_run->task_context.uc_stack.ss_sp = task_to_run->task_stack;
    task_to_run->task_context.uc_stack.ss_size = THREAD_STACK_SIZE;
    task_to_run->task_context.uc_link = &m;

    struct messages message;
    strncpy(message.message_type, "CLOSE", 10);
    message.message_task = task_to_run;

    pthread_mutex_lock(&i_mutex);
    queue_insert_tail(wait_queue, queue_new_node(&message));
    printf("Succesfully created CLOSE task\n");
    
    pthread_mutex_unlock(&i_mutex);

    swapcontext(&(task_to_run -> task_context),&m);
}

char *sut_read(){
    pthread_mutex_lock(&i_mutex);
    struct task *task_to_run;
    task_to_run = queue_pop_head(task_queue) -> data;
    pthread_mutex_unlock(&i_mutex);

    getcontext(&(task_to_run->task_context));
    task_to_run->task_context.uc_stack.ss_sp = task_to_run->task_stack;
    task_to_run->task_context.uc_stack.ss_size = THREAD_STACK_SIZE;
    task_to_run->task_context.uc_link = &m;

    struct messages message;
    strncpy(message.message_type, "READ", 10);
    message.message_task = task_to_run;

    pthread_mutex_lock(&i_mutex);
    queue_insert_tail(wait_queue, queue_new_node(&message));
    printf("Succesfully created CLOSE task\n");
    
    pthread_mutex_unlock(&i_mutex);

    swapcontext(&(task_to_run -> task_context),&m);
}

void sut_shutdown(){
    is_shutdown = true;
    // wait for IO queue to finish processing first
    pthread_join(i_thread_handle, NULL);
    // wait for computation queue to finish last
    pthread_join(c_thread_handle, NULL);

    printf("Shutdown successfully\n");
}
