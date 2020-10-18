/*
CREATE PTHREAD

int pthread_create(pthread_t *thread, const pthread_attr_t *attr, void *(*start_routine) (void *), void *arg);

thread --> pthread handle id
attr --> pthread attributes, info about the thread: stack size, ...
start_routine --> the function pointer to execute. The function MUST return a void pointer
                  and MUST take a lone void pointer argument.
arg --> lone argument pointer of start_routine
*/

/*
JOIN PTHREAD

int pthread_join(pthread_t thread, void **retval);

thread --> pthread handle id
retval --> copies exit status of target thread into this location
*/

/*
MUTEX FUNCTIONS

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_lock(pthread_mutex_t *mutex)
pthread_mutex_unlock(pthread_mutex_t *mutex)


*/

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <stdbool.h>


void *thread_1(void *arg)
{
    pthread_mutex_t *mutex = arg;
    while (true)
    {
        // lock mutex
        pthread_mutex_lock(mutex);
        printf("Hello ");
        usleep(1000 * 1000);
        printf("from thread 1\n");
        // unlock mutex
        pthread_mutex_unlock(mutex);
        usleep(1000 * 1000);
    }
}

void *thread_2(void *arg)
{
    pthread_mutex_t *mutex = arg;
    while (true)
    {
        // lock mutex
        pthread_mutex_lock(mutex);
        printf("Hello ");
        usleep(1000 * 1000);
        printf("from thread 2\n");
        // unlock mutex
        pthread_mutex_unlock(mutex);

        usleep(1000 * 1000);
    }
}
int main()
{
    pthread_t thread_handle;
    pthread_t thread_handle2;

    /* MUTEX LOCK */

    // initialize a mutex lock
    pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

    // create a thread with ID thread_handle, with default attributes, pointing to function thread_1, and with NULL fct arg
    // pass mutex has variable to our threads
    pthread_create(&thread_handle, NULL, thread_1, &mutex);
    pthread_create(&thread_handle2, NULL, thread_2, &mutex);

    // wait for thread to end
    pthread_join(thread_handle, NULL);
}