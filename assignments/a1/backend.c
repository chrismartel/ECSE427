#include "a1_lib.h"

/* define command struct */
struct Message
{
  char command[COMMANDSIZE];
  char parameters[NBPARAMS][PARAMSIZE];
  int nbargs;
};

/* define process struct */
struct Process
{
  pid_t process_id;
  int available;
};

/* declare functions*/
int findNextProcessAvailable(struct Process *p);
int isCommandValid(char *command);
int isOperatorValid(char *operator);
void *create_shared_memory(size_t size);

int main(int argc, char *argv[])
{

  /* list of valid commands */
  const char *add_cmd = "add";
  const char *mul_cmd = "multiply";
  const char *sleep_cmd = "sleep";
  const char *fact_cmd = "factorial";
  const char *div_cmd = "divide";
  const char *exit_cmd = "exit";
  const char *sd_cmd = "shutdown";

  /* host ip */
  const char *host_ip = argv[1];

  /* host port */
  const char *arg2 = argv[2];
  const int host_port = atoi(arg2);

  /* next process id to fork*/
  pid_t nextpid;

  /* file descriptor of the server socket */
  int sockfd;

  // SETUP SHARED MEMORY SEGMENTS //

  /* keep track of if server is available (0) or not (1) */
  int *serverAvailable = create_shared_memory(INTSIZE);
  *serverAvailable = 0;

  /* keep track of next process index available*/
  int *nextProcessAvailable = create_shared_memory(INTSIZE);

  /* children process structures*/
  struct Process *children_shm = create_shared_memory(sizeof(struct Process) * MAX_NB_CLIENTS);

  /* initialize parameters of process structures*/
  for (int i = 0; i < MAX_NB_CLIENTS; i++)
  {
    struct Process *p = children_shm;
    for (int j = 0; j < i; j++)
    {
      p++;
    }
    p->available = 0;
    p->process_id = 0;
  }

  // SETUP SERVER SOCKET //

  if (create_server(host_ip, host_port, &sockfd) < 0)
  {
    fprintf(stderr, "error in creating server\n");
    return -1;
  }
  fflush(stdout);
  printf("Server listening on %s:%d\n", host_ip, host_port);

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
    if (*serverAvailable == 1)
    {
      /* indicate to frontend that server is not available*/
      sprintf(response, "busy");
      send_message(frontendfd, response, BUFSIZE);
    }

    /* server available*/
    else
    {
      /* indicate to frontend that server is available */
      sprintf(response, "available");
      send_message(frontendfd, response, BUFSIZE);

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
        /* get current child process id */
        nextpid = getpid();

        /* set pointer to shared memory process structures*/
        struct Process *p = (struct Process *)children_shm;

        /* point to next available process structure*/
        for (int i = 0; i < *nextProcessAvailable; i++)
        {
          p++;
        }
        /* set params of process structure*/
        p->process_id = nextpid;
        p->available = 1;

        /* reset process structures pointer*/
        p = (struct Process *)children_shm;

        /* update next available process index */
        *nextProcessAvailable = findNextProcessAvailable(p);

        /* update server status*/
        if (*nextProcessAvailable == -1)
        {
          *serverAvailable = 1;
        }
        else
        {
          *serverAvailable = 0;
        }

        // RECEIVE FRONTEND MESSAGES //
        while (1)
        {
          memset(msg, 0, sizeof(msg));
          memset(response, 0, sizeof(response));

          /* receive message from frontend*/
          ssize_t byte_count = recv_message(frontendfd, msg, sizeof(msg));
          if (byte_count <= 0)
          {
            printf("error in receiving message...\n");
            break;
          }

          /* convert message received into message struct */
          struct Message *msg_struct = (struct Message *)msg;

          // CHECK VALIDITY OF COMMAND //
          int ret = isCommandValid(msg_struct->command);

          /* invalid command */
          if (ret == -1)
          {
            char errorMessage[BUFSIZE];

            strncpy(errorMessage, msg, BUFSIZE);

            char *invalid_command = strtok(errorMessage, " ");
            sprintf(response, "Error: Command %c%s%c not found", '"', invalid_command, '"');
          }

          /* valid command */
          else
          {
            /* command */
            char *command = msg_struct->command;

            /* first operand */
            char *op1 = msg_struct->parameters[0];

            /* second operand*/
            char *op2 = msg_struct->parameters[1];

            /* number of arguments */
            int nbargs = msg_struct->nbargs;

            // ADD COMMAND //
            if (strcmp(command, add_cmd) == 0)
            {
              // check number of arguments
              if (nbargs != 3)
              {
                sprintf(response, "Error: Command %cadd%c takes 2 arguments\n", '"', '"');
              }
              else
              {
                int int1 = atoi(op1);
                int int2 = atoi(op2);
                // chekc that arguments are integers
                if (int1 == 0 && strcmp(op1, "0") != 0 || int2 == 0 && strcmp(op2, "0") != 0)
                {
                  sprintf(response, "arguments of command %cadd%c must be integers", '"', '"');
                }
                else
                {
                  int isum = addInts(int1, int2);
                  sprintf(response, "%d", isum);
                }
              }
            }
            // MULTIPLICATION COMMAND //
            else if (strcmp(command, mul_cmd) == 0)
            {
              // check number of arguments
              if (nbargs != 3)
              {
                sprintf(response, "Error: Command %cmultiply%c takes 2 arguments\n", '"', '"');
              }
              else
              {
                int int1 = atoi(op1);
                int int2 = atoi(op2);
                // check if arguments are integers
                if (int1 == 0 && strcmp(op1, "0") != 0 || int2 == 0 && strcmp(op2, "0") != 0)
                {
                  sprintf(response, "arguments of command %cmultiply%c must be integers", '"', '"');
                }
                else
                {
                  int imul = multiplyInts(atoi(op1), atoi(op2));
                  sprintf(response, "%d", imul);
                }
              }
            }
            // DIVISION COMMAND //
            else if (strcmp(command, div_cmd) == 0)
            {
              // check number of arguments
              if (nbargs != 3)
              {
                sprintf(response, "Error: Command %cdivide%c takes 2 arguments\n", '"', '"');
              }
              else
              {
                float f1 = atof(op1);
                float f2 = atof(op2);

                // check that arguments are floats
                if (f1 == 0 && strcmp(op1, "0") != 0 || f2 == 0 && strcmp(op2, "0") != 0)
                {
                  sprintf(response, "arguments of command %cdivide%c must be floats", '"', '"');
                }
                else
                {
                  float fdiv = divideFloats(f1, f2);
                  // check division by 0
                  if (fdiv == 99999999)
                  {
                    sprintf(response, "Error: Division by zero");
                  }
                  else
                  {
                    sprintf(response, "%f", fdiv);
                  }
                }
              }
            }
            // SLEEP COMMAND //
            else if (strcmp(command, sleep_cmd) == 0)
            {
              if (nbargs != 2)
              {
                sprintf(response, "Error: Command %csleep%c takes 1 argument\n", '"', '"');
              }
              else
              {
                int int1 = atoi(op1);

                // check if argument is integer
                if (int1 == 0 && strcmp(op1, "0") != 0)
                {
                  sprintf(response, "argument of command %csleep%c must be an integer", '"', '"');
                }
                else
                {
                  if (int1 < 0)
                  {
                    sprintf(response, "argument of command %csleep%c must be positive", '"', '"');
                  }
                  else
                  {
                    sleepFor(int1);
                    sprintf(response, "sleep");
                  }
                }
              }
            }
            // FACTORIAL COMMAND
            else if (strcmp(command, fact_cmd) == 0)
            {
              if (nbargs != 2)
              {
                sprintf(response, "Error: Command %cfactorial%c takes 1 argument\n", '"', '"');
              }
              else
              {
                uint64_t int1 = atoi(op1);
                // check if argument is integer
                if (int1 == 0 && strcmp(op1, "0") != 0)
                {
                  sprintf(response, "argument of command %cfactorial%c must be an integer", '"', '"');
                }
                else
                {
                  uint64_t ifac = factorial(int1);
                  if (ifac == -1)
                  {
                    sprintf(response, "Error: factorial argument must be greater than 0");
                  }
                  else if (ifac == -2)
                  {
                    sprintf(response, "Error: factorial argument must be less than 20");
                  }
                  else
                  {
                    sprintf(response, "%ld", ifac);
                  }
                }
              }
            }
            // EXIT COMMAND
            else if (strcmp(command, exit_cmd) == 0)
            {
              if (nbargs != 1)
              {
                sprintf(response, "Error: Command %cexit%c does not take any argument\n", '"', '"');
              }
              else
              {
                /* reset pointer to first process struct*/
                p = (struct Process *)children_shm;

                /* iterate through all children processes struct*/
                for (int i = 0; i < MAX_NB_CLIENTS; i++)
                {
                  /* if struct of current process*/
                  if (p->process_id == getpid())
                  {
                    /* struct is now available */
                    p->available = 0;

                    /* process id vacant */
                    p->process_id = 0;
                    break;
                  }
                  p++;
                }

                /* reset pointer to first process struct*/
                p = (struct Process *)children_shm;

                /* find next available process index */
                *nextProcessAvailable = findNextProcessAvailable(p);

                /* update server status*/
                if (*nextProcessAvailable == -1)
                {
                  *serverAvailable = 1;
                }
                else
                {
                  *serverAvailable = 0;
                }
                sprintf(response, "exit");
                send_message(frontendfd, response, BUFSIZE);
                close(socket);
                close(sockfd);
                exit(0);
              }
            }
            // SHUTDOWN COMMAND
            else if (strcmp(command, sd_cmd) == 0)
            {
              if (nbargs != 1)
              {
                sprintf(response, "Error: Command %cshutdown%c does not take any argument\n", '"', '"');
              }
              else
              {
                sprintf(response, "shutdown");
                send_message(frontendfd, response, BUFSIZE);
                close(socket);
                close(sockfd);

                /* kill all children processes*/

                /* reset pointer to first process struct*/
                p = (struct Process *)children_shm;

                for (int i = 0; i < MAX_NB_CLIENTS; i++)
                {
                  pid_t cid = p->process_id;
                  if (cid != getpid())
                  {
                    kill(cid, SIGTERM);
                  }
                  p++;
                }
                /* kill parent process*/
                kill(getppid(), SIGTERM);
                /* kill current process*/
                kill(getpid(), SIGTERM);
              }
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
int isCommandValid(char *operator)
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

/**
 * Method finding the next available process index through a list of process structure
 * @params:
 *  children:   pointer to beginning of array of Process structures
 * 
 * @return:     the index of the next available process structure. If no process available return -1
*/
int findNextProcessAvailable(struct Process *children)
{
  /* iterate through all process structures*/
  for (int i = 0; i < MAX_NB_CLIENTS; i++)
  {
    /* reset pointer */
    struct Process *p = children;

    for (int j = 0; j < i; j++)
    {
      p++;
    }
    int av = p->available;
    /* return index of available process */
    if (av == 0)
    {
      return i;
    }
  }
  /* no process available */
  return -1;
}
