#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <stdbool.h>

/**
 * Information on the pthreads file
 * 
 * Posix threads are implemented which include the following functions: 
 * (**important**: pthreads are just to interact with the kernel**) 
 * 
 *  - pthread_create(pthread_t *thread, const pthread_attr_t *attr, void *(*start_routine) (void *), void *arg)
 *  
 *  - pthread_join(pthread_t __th, void **__thread_return) -> int
 * 
 * Mutext locks will be implemented to take care of synchronization. Involves the following:
 *  
 *  - pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER; 
 * (Equivalent to initializing a mutex with init() function with parameter attribute as NULL and no error checks)
 * 
 *  - pthread_mutex_lock();
 * 
 *  - pthread_mutex_unlock();
 * 
 * General idea: Protect critical section with mutex. Before critical section, take a lock, and release 
 * the lock when the critical section is over. 
 *  
 */

/**
 * Function continuously prints hello world from thread 1
 * 
 * @params: 
 *  arg:      Void pointer as argument
 */
void *thread_1(void *arg){ 
    pthread_mutex_t *mutex = arg; //give access to mutex
    while(true){
        pthread_mutex_lock(mutex);
        printf("Hello from\n");
        usleep(1000);
        printf("thread1\n");
        pthread_mutex_unlock(mutex);
        usleep(1000*1000); //Every second print (less terminal spam)
    }
}

/**
 * Function continuously prints hello world from thread 2
 * 
 * @params: 
 *  arg:      Void pointer as argument
 */
void *thread_2(void *arg){ 
    pthread_mutex_t *mutex = arg;
    while(true){
        pthread_mutex_lock(mutex);
        printf("Hello from\n");
        usleep(1000);
        printf("thread2\n");
        pthread_mutex_unlock(mutex);
        usleep(1000*1000); //Every second print (less terminal spam)
    }
}

int main(){
    /* Initializing posix thread handles*/
    pthread_t thread_handle1; 
    pthread_t thread_handle2; 

    /* Initializing mutext */
    pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

    pthread_create(&thread_handle1,NULL, thread_1, &mutex);
    pthread_create(&thread_handle2,NULL, thread_2, &mutex);

    /* Takes a thread handle and takes a spot to place return of that thread*/
    pthread_join(thread_handle1,NULL);
    pthread_join(thread_handle2,NULL);
}