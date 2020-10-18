#ifndef __SUT_H__
#define __SUT_H__

#include <stdbool.h>
#include <ucontext.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

typedef void (*sut_task_f)();

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