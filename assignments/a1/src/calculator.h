#include <stdint.h>
#include <stdio.h>
#include <unistd.h>

/**
 * add two integers
 * 
 * @params: 
 *   a:        first integer a
 *   b:        second integer b
 * @return:    The sum of integers a and b
 */
int addInts(int a, int b);

/**
 * multiply two integers
 * 
 * @params: 
 *   a:        first integer a
 *   b:        second integer b
 * @return:    The product of integers a and b
 */
int multiplyInts(int a, int b);

/**
 * divide two float numbers
 * 
 * @params: 
 *   a:        the dividend float number
 *   b:        the divisor float number
 * @return:    On success, the quotient of a divided by b is returned. 
 *             If the method fails because of a division by 0, it returns null.
 */
float divideFloats(float a, float b);

/**
 * sleep calculator
 * 
 * @params: 
 *   x:        the time to sleep (seconds)
 * @return:    After the process has slept x seconds, it returns 0
 */
int sleepFor(int x);

/**
 * compute the factorial of a number
 * 
 * @params: 
 *   x:        the number to compute the factorial value
 * @return:    The factorial number of x if x > 0. Returns -1, if x is negative.
 */
uint64_t factorial(int x);