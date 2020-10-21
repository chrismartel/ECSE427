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
struct queue *task_queue;

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

// C-EXEC function
void *thread_c(){

}

/* User-level threads */

// Context switching can be used for user-level threads

ucontext_t m; //main context

// I-EXEC function
void *thread_i(){

}

/* SUT Functions */

// Initializing the SUT library (creating 2 kernel-level threads)

void sut_init(){

    // Creating kernel-level threads
    //pthread_create(&c_thread_handle,NULL, thread_c, NULL);
    //pthread_create(&i_thread_handle,NULL, thread_i, NULL);
}

bool sut_create(sut_task_f fn){

}

void sut_yield(){

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
