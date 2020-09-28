#include "a1_lib.h"

int isCommandValid(char *command);
int isOperatorValid(char *operator);
void *create_shared_memory(size_t size);

int main()
{

  /* list of valid commands */
  const char *add_cmd = "add";
  const char *mul_cmd = "multiply";
  const char *sleep_cmd = "sleep";
  const char *fact_cmd = "factorial";
  const char *div_cmd = "divide";
  const char *exit_cmd = "exit";
  const char *sd_cmd = "shutdown";

  /* next process id to fork*/
  pid_t nextpid;

  /* file descriptor of the server socket */
  int sockfd;

  // SETUP SHARED MEMORY SEGMENTS //

  /* children process ids*/
  pid_t *children_pids_shm = create_shared_memory(PIDSIZE * MAX_NB_CLIENTS);

  /* snumber of children processes currently running */
  int *nclients_shm = create_shared_memory(INTSIZE);

  /* initialize number of clients currently running */
  int start_index = 0;
  memcpy(nclients_shm, &start_index, INTSIZE);

  // SETUP SERVER SOCKET //

  if (create_server("127.0.0.5", 10000, &sockfd) < 0)
  {
    fprintf(stderr, "error in creating server\n");
    return -1;
  }

  while (1)
  {
    // SETUP CLIENT CONNECTION //

    /* client message buffer */
    char msg[BUFSIZE];

    /* server response buffer */
    char response[BUFSIZE];

    /* file descriptor of client connection */
    int frontendfd;

    int socket;
    printf("waiting for client connection...\n\n");
    if (socket = accept_connection(sockfd, &frontendfd) < 0)
    {
      fprintf(stderr, "error in accepting connection from client\n");
      exit(-1);
    }

    // CHECK NUMBER OF CLIENTS RUNNING ON SERVER* //

    /*server unavailable*/
    if (*nclients_shm >= MAX_NB_CLIENTS)
    {
      sprintf(response, "busy");
      send_message(frontendfd, response, BUFSIZE);
    }

    /* server available*/
    else
    {
      /* increment number of clients running on server*/
      *nclients_shm = *nclients_shm + 1;
      //printf("accepted client connection\nCurrently serving %d clients\n", *nclients_shm);

      /* fork a child process */
      nextpid = fork();

      if (nextpid < 0)
      {
        fprintf(stderr, "Fork Failed");
        exit(-1);
      }

      // EXECUTION OF CHILD PROCESS //
      else if (nextpid == 0)
      {
        /* store child id in shared memory segment*/
        memcpy((children_pids_shm + (*(nclients_shm)*PIDSIZE)), &nextpid, PIDSIZE);

        // RECEIVE FRONTEND COMMANDS //
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

          // CHECK VALIDITY OF COMMAND //

          if (isCommandValid(msg) != 0)
          {
            strncpy(response, "NOT_FOUND", BUFSIZE);
          }

          else
          {
            // PARSE COMMAND //

            char msg_cpy[BUFSIZE];
            strncpy(msg_cpy, msg, BUFSIZE);

            /* operator */
            char *operator= NULL;

            /* first operand */
            char *op1 = "0";

            /* second operand*/
            char *op2 = "0";

            /* split command into tokens*/
            char *param = strtok(msg_cpy, " ");
            int n = 0;
            while (param != NULL)
            {
              n++;
              if (n == 1)
              {
                operator= trim(param);
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
            // ADD COMMAND //
            if (strcmp(operator, add_cmd) == 0)
            {
              int isum = addInts(atoi(op1), atoi(op2));
              sprintf(response, "%d", isum);
            }
            // MULTIPLICATION COMMAND //
            else if (strcmp(operator, mul_cmd) == 0)
            {
              int imul = multiplyInts(atoi(op1), atoi(op2));
              sprintf(response, "%d", imul);
            }
            // DIVISION COMMAND //
            else if (strcmp(operator, div_cmd) == 0)
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
            // SLEEP COMMAND //
            else if (strcmp(operator, sleep_cmd) == 0)
            {
              sleepFor(atoi(op1));
              sprintf(response, "Slept for %d seconds", atoi(op1));
            }
            // FACTORIAL COMMAND
            else if (strcmp(operator, fact_cmd) == 0)
            {
              int ifac = factorial(atoi(op1));
              sprintf(response, "%d", ifac);
            }
            // EXIT COMMAND
            else if (strcmp(operator, exit_cmd) == 0)
            {
              sprintf(response, "exit");
              send_message(frontendfd, response, BUFSIZE);
              close(socket);
              close(sockfd);
              /* decrement number of running clients */
              *nclients_shm = *nclients_shm - 1;
              exit(0);
            }
            // SHUTDOWN COMMAND
            else if (strcmp(operator, sd_cmd) == 0)
            {
              sprintf(response, "shutdown");
              send_message(frontendfd, response, BUFSIZE);
              close(socket);
              close(sockfd);
              /* kill all children processes*/
              for (int i = 0; i < *nclients_shm; i++)
              {
                int status;
                pid_t cid = *(children_pids_shm + i * PIDSIZE);
                if (cid != getpid())
                {
                  kill(cid, SIGTERM);
                }
              }
              /* kill parent process*/
              kill(getppid(), SIGTERM);
              /* kill current process*/
              kill(getpid(), SIGTERM);
            }
          }

          send_message(frontendfd, response, BUFSIZE);
        }
      }
      // EXECUTION OF PARENT PROCESS //
      close(socket);
      /* start new while loop --> listen for new connection */
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
int isOperatorValid(char *operator)
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
  char *operator= NULL;
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
      operator= trim(param);
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

  // check if operator is valid
  if (isOperatorValid(operator) != 0)
  {
    printf("Invalid command: invalid operator\n");
    return -1;
  }

  // add/multiply validity
  if (strcmp(operator, "add") == 0 || strcmp(operator, "multiply") == 0)
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
  else if (strcmp(operator, "divide") == 0)
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
  else if (strcmp(operator, "sleep") == 0 || strcmp(operator, "factorial") == 0)
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

/**
 * Method creating a shared memory segment
 * @params:
 *  size:   size of desired shared memory segment
 * 
 * @return:     pointer to shared memory segment
*/
void *create_shared_memory(size_t size)
{
  int protection = PROT_READ | PROT_WRITE;
  int visibility = MAP_SHARED | MAP_ANONYMOUS;
  return mmap(NULL, size, protection, visibility, -1, 0);
}

pid_t *find_next_available_process(pid_t *pids)
{
}
