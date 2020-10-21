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

// Define a type name for function pointer
typedef void (*sut_task_f)();

/* Queue data */

// Ready task queue
struct queue* task_queue;

// IO wait queue
struct queue* wait_queue;

// A task is an entry in the queue
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

// C-EXEC function
void *thread_c(){

}

/* User-level threads */

// Context switching can be used for user-level threads

ucontext_t m; //main context

int numberThreads;

// I-EXEC function
void *thread_i(){

}

/* SUT Functions */

// Initializing the SUT library (creating 2 kernel-level threads)

void sut_init(){

    // Thread counter
    numberThreads = 0;

    // Create queue
    task_queue = malloc(sizeof(struct queue));
    *task_queue = queue_create();
    // Initialize queue
    queue_init(task_queue);

    wait_queue = malloc(sizeof(struct queue));
    *wait_queue = queue_create();
    queue_init(wait_queue);

    // Creating kernel-level threads
    //pthread_create(&c_thread_handle,NULL, thread_c, NULL);
    //pthread_create(&i_thread_handle,NULL, thread_i, NULL);
    printf("init success\n");
}

/**
 *  Takes in a C function as argument and creates new task added 
 *  to the end of the ready queue.
 */ 
bool sut_create(sut_task_f fn){

    if(numberThreads > MAX_THREADS){
        printf("Error: Limit of created threads has been reached.\n");
        return false;
    }else{
        // Create new task by initializing struct type
        struct task *new_task = malloc(sizeof(struct task));
        new_task -> task_id = numberThreads;
        new_task -> task_func = fn;
        new_task -> task_context.uc_stack.ss_sp = new_task -> task_stack;
        new_task -> task_context.uc_stack.ss_size = THREAD_STACK_SIZE;
        new_task -> task_context.uc_link = 0;
        makecontext(&(new_task -> task_context), fn, 0);
        numberThreads++;

        // Add task to back of ready queue
        queue_insert_tail(task_queue, queue_new_node(new_task));

        printf("created new task is a success\n");
        return true;
    }
}

/**
 * Pauses execution of a thread until scheduler selects it again. 
 * The task is placed at back of ready queue for further execution.  
 */

void sut_yield(){
    struct task* current_task;
    current_task = queue_pop_head(task_queue) ->data;

    // When we swap tasks, the context must be saved before in TCB
    getcontext(&(current_task->task_context));
    current_task -> task_context.uc_stack.ss_sp = current_task -> task_stack;
    current_task -> task_context.uc_stack.ss_size = THREAD_STACK_SIZE;
    current_task -> task_context.uc_link = &m;

    // Add back to ready queue at back
    queue_insert_tail(task_queue, queue_new_node(current_task));

    // Swap contexts
    swapcontext(&(current_task->task_context),&m);

}

void sut_exit(){

}

void sut_open(char *dest, int port){

}

void sut_write(char *buf, int size){

}

void sut_close(){

}

char *sut_read(){

}

void sut_shutdown(){
    
}
