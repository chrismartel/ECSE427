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

    /* define message struct */
    struct Message
    {
        char command[COMMANDSIZE];
        char parameters[NBPARAMS][PARAMSIZE];
        int nbargs;
    };

    // CONNECT TO SERVER //
    if (connect_to_server(host_ip, host_port, &sockfd) < 0)
    {
        fflush(stdout);
        fprintf(stderr, "Error connecting client to server\n");
        return -1;
    }

    // CHECK IF SERVER IS AVAILABLE //

    /* receive server response*/
    ssize_t byte_count = recv_message(sockfd, backend_msg, sizeof(backend_msg));
    if (byte_count <= 0)
    {
        fflush(stdout);
        fprintf(stderr, "Error in receiving message from server...\n");
        return 0;
    }
    // SERVER IS BUSY //
    if (strcmp(trim(backend_msg), "busy") == 0)
    {
        fflush(stdout);
        fprintf(stderr, "Server busy at the moment\nTry again later!\n");
        return 0;
    }

    // PROMPT USER COMMANDs //
    while (1)
    {
        fflush(stdout);
        printf(">> ");
        memset(user_input, 0, sizeof(user_input));
        memset(backend_msg, 0, sizeof(backend_msg));

        /* get command from terminal*/
        fgets(user_input, BUFSIZE, stdin);

        /* declare message struct*/
        struct Message message;

        char input_cpy[BUFSIZE];
        strncpy(input_cpy, user_input, BUFSIZE);

        /* split message into tokens*/
        char *param = strtok(input_cpy, " ");
        int n = 0;
        while (param != NULL)
        {
            if (n == 0)
            {
                /* set command param of message*/
                strncpy(message.command, trim(param), COMMANDSIZE);
            }
            else if (n == 1)
            {
                /* set first operand of message */
                strncpy(message.parameters[0], trim(param), PARAMSIZE);
            }
            else if (n == 2)
            {
                /* set second operand of message*/
                strncpy(message.parameters[1], trim(param), PARAMSIZE);
            }
            n++;
            param = strtok(NULL, " ");
        }
        message.nbargs = n;

        /* send serialized message to server*/
        send_message(sockfd, (char *)&message, sizeof(message));

        /* receive server response*/
        ssize_t byte_count = recv_message(sockfd, backend_msg, sizeof(backend_msg));
        if (byte_count <= 0)
        {
            fflush(stdout);
            fprintf(stderr, "Error in receiving message from server...\n");
            break;
        }
        // SLEEP //
        if (strcmp(trim(backend_msg), "sleep") == 0)
        {
            fflush(stdout);
        }
        // QUIT OR SHUTDOWN //
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
