#include "a1_lib.h"
#include "calculator.h"
#include "mystringlib.h"

#define MAX_NB_CLIENTS 5
#define BUFSIZE 1024

int isCommandValid(char *command);
int isOperationValid(char *operator);
char *ltrim(char *s);
char *rtrim(char *s);
char *trim(char *s);

int main()
{

  // valid commands
  const char *add_cmd = "add";
  const char *mul_cmd = "multiply";
  const char *sleep_cmd = "sleep";
  const char *fact_cmd = "factorial";
  const char *div_cmd = "divide";
  const char *exit_cmd = "exit";
  const char *sd_cmd = "shutdown";

  // number of frontend processes running
  int running = 0;

  // the file descriptor associated with the server socket
  int sockfd;

  // set up the server socket
  if (create_server("127.0.0.7", 10000, &sockfd) < 0)
  {
    fprintf(stderr, "error in creating server\n");
    return -1;
  }
  printf("server created successfully\n\n");

  while (running < 5)
  {
    // server response
    char response[BUFSIZE];

    // the file descriptor of the client connection
    int frontendfd;

    // The message queue
    char msg[BUFSIZE];

    // accept a client connection on a server socket
    printf("waiting for client connection...\n\n");
    if (accept_connection(sockfd, &frontendfd) < 0)
    {
      fprintf(stderr, "error in accepting connection from client\n");
      return -1;
    }
    printf("accepted client connection successfully!\n\n");

    pid_t pid;

    // fork a child process
    pid = fork();

    // error
    if (pid < 0)
    {
      fprintf(stderr, "Fork Failed");
      return 1;
    }
    // child process
    else if (pid == 0)
    {
      /* EXECUTION OF CHILD PROCESS */

      while (strcmp(msg, "exit\n"))
      {
        memset(msg, 0, sizeof(msg));
        memset(response, 0, sizeof(response));

        ssize_t byte_count = recv_message(frontendfd, msg, BUFSIZE);
        if (byte_count <= 0)
        {
          printf("error in receiving message...\n");
          break;
        }

        // check validity of command

        if (isCommandValid(msg) != 0)
        {
          strncpy(response, "NOT_FOUND", BUFSIZE);
        }
        // parse command
        else
        {
          char msg_cpy[BUFSIZE];
          strncpy(msg_cpy, msg, BUFSIZE);
          char *operation = NULL;
          char *op1 = "0";
          char *op2 = "0";

          char *param = strtok(msg_cpy, " ");
          // number of parameters
          int n = 0;
          // parse the command into parameters

          while (param != NULL)
          {
            n++;
            if (n == 1)
            {
              operation = trim(param);
            }
            else if (n == 2)
            {
              op1 = trim(param);
            }
            else if (n == 3)
            {
              op2 = trim(param);
            }
            param = strtok(NULL, " ");
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
              sprintf(response, "Error: Division by 0");
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
          else if (strcmp(operation, exit_cmd) == 0)
          {
            sprintf(response, "exit");
            send_message(frontendfd, response, BUFSIZE);
            printf("Child exits... ");
            // child exits
            exit(0);
          }
          else if (strcmp(operation, sd_cmd) == 0)
          {
            printf("Shutting down....");
            return 0;
          }
        }

        send_message(frontendfd, response, BUFSIZE);
      }
    }
    /* EXECUTION OF PARENT PROCESS */
    else
    {
      running++;
    }
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
  if ((strcmp(operator, "add") == 0) || (strcmp(operator, "shutdown") == 0) || (strcmp(operator, "exit") == 0) || (strcmp(operator, "multiply") == 0) || (strcmp(operator, "divide") == 0) || (strcmp(operator, "sleep") == 0) || (strcmp(operator, "factorial") == 0))
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
  char *op1 = "0";
  char *op2 = "0";
  char command_cpy[BUFSIZE];
  strncpy(command_cpy, command, BUFSIZE);

  char *param = strtok(command_cpy, " ");
  int n = 0;

  // parse the command into tokens
  while (param != NULL)
  {
    n++;
    if (n == 1)
    {
      operation = trim(param);
    }

    else if (n == 2)
    {
      op1 = trim(param);
    }
    else if (n == 3)
    {
      op2 = trim(param);
    }

    param = strtok(NULL, " ");
  }

  if (n >= 4)
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
    if (n > 3)
    {
      printf("Invalid command: wrong number of operators for this command\n");
      return -1;
    }
    // check that op 1 and op 2 are integers
    if ((atoi(op1) == 0 && atoi(op1) != 0) || (atoi(op2) == 0 && atoi(op2) != 0))
    {
      printf("Invalid command: operators must be ints\n");

      return -1;
    }
  }

  // divide validity
  else if (strcmp(operation, "divide") == 0)
  {
    // check if correct number of operators
    if (n > 3)
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
    if (n > 2)
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


