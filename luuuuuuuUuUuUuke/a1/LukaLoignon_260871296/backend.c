#include "a1_lib.h"

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

int addInts(int a, int b){
    int sum = a+b;
    return(sum);    
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

int multiplyInts(int a, int b){
    int product = a*b;
    return(product);
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

float divideFloats(float a, float b){
    //To handle dividion by 0 exception
    if(b == 0.0){
        return -1;
    }
    else{
        float quotient = a/b;
        return(quotient);
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

int sleepTime(int x){
    //To handle a negative sleep time exception
    if(x<0){
        return -1;
    }else
    {
        sleep(x);
        return 0;
    }
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
uint64_t factorial(int x){
    //To handle a negative factorial exception
    if(x<0){
        return -1;
    }
    else if(x == 0){
        return 1;
    }
    //To handle upper limit for factorial exception
    else if(x>20){
        return -2;
    }
    else{
        uint64_t factor = 1;
        for(int i=x; i>=1; i--){
            factor = factor *i;
        }
        return factor;
    }
    return 0;
}

/**
 * Function that creates memory buffers that a process can share with childs
 * 
 * @params:
 *  size:    size of buffer
 * @return:  pointer to a memory buffer that acts as shared memory
 * 
 */

void *create_shared_memory(size_t size){
  int buf_protect = PROT_READ | PROT_WRITE; //can read and write in buffer
  int buf_visible = MAP_SHARED | MAP_ANONYMOUS; //share the memory buffer only with process and its childs
  return mmap(NULL, size, buf_protect, buf_visible, -1, 0); //mmap() function allocates the memory buffer 
}

/**
 * Function that determines if a command is valid 
 * 
 * @params: 
 *  command: points to command string to be verified
 * @return:  On success, return 0.
 *           If the function returns -1, invalid command.
 * 
 */
int isValid(char *command){
    //Strcmp returns 0 if both arguments are equal
    if(strcmp(command,"add")==0 
        || strcmp(command,"multiply")==0 
        || strcmp(command,"divide")==0 
        || strcmp(command,"sleep")==0 
        || strcmp(command,"factorial")==0 
        || strcmp(command,"exit")==0 
        || strcmp(command,"shutdown")==0)
    {
        printf("valid command\n");      //For server purposes
        return 0;
    }
    else{
        printf("Error: invalid command\n");//For server purposes
        return -1;
    }
}


//*******************************//


int main(int argc, char *argv[]){

    int sockfd; // file descriptor for socket
    pid_t process_pid; //pid of the process to fork next

    /* Data for server */
    const char *host = argv[1];     //ip
    const char *portarg = argv[2];  //port
    const int port = atoi(portarg); //convert to int

    /* Data for shared memory segments */
    int *nbofclients_shmseg = create_shared_memory(SHMSEGSIZE);     //number of clients counter
    pid_t *pid_shmseg = create_shared_memory(PIDLENGTH*CLIENTMAX);  //child processes

    /* Commands that the user cand enter */
    const char *addition = "add";
    const char *multiplication = "multiply";
    const char *division = "divide";
    const char *sleeping = "sleep";
    const char *factoring = "factorial";
    const char *exiting = "exit";           //terminates frontend where command was entered, but not backend
    const char *shutting_down = "shutdown"; //terminates frontend and backend

    /* To pass structs (parameters of struct) */
    struct Request {
        char commandoperation[COMMANDLENGTH];               //command entered (ex. add)
        char arguments[NUMBEROFARGUMENTS][ARGUMENTLENGTH];  //arguments entered (ex. 1 2)
        int numberOfArgs;                                   //number of arguments in the entered command (ex. 3)
    };

    int clientCounter = 0; // Initialize the counter for the number of clients in memory
    memcpy(nbofclients_shmseg, &clientCounter, SHMSEGSIZE); //Copy the address of the counter to the shared memory segment created for this purpose
    
    /**Create the server (backend)**/
    if((create_server(host, port, &sockfd) < 0)) {
        fprintf(stderr, "Exit: The server failed to create.\n");
        return -1;
    }else{
          printf("Server listening on %s:%d\n", host, port);
    }

    while(1){
        int clientfd; //file descriptor for client
        char request[BUFSIZE];  //buffer for user
        char output[BUFSIZE]; //buffer for server to send output buffer message strings to frontend

        // Client failed to connect
        if( accept_connection(sockfd, &clientfd) < 0) {
            fprintf(stderr, "Error: The client failed to connect to the server.\n");
            exit(-1);
        }

        // 5 users are connected already, cannot connect an another user
       if(*nbofclients_shmseg>=CLIENTMAX){
           sprintf(output, "Error: Server is unavailable right now.\n");
           send_message(clientfd, output, BUFSIZE);
       }

       // If not, the there the server can serve another user (available)
       else{
           *nbofclients_shmseg +=1; //Increment the number of clients counter variable by 1
           process_pid = fork();    //Create child process
           if(process_pid<0){       //If fork returns a negative value, then fork failed
               printf("Error: Failed fork.");
               exit(-1);
           }

           //If fork returns a value of 0, then the child was created successfully
           else if (process_pid == 0){
               /**Address of the created child is copied to the shared memory buffer for child process
                * to keep track of which process is which
                */  
               memcpy((pid_shmseg + (*(nbofclients_shmseg)*PIDLENGTH)), &process_pid, PIDLENGTH);

               //Once the child process is copied in memory, can start waiting for a request by the frontend to be received
                while (1){
                    //reset the input/output buffers
                    memset(request, 0, sizeof(request));
                    memset(output, 0, sizeof(output));
                    ssize_t byte_count = recv_message(clientfd,request,BUFSIZE);    //receiving a request from the client (frontend)
                    //Nothing in request
                    if(byte_count <= 0){
                        break; //end while loop
                    }

                    // To pass request as struct
                    struct Request *requestStruct = (struct Request *)request;
                    int commandReceived = isValid(requestStruct->commandoperation); //Command check if valid
                    //If isValid returns -1, then invalid command used and output according message
                    if(commandReceived == -1){
                        char *wrongcommand = strtok(request," "); //Delimiter is first space
                        sprintf(output, "Error: Command %c%s%c not found", '"',wrongcommand,'"');
                        send_message(clientfd,output,BUFSIZE);
                    }
                    else{
                        /** Data variables for user request **/
                        char *command = requestStruct->commandoperation;    //pointer to command
                        char *first_argument = requestStruct->arguments[0]; //pointer to 1st argument
                        char *second_argument = requestStruct->arguments[1];//pointer to 2nd argument
                        int numberOfArgs = requestStruct->numberOfArgs;     //number of arguments

                        /*** Check for each command ***/
                        if(strcmp(command,addition)==0){
                            //Check if correct number of arguments entered
                            if(numberOfArgs!=3){
                                sprintf(output, "Error: The command takes 2 arguments.");
                                send_message(clientfd,output,BUFSIZE);
                            }
                            else{
                                int operand1;
                                int operand2;
                                int operandsum;
                                //Convert argument strings to integer for comparison purposes, if atoi returns 0 -> 0 or not integer
                                operand1 = atoi(first_argument); 
                                operand2 = atoi(second_argument);
                                if(((operand1 == 0)&&(strcmp(first_argument,"0")!=0))||((operand2 == 0)&&(strcmp(second_argument,"0")!=0))){
                                    sprintf(output, "Error: The arguments are not integers.");
                                    send_message(clientfd,output,BUFSIZE);
                                }
                                else{
                                    operandsum = addInts(operand1,operand2);
                                    sprintf(output, "%d", operandsum);
                                    send_message(clientfd,output,BUFSIZE);
                                }
                            }
                        }
                        else if(strcmp(command,division)==0){
                            if(numberOfArgs!=3){
                                sprintf(output, "Error: The command takes 2 arguments.");
                                send_message(clientfd,output,BUFSIZE);
                            }
                            else{
                                float operand1;
                                float operand2;
                                float operandquotient;
                                operand1 = atof(first_argument);
                                operand2 = atof(second_argument);
                                if(((operand1 == 0)&&(strcmp(first_argument,"0")!=0))||((operand2 == 0)&&(strcmp(second_argument,"0")!=0))){
                                    sprintf(output, "Error: The arguments are not floats.");
                                    send_message(clientfd,output,BUFSIZE);
                                }
                                else{
                                    operandquotient = divideFloats(operand1,operand2);
                                    if(operandquotient == -1){
                                        sprintf(output, "Error: Division by zero");
                                    }
                                    else{
                                        sprintf(output, "%f", operandquotient);
                                    }
                                    send_message(clientfd,output,BUFSIZE);
                                }
                            }
                        }
                        else if(strcmp(command,multiplication)==0){
                            if(numberOfArgs!=3){
                                sprintf(output, "Error: The command takes 2 arguments.");
                                send_message(clientfd,output,BUFSIZE);
                            }
                            else{
                                int operand1;
                                int operand2;
                                int operandproduct;
                                operand1 = atoi(first_argument);
                                operand2 = atoi(second_argument);
                                if(((operand1 == 0)&&(strcmp(first_argument,"0")!=0))||((operand2 == 0)&&(strcmp(second_argument,"0")!=0))){
                                    sprintf(output, "Error: The arguments are not integers.");
                                    send_message(clientfd,output,BUFSIZE);
                                }
                                else{
                                    operandproduct = multiplyInts(operand1,operand2);
                                    sprintf(output, "%d", operandproduct);
                                    send_message(clientfd,output,BUFSIZE);
                                }
                            }
                        }
                        else if(strcmp(command,factoring)==0){
                            if(numberOfArgs!=2){
                                sprintf(output, "Error: The command takes 1 argument.");
                                send_message(clientfd,output,BUFSIZE);
                            }
                            else{
                                uint64_t operand1;
                                uint64_t operandfactorial;
                                operand1 = atoi(first_argument);
                                if(((operand1 == 0)&&(strcmp(first_argument,"0")!=0))){
                                    sprintf(output, "Error: The arguments is not an integer.");
                                    send_message(clientfd,output,BUFSIZE);
                                }
                                else{
                                    operandfactorial = factorial(operand1);
                                    if(operandfactorial == -1){
                                        sprintf(output, "Error: The operand cannot be negative.");
                                    }
                                    else if(operandfactorial == -2){
                                        sprintf(output, "Error: The operand cannot be greater than 20.");
                                    }
                                    else{
                                        sprintf(output, "%ld", operandfactorial);
                                    }
                                    send_message(clientfd,output,BUFSIZE);
                                }
                            }
                        }
                        else if(strcmp(command,sleeping)==0){
                            if(numberOfArgs!=2){
                                sprintf(output, "Error: The command takes 1 arguments.");
                                send_message(clientfd,output,BUFSIZE);
                            }
                            else{
                                int operand1;
                                operand1 = atoi(first_argument);
                                if(((operand1 == 0)&&(strcmp(first_argument,"0")!=0))){
                                    sprintf(output, "Error: The argument is not an integer.");
                                    send_message(clientfd,output,BUFSIZE);
                                }
                                else{
                                    sleepTime(operand1);
                                    sprintf(output, "Sleeping");
                                    send_message(clientfd,output,BUFSIZE);
                                }
                            }
                        }
                        else if(strcmp(command,exiting)==0){
                            if(numberOfArgs!=1){
                                sprintf(output, "Error: The command takes 0 arguments.");
                                send_message(clientfd,output,BUFSIZE);
                            }
                            else{
                                sprintf(output, "exiting");
                                send_message(clientfd,output,BUFSIZE);
                                close(clientfd);            //Close the client's file descriptor
                                close(sockfd);
                                *nbofclients_shmseg -=1;    //Decrease the counter of users, since a user exited
                                exit(0);
                            }
                        }
                        else if(strcmp(command,shutting_down)==0){
                            if(numberOfArgs!=1){
                                sprintf(output, "Error: The command takes 0 arguments.");
                                send_message(clientfd,output,BUFSIZE);
                            }
                            else{
                                sprintf(output, "Shuttingdown");
                                send_message(clientfd,output,BUFSIZE);
                                close(clientfd);       //Close the frontend (client's fd)
                                close(sockfd);
                                //Terminate all children processes by fetching them in the shared memory segment
                                for (int i = *nbofclients_shmseg; i >= 1; i--){
                                    pid_t children = *(pid_shmseg+i*PIDLENGTH);
                                    if(children != getpid()){
                                        kill(children, SIGTERM);
                                    }
                                }
                                kill(getppid(), SIGTERM);   //Get the process ID of the parent of calling process and terminate (SIGTERM)
                                kill(getpid(), SIGTERM);    //Get the process ID of the calling process and terminate (SIGTERM)
                            }
                        }
                    }
                }
           }
       }
    }
}