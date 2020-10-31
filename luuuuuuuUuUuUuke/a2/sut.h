#define _XOPEN_SOURCE

#ifndef __SUT_H__
#define __SUT_H__

#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <ucontext.h>
#include <stdbool.h>

/*** Initialize macros ***/

#define BUFSIZE 1024
#define MAX_THREADS 32 // Max number of threads
#define THREAD_STACK_SIZE 1024*64 // YAUT variale
#define CSLEEP 100 // Sleep macro equivalent to 100 ms as defined in instructions

// Define a type name for function pointer
typedef void (*sut_task_f)();

/*** Define the struct parameters for IO messaging ***/
typedef struct messages
{
    int message_id;
    int message_size;
    int message_port;
    char message_type[10];
    char *message_buffer;
    struct task *message_task;
} messages;

/*** Define the struct parameters for a task, based on YAUThreads ***/

typedef struct task
{
	int task_id;
	char *task_stack;
	char *content;
    sut_task_f task_func;
	ucontext_t task_context;
    
} task;

/*** SUT library API ***/

void sut_init();
bool sut_create(sut_task_f fn);
void sut_yield();
void sut_exit();
void sut_open(char *dest, int port);
void sut_write(char *buf, int size);
void sut_close();
char *sut_read();
void sut_shutdown();


#endif
