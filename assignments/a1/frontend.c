#include "a1_lib.h"

#define BUFSIZE 1024

int main(){
    // file descriptor of the socket
    int sockfd;
    // stores user input
    char user_input[BUFSIZE];
    // stores message from server
    char server_msg[BUFSIZE];

    if (connect_to_server("0.0.0.0", 10000, &sockfd) < 0) {
        fprintf(stderr, "Error connecting client to server\n");
        return -1;
    }
    printf("Succesfully connected client to server!\n")
    ;
    while (strcmp(user_input, "exit\n")) {
        memset(user_input, 0, sizeof(user_input));
        memset(server_msg, 0, sizeof(server_msg));

        // read user input from command line
        printf("Type your command:\n");
        fgets(user_input, BUFSIZE, stdin);

        // send the input to server
        send_message(sockfd, user_input, BUFSIZE);
        // receive a msg from the server
        ssize_t byte_count = recv_message(sockfd, server_msg, sizeof(server_msg));
        if (byte_count <= 0) {
        printf("Error in receiving message from server...\n");
        break;
        }
        printf("\n%s\n\n", server_msg);
    }

    return 0;
}
