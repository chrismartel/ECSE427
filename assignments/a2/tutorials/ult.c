
/*
MAKE CONTEXT

void makecontext(ucontext_t *ucp, void (*func)(), int argc, ...);

The  makecontext()  function modifies the context pointed to by ucp (which was obtained from a call to getcon‐
ext(3)).  Before invoking makecontext(), the caller must allocate a new stack for this context and assign its
address to ucp->uc_stack, and define a successor context and assign its address to ucp->uc_link. 
When  this  context is later activated (using setcontext(3) or swapcontext()) the function func is called, and
passed the series of integer (int) arguments that follow argc; the caller must specify  the  number  of  these
arguments  in argc.  When this function returns, the successor context is activated.  If the successor context
pointer is NULL, the thread exits.
*/

/*
SWAP CONTEXTS 

int swapcontext(ucontext_t *oucp, const ucontext_t *ucp);

The swapcontext() function saves the current context in the structure pointed to by oucp, and  then  activates
the context pointed to by ucp.
*/

/*
GET CONTEXT

int getcontext(ucontext_t *ucp);

The function getcontext() initializes the structure pointed at by ucp to the currently active context.
*/

/*
SET CONTEXT

int setcontext(const ucontext_t *ucp);

The function setcontext() restores the user context pointed at by ucp.  A successful call does not  return.
The  context  should  have  been  obtained by a call of getcontext(), or makecontext(3), or passed as third
argument to a signal handler.
*/

#include <ucontext.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>

void f1(void *main, void *self)
{
    while (true)
    {
        usleep(1000 * 1000);
        printf("Hello from thread 1\n");
        // swap back to main context
        // start point --> where the previous swap context happened
        swapcontext(self, main);
    }
}

void f2(void *main, void *self)
{
    while (true)
    {
        usleep(1000 * 1000);
        printf("Hello from thread 2\n");
        swapcontext(self, main);
    }
}


int main()
{

    ucontext_t t1, t2, m;
    // m --> main context: responsible for scheduling the next thread.

    // create stacks for user level threads
    char f1stack[16 * 1024];
    char f2stack[16 * 1024];

    // make a copy of currently running context (main) into context t1 using "getcontext"
    getcontext(&t1);

    // SET UP CONTEXT

    // set stack pointer
    t1.uc_stack.ss_sp = f1stack;
    // set stack size
    t1.uc_stack.ss_size = sizeof(f1stack);
    // specify a parent context (context that will resume in case of error/crash)
    t1.uc_link = &m;

    // CREATE CONTEXT
    // when context runs--> run f1()
    makecontext(&t1, (void (*)())f1, 2, &m, &t1);

    getcontext(&t2);
    t2.uc_stack.ss_sp = f2stack;
    t2.uc_stack.ss_size = sizeof(f2stack);
    t2.uc_link = &m;
    makecontext(&t2, (void (*)())f2, 2, &m, &t2);

    bool flag = true;
    int counter = 0;
    while (counter <= 10)
    {
        if (flag)
        {
            // SWAP CONTEXT
            // will make t1 active
            // will save previous active context in m
            swapcontext(&m, &t1);
        }
        else
        {
            swapcontext(&m, &t2);
        }
        flag = !flag;
        counter++;
    }

    printf("exit from main\n");
    return 0;
}