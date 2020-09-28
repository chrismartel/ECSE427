#include "a1_lib.h"
#include "mystringlib.h"

#define BUFSIZE 1024

int main()
{
    // file descriptor of the socket
    int sockfd;
    // stores user input
    char user_input[BUFSIZE];
    // stores message from server
    char backend_msg[BUFSIZE];

    if (connect_to_server("127.0.0.1", 10000, &sockfd) < 0)
    {
        fprintf(stderr, "Error connecting client to server\n");
        return -1;
    }


    while (1)
    {
        memset(user_input, 0, sizeof(user_input));
        memset(backend_msg, 0, sizeof(backend_msg));

        // read user input from command line
        printf("Type your command: ");
        fgets(user_input, BUFSIZE, stdin);

        // send the input to server
        send_message(sockfd, user_input, BUFSIZE);
        // receive a msg from the server
        ssize_t byte_count = recv_message(sockfd, backend_msg, sizeof(backend_msg));
        if (byte_count <= 0)
        {
            printf("Error in receiving message from server...\n");
            break;
        }

        if (strcmp(trim(backend_msg), "shutdown") == 0 || strcmp(trim(backend_msg), "exit") == 0)
        {
            return 0;
        }
        else
        {
            printf("%s\n\n", backend_msg);
        }
    }

    return 0;
}
