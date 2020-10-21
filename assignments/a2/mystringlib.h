#include "ctype.h"
/**
 * left trim of string s
 * @params:
 *  s:   pointer to string we want to left trim
 * 
 * @return:   pointer to beginning of left-trimmed string
 *            
*/
char *ltrim(char *s);

/**
 * right trim of string s
 * @params:
 *  s:   pointer to the string we want to right trim
 * 
 * @return:   pointer to beginning of right-trimmed string
 *            
*/
char *rtrim(char *s);

/**
 * full trim of string s
 * @params:
 *  s:   pointer to the string we want to trim
 * 
 * @return:   pointer to beginning of trimmed string
 *            
*/
char *trim(char *s);