#include "a1_lib.h"
#include "mystringlib.h"

int main(int argc, char *argv[])
{
    /* file descriptor of the socket*/
    int sockfd;

    /* user input buffer */
    char user_input[BUFSIZE];

    /* server message buffer*/
    char backend_msg[BUFSIZE];

    /* host ip */
    const char *host_ip = argv[1];

    /* host port */
    const char *arg2 = argv[2];
    const int host_port = atoi(arg2);

    // CONNECT TO SERVER //
    if (connect_to_server(host_ip, host_port, &sockfd) < 0)
    {
        fflush(stdout);
        fprintf(stderr, "Error connecting client to server\n");
        return -1;
    }

    // PROMPT USER COMMANDs //
    while (1)
    {
        memset(user_input, 0, sizeof(user_input));
        memset(backend_msg, 0, sizeof(backend_msg));

        /* get command from terminal*/
        fgets(user_input, BUFSIZE, stdin);

        /* send command to server*/
        send_message(sockfd, user_input, BUFSIZE);

        /* receive server response*/
        ssize_t byte_count = recv_message(sockfd, backend_msg, sizeof(backend_msg));
        if (byte_count <= 0)
        {
            fflush(stdout);
            fprintf(stderr, "Error in receiving message from server...\n");
            break;
        }
        // SERVER IS BUSY //
        if (strcmp(trim(backend_msg), "busy") == 0)
        {
            fflush(stdout);
            fprintf(stderr, "Server busy at the moment\n");
            return 0;
        }
        // SLEEP //
        else if (strcmp(trim(backend_msg), "sleep") == 0)
        {
            fflush(stdout);
        }
        // EXIT OR SHUTDOWN //
        else if (strcmp(trim(backend_msg), "shutdown") == 0 || strcmp(trim(backend_msg), "exit") == 0)
        {
            fflush(stdout);
            printf("Good Bye!\n");
            return 0;
        }
        else
        {
            fflush(stdout);
            printf("%s\n", backend_msg);
        }
    }
    return 0;
}
