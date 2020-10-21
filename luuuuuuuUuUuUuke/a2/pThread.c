#include <stdio.h>
#include <pthread.h>
#include <unitstd.h>
#include <stdbool.h>

/**
 * Function continuously prints hello world from thread 1
 * 
 * @params: 
 *  arg:      Void pointer as argument
 */
void *thread_1(void *arg){ 
    while(true){
        printf("Hello from thread1\n");
        // To have less terminal span, include small delay before looping again
        usleep(1000*1000); //every second print 
    }
}

int main(){
    /* Initializing pthread handles*/
    pthread_t thread_handle1; 
    pthread_t thread_handle2; 

    pthread_create(&thread_handle1,NULL, thread_1, NULL)

    /* Takes a thread handle and takes a spot to place return of that thread*/
    pthread_join(&thread_handle1,NULL);
}