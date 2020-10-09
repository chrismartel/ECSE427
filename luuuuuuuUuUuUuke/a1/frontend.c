#include "a1_lib.h"


/** Trim function **/

char *trim(char *s) {
  char *original = s;
  size_t len = 0;

  while (isspace((unsigned char) *s)) {
    s++;
  } 
  if (*s) {
    char *p = s;
    while (*p) p++;
    while (isspace((unsigned char) *(--p)));
    p[1] = '\0';
    len = (size_t) (p - s + 1);  
  }
  return (s == original) ? s : memmove(original, s, len + 1);
}

int main(int argc, char *argv[]){
    int sockfd;
    char user_input[BUFSIZE];
    char server_msg[BUFSIZE];

    /* Data for server */
    const char *host = argv[1];     //ip
    const char *portarg = argv[2];  //port
    const int port = atoi(portarg); //convert to int

    /** Verify connection to server **/
    if (connect_to_server(host, port, &sockfd) < 0) {
        fflush(stdout);
        fprintf(stderr, "Error: Connection with client to server failed.\n");
        return -1;
    }
    
     /* To pass structs (parameters of struct) */
    struct Request {
        char commandoperation[COMMANDLENGTH];               //command entered (ex. add)
        char arguments[NUMBEROFARGUMENTS][ARGUMENTLENGTH];  //arguments entered (ex. 1 2)
        int numberOfArgs;                                   //number of arguments in the entered command (ex. 3)
    };

    while(1){
        fflush(stdout);
        printf(">> ");
        // Reset the input/output buffers
        memset(user_input, 0, sizeof(user_input));
        memset(server_msg, 0, sizeof(server_msg));
        
        // Read user input from command line
        fgets(user_input, BUFSIZE, stdin);

        // Breakdown the string entered by user into tokens 
        char *line = strtok(user_input, " "); // A space is the delimiter
        int position =0; // Declare a variable to hold the token count
        struct Request request; 
        while(line!=NULL){
            // Command copied into struct request commandoperation param
            if(position==0){
                strncpy(request.commandoperation,trim(line),COMMANDLENGTH);
            }
            // First argument copied into struct
            else if(position==1){
                strncpy(request.arguments[0],trim(line),ARGUMENTLENGTH);
            }
            // Second argument copied into struct
            else if(position==2){
                strncpy(request.arguments[1],trim(line),ARGUMENTLENGTH);
            }
            position++; // Increment counter for each argument of the user input
            line = strtok(NULL, " ");
        }
        request.numberOfArgs = position; //Counter is the number of argumnets, initialize it to the corresponding param of the struct

        // Send to server
        send_message(sockfd, (char *)&request, sizeof(request));
        // Receive an output from the server
        ssize_t byte_count = recv_message(sockfd, server_msg, sizeof(server_msg));
        // Nothing received in response
        if(byte_count<=0){
            break; // Exit while loop
        }

        // Fflush(sdtout) outputs

        // Exit frontend by fflush
        if(strcmp(server_msg,"exiting")==0){
            fflush(stdout);
            return 0;
        }
        // Shutdown
        if(strcmp(server_msg,"Shuttingdown")==0){
            fflush(stdout);
            return 0;
        }
        // Fflush sleep
        else if(strcmp(server_msg,"Sleeping") ==0){
            fflush(stdout);
        }
        // Return value to terminal
        else{
            fflush(stdout);
            printf("%s\n", server_msg);
        }
    }
    return 0;
}