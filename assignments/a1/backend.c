#include "a1_lib.h"
#include "calculator.h"
#include "mystringlib.h"
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#define MAX_NB_CLIENTS 3
#define BUFSIZE 1024
#define SLEEPTIME 2
#define SHMSIZE 4

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

  // number of client processes running
  int running = 0;

  // id of parent process
  pid_t parent_pid = getpid();

  // the file descriptor associated with the server socket
  int sockfd;

  pid_t children_pids[MAX_NB_CLIENTS];

  // set up the server socket
  if (create_server("127.0.0.30", 10000, &sockfd) < 0)
  {
    fprintf(stderr, "error in creating server\n");
    return -1;
  }

  while (1)
  {
    if (running < MAX_NB_CLIENTS)
    {
      // The message buffer
      char msg[BUFSIZE];

      // server response buffer
      char response[BUFSIZE];

      // the file descriptor of the client connection
      int frontendfd;

      // accept a client connection on a server socket
      printf("waiting for client connection...\n\n");
      int socket;
      if (socket = accept_connection(sockfd, &frontendfd) < 0)
      {
        fprintf(stderr, "error in accepting connection from client\n");
        return -1;
      }
      printf("accepted client connection\n\n");
      // fork a child process
      children_pids[running] = fork();

      /* ERROR IN FORKING */
      if (children_pids[running] < 0)
      {
        fprintf(stderr, "Fork Failed");
        return 1;
      }

      /* EXECUTION OF CHILD PROCESS */
      else if (children_pids[running] == 0)
      {
        while (1)
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
              // child exits normally
              close(socket);
              close(sockfd);
              exit(0);
            }
            else if (strcmp(operation, sd_cmd) == 0)
            {
              sprintf(response, "shutdown");
              send_message(frontendfd, response, BUFSIZE);
              close(socket);
              close(sockfd);
              kill(parent_pid, SIGTERM);
            }
          }

          send_message(frontendfd, response, BUFSIZE);
        }
      }

      /* EXECUTION OF PARENT PROCESS */
      else
      {
        close(socket);
        // increment number of running processes
        running++;
        printf("Incremented running to: %d\n", running);
      }
    }
    // server full
    else
    {
      pid_t check_pid;
      int status;
      int available_index = -1;
      int terminated_processes = 0;
      for (int i = 0; i < running; i++)
      {
        check_pid = waitpid(children_pids[i], &status, WNOHANG);
        if (check_pid == children_pids[i])
        {
          available_index = i;
          break;
        }
      }
      if (available_index != -1)
      {
        // swap processes Ids
        pid_t temp = children_pids[MAX_NB_CLIENTS - 1];
        children_pids[MAX_NB_CLIENTS - 1] = children_pids[available_index];
        children_pids[available_index] = temp;
        running--;
      }
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
