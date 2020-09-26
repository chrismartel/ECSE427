#include "a1_lib.h"
#include "calculator.h"

#define MAX_NB_CLIENTS 5
#define BUFSIZE 1024

int isCommandValid(char *command);
int isOperationValid(char *operator);

int main()
{

  // valid commands
  const char *add_cmd = "add";
  const char *mul_cmd = "multiply";
  const char *sleep_cmd = "sleep";
  const char *fact_cmd = "factorial";
  const char *div_cmd = "divide";

  // number of frontend processes running
  int running = 0;

  // server response
  char response[BUFSIZE];

  // the file descriptor associated with the server socket
  int sockfd;

  // the file descriptor of the client connection
  int clientfd;

  // The message queue
  char msg[BUFSIZE];

  // set up the server socket
  if (create_server("0.0.0.0", 10000, &sockfd) < 0)
  {
    fprintf(stderr, "error in creating server\n");
    return -1;
  }
  printf("server created successfully\n\n");

  // accept a client connection on a server socket
  printf("waiting for client connection...\n\n");
  if (accept_connection(sockfd, &clientfd) < 0)
  {
    fprintf(stderr, "error in accepting connection from client\n");
    return -1;
  }
  printf("accepted client connection successfully!\n\n");

  while (strcmp(msg, "exit\n"))
  {
    memset(msg, 0, sizeof(msg));
    memset(response, 0, sizeof(response));

    ssize_t byte_count = recv_message(clientfd, msg, BUFSIZE);
    if (byte_count <= 0)
    {
      printf("error in receiving message...\n");
      break;
    }

    // check validity of command

    if (isCommandValid(msg) != 0)
    {
      strncpy(response, "Invalid Command", BUFSIZE);
    }
    // parse command
    else
    {
      char msg_cpy[BUFSIZE];
      strncpy(msg_cpy, msg, BUFSIZE);
      char *operation = NULL;
      char *op1 = NULL;
      char *op2 = NULL;

      char *token = strtok(msg_cpy, " ");
      int index = 0;
      // parse the command into tokens
      while (token != NULL)
      {
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

      if (strcmp(operation, add_cmd) == 0)
      {
        int isum = addInts(atoi(op1), atoi(op2));
        sprintf(response, "%d", isum);
      }
      else if (strcmp(operation, mul_cmd) == 0)
      {
        int imul = multiplyInts(atoi(op1), atoi(op2));
        sprintf(response, "%d", imul);
      }
      else if (strcmp(operation, div_cmd) == 0)
      {
        float fdiv = divideFloats(atof(op1), atof(op2));
        if (fdiv == 0)
        {
           sprintf(response, "division by 0, undefined result");         
        }
        else
        {
          sprintf(response, "%f", fdiv);
        }
      }
      else if (strcmp(operation, sleep_cmd) == 0)
      {
        sleepFor(atoi(op1));
        sprintf(response, "Slept for %d seconds", atoi(op1));
      }
      else if (strcmp(operation, fact_cmd) == 0)
      {
        int ifac = factorial(atoi(op1));
        sprintf(response, "%d", ifac);
      }
    }

    send_message(clientfd, response, BUFSIZE);
  }
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
  char command_cpy[BUFSIZE];
  strncpy(command_cpy, command, BUFSIZE);

  char *token = strtok(command_cpy, " ");
  int index = 0;

  // parse the command into tokens
  while (token != NULL)
  {
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
  if (isOperationValid(operation) != 0)
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
    if ((atoi(op1) == 0 && atoi(op1) != 0) || (atoi(op2) == 0 && atoi(op2) != 0))
    {
      printf("%s\n", op2);
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
    if ((atof(op1) == 0 && atof(op1) != 0) || (atof(op2) == 0 && atof(op2) != 0))
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
    if (atoi(op1) == 0 && atoi(op1) != 0)
    {
      printf("Invalid command: operator must be int\n");
      return -1;
    }
  }
  return 0;
}