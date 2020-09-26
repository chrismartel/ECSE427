#include "a1_lib.h"

int isOperationValid(char *operator);
int isCommandValid(char *command);

int main()
{

  char test1[] = "add 1 2";
  char test2[] = "multiply 5 6";
  char test3[] = "divide 4.5 6";
  char test4[] = "sleep 5";
  char test5[] = "factorial 6";
  char test6[] = "factorial 6 8";
  char test7[] = "sleep 6 7";

  printf("%d\n", isCommandValid(test1));
  printf("%d\n", isCommandValid(test2));
  printf("%d\n", isCommandValid(test3));
  printf("%d\n", isCommandValid(test4));
  printf("%d\n", isCommandValid(test5));
  printf("%d\n", isCommandValid(test6));
  printf("%d\n", isCommandValid(test7));

  return 0;
}
/**
 * Method verifying if a string is a valid operator for the calculator
 * @params:
 *  operator:   pointer to the string to verify
 * 
 * @return:     0 if the string is a valid operator
 *              -1 if the string is not a valid operator
*/
int isOperationValid(char *operator)
{
  if ((strcmp(operator, "add") == 0) || (strcmp(operator, "multiply") == 0) || (strcmp(operator, "divide") == 0) || (strcmp(operator, "sleep") == 0) || (strcmp(operator, "factorial") == 0))
  {
    return 0;
  }
  else
  {
    return -1;
  }
}

/**
 * Method verifying if the calculator command is valid
 * @params:
 *  command:   pointer to the command string
 * 
 * @return:     0 if the command is valid
 *              -1 if the command is not valid
*/
int isCommandValid(char command[])
{
  char *operation = NULL;
  char *op1 = NULL;
  char *op2 = NULL;

  char *token = strtok(command, " ");
  int index = 0;

  // parse the command into tokens
  while (token != NULL)
  {
    printf("%s\n", token);
    if (index == 0)
    {
      operation = token;
    }

    else if (index == 1)
    {
      op1 = token;
    }
    else if (index == 2)
    {
      op2 = token;
    }
    index++;
    token = strtok(NULL, " ");
  }
  if (index >= 4)
  {
    printf("Invalid Command: too many operators\n");
    return -1;
  }

  // check if operation is valid
  if (isOperationValid(operation) == 0)
  {
    printf("Valid operation\n");
  }
  else
  {
    printf("Invalid command: invalid operation\n");
    return -1;
  }

  // add/multiply validity
  if (strcmp(operation, "add") == 0 || strcmp(operation, "multiply") == 0)
  {
    // check if correct number of operators
    if (index > 3)
    {
      printf("Invalid command: wrong number of operators for this command\n");
      return -1;
    }
    // check that op 1 and op 2 are integers
    if (atoi(op1) == 0 || atoi(op2) == 0)
    {
      printf("Invalid command: operators must be ints\n");
      return -1;
    }
  }

  // divide validity
  else if (strcmp(operation, "divide") == 0)
  {
    // check if correct number of operators
    if (index > 3)
    {
      printf("Invalid command: wrong number of operators for this command\n");
      return -1;
    }
    // check that op 1 and op 2 are floats
    if (atof(op1) == 0 || atof(op2) == 0)
    {
      printf("Invalid command: operators must be floats\n");
      return -1;
    }
  }

  // sleep/factorial validity
  else if (strcmp(operation, "sleep") == 0 || strcmp(operation, "factorial") == 0)
  {
    // check if correct number of operators
    if (index > 2)
    {
      printf("Invalid command: wrong number of operators for this command\n");
      return -1;
    }
    // check that op 1 is integer
    if (atoi(op1) == 0)
    {
      printf("Invalid command: operator must be int\n");
      return -1;
    }
  }
  return 0;
}
