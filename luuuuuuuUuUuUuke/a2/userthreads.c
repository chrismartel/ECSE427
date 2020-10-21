

/**
 * User context -> tells kernel what is the running thread
 *  
 *      - void makecontext(ucontext_t *ucp, void (*func)(), in argc, ...);
 * (give it a task and a function that should run inside a new context, and then
 * initializes that context, so starts that function)
 * 
 *      - int swapcontext(ucontext_t *oucp, const ucontext_t *ucp);
 * (switching between mutliplt threads of control within a process)
 * (saves the current context in the structure pointed to by oucp, 
 * and then activates the context pointed to by ucp)
 * 
 */
#include <stdlib.h>
#include <stdio.h>
#include <ucontext.h>
#include <unistd.h>
#include <stdbool.h>

void f1(void *main, void *self){
    while(true){
        usleep(1000*1000);
        printf("Hello from thread 1\n");
        swapcontext(self, main);
    }
}

void f2(void *main, void *self){
    while(true){
        usleep(1000*1000);
        printf("Hello from thread 2\n");
        swapcontext(self, main);
    }
}

int main(){

    ucontext_t t1, t2, m; //task 1, task 2, main contexts

    /* Initialize stacks for functions */
    char f1stack[16*1024];
    char f2stack[16*1024];

    getcontext(&t1);
    t1.uc_stack.ss_sp = f1stack; //uc_stack: is the stack, ss_sp: stack pointer
    t1.uc_stack.ss_size = sizeof(f1stack);

    /* The parent context is called the linker in case task crashes or yields */
    t1.uc_link = &m;

    /* set initial state of context, run as f1 called */
    makecontext(&t1, (void(*)())f1, 2, &m, &t1); //last argument sets number of arguments to pass

    getcontext(&t2);
    t2.uc_stack.ss_sp = f2stack; 
    t2.uc_stack.ss_size = sizeof(f2stack);
    t2.uc_link = &m;
    makecontext(&t2, (void(*)())f2, 2, &m, &t2);

    bool flagfirst = true;
    while(true){
        if(flagfirst){
            swapcontext(&m, &t1);
        }else{
            swapcontext(&m,&t2);
        }
        flagfirst = !flagfirst;
    }
    

    printf("exit from main\n");
    return 0;

}