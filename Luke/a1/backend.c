#include "a1_lib.h"

void *create_shared_memory(size_t size); //resolve implicit declaration error
int isValid(char *command);
uint64_t factorial(int x);
float divideFloats(float a, float b);

/** Functions for calculating **/

/**
 * Function adding two integers.
 * 
 * @params: 
 *  a:      Integer a
 *  b:      Integer b
 * @return: Sum of both integers
 * 
 */

int addInts(int a, int b)
{
    int sum = a + b;
    return (sum);
}

/**
 * Function multiplying two integers.
 * 
 * @params: 
 *  a:      Integer a
 *  b:      Integer b
 * @return: Product of both integers
 * 
 */

int multiplyInts(int a, int b)
{
    int product = a * b;
    return (product);
}

/**
 * Function dividing two float numbers.
 * 
 * @params: 
 *  a:      Float number a
 *  b:      Float number b
 * @return: On success, a quotient of a divided by b.
 *          If the function fails, returns -1 (exit with a fail).
 * 
 */

float divideFloats(float a, float b)
{

    if (b == 0.0)
    {
        return -1;
    }
    else
    {
        float quotient = a / b;
        return (quotient);
    }
}

/**
 * Makes the calculator sleep for x seconds.
 * 
 * @params: 
 *  x:      Integer x for amount of time to sleep.
 * @return: Return 0 (exit with success).
 * 
 */

int sleepTime(int x)
{
    sleep(x);
    return 0;
}

/**
 * Function that returns the factorial of an integer
 * 
 * @params: 
 *  x:      Integer number x
 * @return: On success, returns factorial of integer x.
 *          If the function fails because of a negative argument, returns -1 (exit with a fail).
 *          If the function is out of bound it exits with return -2.
 * 
 */
uint64_t factorial(int x)
{
    if (x < 0)
    {
        return -1;
    }
    else if (x == 0)
    {
        return 1;
    }
    else if (x > 20)
    {
        return -2;
    }
    else
    {
        int factor = 1;
        for (int i = x; i >= 1; i--)
        {
            factor = factor * i;
        }
        return factor;
    }
    return 0;
}

void *create_shared_memory(size_t size)
{
    int buf_protect = PROT_READ | PROT_WRITE;
    int buf_visible = MAP_SHARED | MAP_ANONYMOUS;
    return mmap(NULL, size, buf_protect, buf_visible, -1, 0);
}

/**
 * Function that determines if a command is valid 
 * 
 * @params: 
 *  command:      points to command string to be verified
 * @return: On success, return 0.
 *          If the function returns -1, invalid command.
 * 
 */
int isValid(char *command)
{
    if (strcmp(command, "add") == 0 || strcmp(command, "multiply") == 0 || strcmp(command, "dvide") == 0 || strcmp(command, "sleep") == 0 || strcmp(command, "factorial") == 0 || strcmp(command, "exit") == 0 || strcmp(command, "shutdown") == 0)
    {
        printf("valid command\n");
        return 0;
    }
    else
    {
        printf("Error: invalid command\n");
        return -1;
    }
}

//*******************************//

int main(int argc, char *argv[])
{

    int sockfd;        // file descriptor for socket
    pid_t process_pid; //pid of the process to fork next

    /* Data for server */
    const char *host = argv[1];
    const char *portarg = argv[2];
    const int port = atoi(portarg);

    /* Data for shared memory segments */
    int *nbofclients_shmseg = create_shared_memory(SHMSEGSIZE);
    pid_t *pid_shmseg = create_shared_memory(PIDLENGTH * CLIENTMAX);

    /* Commands that the user cand enter */
    const char *addition = "add";
    const char *multiplication = "multiply";
    const char *division = "divide";
    const char *sleeping = "sleep";
    const char *factoring = "factorial";
    const char *exiting = "exit";
    const char *shutting_down = "shutdown";

    /* To pass structs */
    struct Request
    {
        char commandoperation[COMMANDLENGTH];
        char parameters[NUMBEROFARGUMENTS][ARGUMENTLENGTH];
        int numberOfArgs;
    };

    // Initialize the counter for the number of clients in memory
    int clientCounter = 0;
    memcpy(nbofclients_shmseg, &clientCounter, SHMSEGSIZE);

    if ((create_server(host, port, &sockfd) < 0))
    {
        fprintf(stderr, "Exit: The server failed to create.\n");
        return -1;
    }
    printf("Server connection successful\n");

    while (1)
    {

        int socket;
        int clientfd;           //file descriptor for client
        char request[BUFSIZE];  //for user
        char response[BUFSIZE]; //for server to send output buffer message strings to frontend
        char error[BUFSIZE];    //for passing error messages

        if (socket = accept_connection(sockfd, &clientfd) < 0)
        {
            fprintf(stderr, "Error: The client failed to connect to the server.\n");
            exit(-1);
        }

        /** 5 users already connected check **/

        if (*nbofclients_shmseg >= CLIENTMAX)
        {
            sprintf(response, "unavailable");
            send_message(clientfd, response, BUFSIZE);
        }

        /** then a server is free to use by a user **/

        else
        {

            *nbofclients_shmseg += 1; //1 more user in server
            //sprintf(response, "server is free");
            //send_message(clientfd,response,BUFSIZE);

            process_pid = fork();
            if (process_pid < 0)
            {
                printf("Error: Forking did not work.");
                exit(-1);
            }

            /** if fork returns 0 value, it is the value returned to the newly
            * created child process, meaning the forking was successful
            */

            else if (process_pid == 0)
            {
                memcpy((pid_shmseg + (*(nbofclients_shmseg)*PIDLENGTH)), &process_pid, PIDLENGTH);

                while (1)
                {
                    //reset the input/output buffers
                    memset(request, 0, sizeof(request));
                    memset(response, 0, sizeof(response));
                    ssize_t byte_count = recv_message(clientfd, request, BUFSIZE); //receiving a request from the client (frontend)
                    if (byte_count <= 0)
                    {
                        break;
                    }

                    // To pass messages as structs
                    struct Request *request_struct = (struct Request *)request;
                    printf("command operation: %s\n", request_struct->commandoperation);
                    printf("param 1: %s\n", request_struct->parameters[0]);
                    printf("param2: %s\n", request_struct->parameters[1]);

                    int commandReceived = isValid(request_struct->commandoperation);
                    if (commandReceived == -1)
                    {
                        sprintf(response, "Error: command NOT_FOUND");
                        send_message(clientfd, response, BUFSIZE);
                    }
                    else
                    {

                        /** Data variables for command **/
                        char *command = request_struct->commandoperation;
                        char *first_argument = request_struct->parameters[0];
                        char *second_argument = request_struct->parameters[1];

                        int numberOfArgs = request_struct->numberOfArgs;

                        if (strcmp(command, addition) == 0)
                        {
                            if (numberOfArgs != 3)
                            {
                                sprintf(response, "Error: The command takes 2 arguments.");
                                send_message(clientfd, response, BUFSIZE);
                            }
                            else
                            {
                                int operand1 = atoi(first_argument);
                                int operand2 = atoi(second_argument);
                                if (((operand1 == 0) && (strcmp(first_argument, "0") != 0)) || ((operand2 == 0) && (strcmp(second_argument, "0") != 0)))
                                {
                                    sprintf(response, "Error: The arguments are not integers.");
                                    send_message(clientfd, response, BUFSIZE);
                                }
                                else
                                {
                                    int operandsum = addInts(operand1, operand2);
                                    sprintf(response, "%d", operandsum);
                                    send_message(clientfd, response, BUFSIZE);
                                }
                            }
                        }
                        else if (strcmp(command, multiplication) == 0)
                        {
                            if (numberOfArgs != 3)
                            {
                                sprintf(response, "Error: The command takes 2 arguments.");
                                send_message(clientfd, response, BUFSIZE);
                            }
                            else
                            {
                                int operand1 = atoi(first_argument);
                                int operand2 = atoi(second_argument);
                                if (((operand1 == 0) && (strcmp(first_argument, "0") != 0)) || ((operand2 == 0) && (strcmp(second_argument, "0") != 0)))
                                {
                                    sprintf(response, "Error: The arguments are not integers.");
                                    send_message(clientfd, response, BUFSIZE);
                                }
                                else
                                {
                                    int operandproduct = multiplyInts(operand1, operand2);
                                    sprintf(response, "%d", operandproduct);
                                    send_message(clientfd, response, BUFSIZE);
                                }
                            }
                        }
                        else if (strcmp(command, division) == 0)
                        {
                            if (numberOfArgs != 3)
                            {
                                sprintf(response, "Error: The command takes 2 arguments.");
                                send_message(clientfd, response, BUFSIZE);
                            }
                            else
                            {
                                float operand1 = atof(first_argument);
                                float operand2 = atof(second_argument);
                                if (((operand1 == 0) && (strcmp(first_argument, "0") != 0)) || ((operand2 == 0) && (strcmp(second_argument, "0") != 0)))
                                {
                                    sprintf(response, "Error: The arguments are not floats.");
                                    send_message(clientfd, response, BUFSIZE);
                                }
                                else
                                {
                                    float operandquotient = divideFloats(operand1, operand2);
                                    if (operandquotient == -1)
                                    {
                                        sprintf(response, "Error: Dividing by zero is impossible.\n");
                                    }
                                    else
                                    {
                                        sprintf(response, "%f", operandquotient);
                                    }
                                    send_message(clientfd, response, BUFSIZE);
                                }
                            }
                        }
                        else if (strcmp(command, sleeping) == 0)
                        {
                            if (numberOfArgs != 2)
                            {
                                sprintf(response, "Error: The command takes 1 arguments.");
                                send_message(clientfd, response, BUFSIZE);
                            }
                            else
                            {
                                int operand1 = atoi(first_argument);
                                if (((operand1 == 0) && (strcmp(first_argument, "0") != 0)))
                                {
                                    sprintf(response, "Error: The argument is not an integer.");
                                    send_message(clientfd, response, BUFSIZE);
                                }
                                else
                                {
                                    sleepTime(operand1);
                                    sprintf(response, "Sleeping");
                                    send_message(clientfd, response, BUFSIZE);
                                }
                            }
                        }
                        else if (strcmp(command, factoring) == 0)
                        {
                            if (numberOfArgs != 2)
                            {
                                sprintf(response, "Error: The command takes 1 argument.");
                                send_message(clientfd, response, BUFSIZE);
                            }
                            else
                            {
                                int operand1 = atoi(first_argument);
                                if (((operand1 == 0) && (strcmp(first_argument, "0") != 0)))
                                {
                                    sprintf(response, "Error: The arguments is not an integer.");
                                    send_message(clientfd, response, BUFSIZE);
                                }
                                else
                                {
                                    int operandfactorial = factorial(operand1);
                                    if (operandfactorial == -1)
                                    {
                                        sprintf(response, "Error: The operand cannot be negative.\n");
                                    }
                                    else if (operandfactorial == -2)
                                    {
                                        sprintf(response, "Error: The operand cannot be greater than 20.\n");
                                    }
                                    else
                                    {
                                        sprintf(response, "%d", operandfactorial);
                                    }
                                    send_message(clientfd, response, BUFSIZE);
                                }
                            }
                        }
                        else if (strcmp(command, exiting) == 0)
                        {
                            if (numberOfArgs != 1)
                            {
                                sprintf(response, "Error: The command takes 0 arguments.");
                                send_message(clientfd, response, BUFSIZE);
                            }
                            else
                            {
                                sprintf(response, "exiting");
                                send_message(clientfd, response, BUFSIZE);
                                close(socket);
                                close(clientfd);
                                *nbofclients_shmseg -= 1;
                                exit(0);
                            }
                        }
                        else if (strcmp(command, shutting_down) == 0)
                        {
                            if (numberOfArgs != 1)
                            {
                                sprintf(response, "Error: The command takes 0 arguments.");
                                send_message(clientfd, response, BUFSIZE);
                            }
                            else
                            {
                                sprintf(response, "Shuttingdown");
                                send_message(clientfd, response, BUFSIZE);
                                close(socket);
                                close(clientfd);
                                for (int i = *nbofclients_shmseg; i >= 1; i--)
                                {
                                    pid_t children = *(pid_shmseg + i * PIDLENGTH);
                                    if (children != getpid())
                                    {
                                        kill(children, SIGTERM);
                                    }
                                }
                                kill(getppid(), SIGTERM);
                                kill(getpid(), SIGTERM);
                            }
                        }
                    }
                }
            }
            close(socket);
        }
    }
}