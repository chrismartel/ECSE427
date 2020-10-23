#ifndef __SUT_H__
#define __SUT_H__

#include <stdbool.h>
#include <ucontext.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>


#define STACKSIZE 1024 * 64
#define MAXTHREADS 15
#define THREADSLEEP 100 // microseconds
#define CMDSIZE 10

typedef void (*sut_task_f)();

typedef struct sut_task
{
    int id;
    char *stack;
    sut_task_f fct;
    ucontext_t context;
    char *return_val;

} sut_task;

typedef struct io_msg
{
    int cmd;
    int port;
    int size;
    char *buf;
    struct sut_task *task;
} io_msg;

void sut_init();
bool sut_create(sut_task_f fn);
void sut_yield();
void sut_exit();
void sut_shutdown();
void sut_open(char *dest, int port);
void sut_write(char *buf, int size);
void sut_close();
char *sut_read();

#endif