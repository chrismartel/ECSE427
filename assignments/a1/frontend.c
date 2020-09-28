#include "a1_lib.h"
#include "mystringlib.h"

int main()
{
    /* file descriptor of the socket*/
    int sockfd;

    /* user input buffer */
    char user_input[BUFSIZE];

    /* server message buffer*/
    char backend_msg[BUFSIZE];

    // CONNECT TO SERVER //
    if (connect_to_server("127.0.0.5", 10000, &sockfd) < 0)
    {
        fprintf(stderr, "Error connecting client to server\n");
        return -1;
    }

    // PROMPT USER COMMANDs //
    while (1)
    {
        memset(user_input, 0, sizeof(user_input));
        memset(backend_msg, 0, sizeof(backend_msg));

        printf("Type your command: ");
        /* get command from terminal*/
        fgets(user_input, BUFSIZE, stdin);

        /* send command to server*/
        send_message(sockfd, user_input, BUFSIZE);

        /* receive server response*/
        ssize_t byte_count = recv_message(sockfd, backend_msg, sizeof(backend_msg));
        if (byte_count <= 0)
        {
            printf("Error in receiving message from server...\n");
            break;
        }
        // SERVER IS BUSY //
        if (strcmp(trim(backend_msg), "busy") == 0)
        {
            printf("Server busy at the moment\n");
            return 0;
        }
        // EXIT OR SHUTDOWN //
        else if (strcmp(trim(backend_msg), "shutdown") == 0 || strcmp(trim(backend_msg), "exit") == 0)
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
