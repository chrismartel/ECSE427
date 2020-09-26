#include<stdint.h>
#include<stdio.h>
#include<unistd.h>

/* CALCULATOR FUNCTIONS*/

// add two integers
int addInts(int a, int b){
    return (a + b);
}

// multiply two integers
int multiplyInts(int a, int b){
    return (a * b);
}

// divide two float numbers
float divideFloats(float a, float b){
    if(b==0){
        return 0;
    }
    else return (a / b);
}

// sleep calculator for x seconds
int sleepFor(int x){
    sleep(x);
    return 0;
}

// factorial
uint64_t factorial(int x){
    if(x<0){
        return -1;
    }
    else if(x==0){
        return 1;
    }
    else{
        int result = 1;
        for(int i = x;i>=1;i--){
            result = result *i;
        }
        return result;
    }
    return 0;
}