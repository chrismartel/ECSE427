#include "a1_lib.h"

int isCommandValid(char *command);
int isOperatorValid(char *operator);
void *create_shared_memory(size_t size);

int main(int argc, char *argv[])
{

  /* define command struct */
  struct Message
  {
    char command[COMMANDSIZE];
    char parameters[NBPARAMS][PARAMSIZE];
  };

  /* list of valid commands */
  const char *add_cmd = "add";
  const char *mul_cmd = "multiply";
  const char *sleep_cmd = "sleep";
  const char *fact_cmd = "factorial";
  const char *div_cmd = "divide";
  const char *quit_cmd = "quit";
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

  /* children process ids*/
  pid_t *children_pids_shm = create_shared_memory(PIDSIZE * MAX_NB_CLIENTS);

  /* snumber of children processes currently running */
  int *nclients_shm = create_shared_memory(INTSIZE);

  /* initialize number of clients currently running */
  int start_index = 0;
  memcpy(nclients_shm, &start_index, INTSIZE);

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

            // ADD COMMAND //
            if (strcmp(command, add_cmd) == 0)
            {
              int isum = addInts(atoi(op1), atoi(op2));
              sprintf(response, "%d", isum);
            }
            // MULTIPLICATION COMMAND //
            else if (strcmp(command, mul_cmd) == 0)
            {
              int imul = multiplyInts(atoi(op1), atoi(op2));
              sprintf(response, "%d", imul);
            }
            // DIVISION COMMAND //
            else if (strcmp(command, div_cmd) == 0)
            {
              float fdiv = divideFloats(atof(op1), atof(op2));
              if (fdiv == 99999999)
              {
                sprintf(response, "Error: Division by zero");
              }
              else
              {
                sprintf(response, "%f", fdiv);
              }
            }
            // SLEEP COMMAND //
            else if (strcmp(command, sleep_cmd) == 0)
            {
              sleepFor(atoi(op1));
              sprintf(response, "sleep");
            }
            // FACTORIAL COMMAND
            else if (strcmp(command, fact_cmd) == 0)
            {
              int ifac = factorial(atoi(op1));
              sprintf(response, "%d", ifac);
            }
            // EXIT COMMAND
            else if (strcmp(command, quit_cmd) == 0)
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
            else if (strcmp(command, sd_cmd) == 0)
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
int isCommandValid(char *operator)
{
  if ((strcmp(operator, "add") == 0) || (strcmp(operator, "shutdown") == 0) || (strcmp(operator, "quit") == 0) || (strcmp(operator, "multiply") == 0) || (strcmp(operator, "divide") == 0) || (strcmp(operator, "sleep") == 0) || (strcmp(operator, "factorial") == 0))
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
