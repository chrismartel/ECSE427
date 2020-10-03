#include "a1_lib.h"

int main(int argc, char *argv[]){
    int sockfd;
    char user_input[BUFSIZE];
    char server_msg[BUFSIZE];

     /* Data for server */
    const char *host = argv[1];
    const char *portarg = argv[2];
    const int port = atoi(portarg);

    /* To pass structs */
    struct Request {
        char commandoperation[COMMANDLENGTH];
        char parameters[ARGUMENTLENGTH][NUMBEROFARGUMENTS];
        int numberOfArgs;
    };

    if (connect_to_server(host, port, &sockfd) < 0) {
        fprintf(stderr, "Error: Connection with client to server failed.\n");
        return -1;
    }

    if(strcmp(server_msg,"unavailable")==0){
        fprintf(stderr, "Error: Server is unavailable right now.\n");
    }
    while(1){
        fflush(stdout);
        printf(">> ");
        struct Request request;
        memset(user_input, 0, sizeof(user_input));
        memset(server_msg, 0, sizeof(server_msg));
        

        char datacopy[BUFSIZE];
        strncpy(datacopy,user_input,BUFSIZE);
        // read user input from command line
        fgets(user_input, BUFSIZE, stdin);

        char *line = strtok(datacopy," ");
        int position=0;
        while(line!=NULL){
            if(position==0){
                strncpy(request.commandoperation,line,COMMANDLENGTH);
            }
            else if(position==1){
                strncpy(request.parameters[0],line,ARGUMENTLENGTH);
            }
            else if(position==2){
                strncpy(request.parameters[1],line,ARGUMENTLENGTH);
            }
            position++;
            line = strtok(NULL, " ");
        }

        request.numberOfArgs = position;

        // send the input to server
        send_message(sockfd, (char *)&request, sizeof(request));
        // receive a msg from the server
        ssize_t byte_count = recv_message(sockfd, server_msg, sizeof(server_msg));
        if(byte_count<=0){
            fflush(stdout);
            fprintf(stderr,"Error: Response from server did not work.\n");
            break;
        }

        if(strcmp(server_msg,"Sleeping") ==0){
            fflush(stdout);
        }
        else if(strcmp(server_msg,"Shuttingdown")==0||strcmp(server_msg,"exiting")==0){
            fflush(stdout);
            return 0;
        }
        else{
            fflush(stdout);
            printf("Server: %s\n", server_msg);
        }

    }
    return 0;
}