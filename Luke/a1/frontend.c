#include "a1_lib.h"
#include <ctype.h>
#include <string.h>

int main(int argc, char *argv[])
{
    int sockfd;
    char user_input[BUFSIZE];
    char server_msg[BUFSIZE];

    // left trim
    char *ltrim(char *s)
    {
        while (isspace(*s))
            s++;
        return s;
    }

    // right trim
    char *rtrim(char *s)
    {
        char *back = s + strlen(s);
        while (isspace(*--back))
            ;
        *(back + 1) = '\0';
        return s;
    }

    // full trim
    char *trim(char *s)
    {
        return rtrim(ltrim(s));
    }

    /* Data for server */
    const char *host = argv[1];
    const char *portarg = argv[2];
    const int port = atoi(portarg);

    /* To pass structs */
    struct Request
    {
        char commandoperation[COMMANDLENGTH];
        char parameters[NUMBEROFARGUMENTS][ARGUMENTLENGTH];
        int numberOfArgs;
    };

    if (connect_to_server(host, port, &sockfd) < 0)
    {
        fprintf(stderr, "Error: Connection with client to server failed.\n");
        return -1;
    }

    if (strcmp(server_msg, "unavailable") == 0)
    {
        fprintf(stderr, "Error: Server is unavailable right now.\n");
    }
    while (1)
    {
        fflush(stdout);
        printf(">> ");
        struct Request request;
        memset(user_input, 0, sizeof(user_input));
        memset(server_msg, 0, sizeof(server_msg));

        // read user input from command line
        fgets(user_input, BUFSIZE, stdin);
        char datacopy[BUFSIZE];
        strncpy(datacopy, user_input, BUFSIZE);

        char *line = strtok(datacopy, " ");
        int position = 0;
        while (line != NULL)
        {
            if (position == 0)
            {
                strncpy(request.commandoperation, trim(line), COMMANDLENGTH);
            }
            else if (position == 1)
            {
                strncpy(request.parameters[0], trim(line), ARGUMENTLENGTH);
            }
            else if (position == 2)
            {
                strncpy(request.parameters[1], trim(line), ARGUMENTLENGTH);
            }
            position++;
            line = strtok(NULL, " ");
        }

        request.numberOfArgs = position;

        printf("param 1: %s\n", request.parameters[0]);
        printf("param2: %s\n", request.parameters[1]);
        // send the input to server
        send_message(sockfd, (char *)&request, sizeof(request));
        // receive a msg from the server
        ssize_t byte_count = recv_message(sockfd, server_msg, sizeof(server_msg));
        if (byte_count <= 0)
        {
            fflush(stdout);
            fprintf(stderr, "Error: Response from server did not work.\n");
            break;
        }

        if (strcmp(server_msg, "Sleeping") == 0)
        {
            fflush(stdout);
        }
        else if (strcmp(server_msg, "Shuttingdown") == 0 || strcmp(server_msg, "exiting") == 0)
        {
            fflush(stdout);
            return 0;
        }
        else
        {
            fflush(stdout);
            printf("Server: %s\n", server_msg);
        }
    }
    return 0;
}
