#include "mystringlib.h"

// left trim
char *ltrim(char *s)
{
  while (isspace(*s))
    s++;
  return s;
}

// right trim
char *rtrim(char *s)
{
  char *back = s + strlen(s);
  while (isspace(*--back))
    ;
  *(back + 1) = '\0';
  return s;
}

// full trim
char *trim(char *s)
{
  return rtrim(ltrim(s));
}