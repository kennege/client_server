#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#include "socket.h"

#define BUF_SIZE 1024

static void sigint_handler(int sig_num)
{
    printf("\nExiting\n");
    _exit(0);
}

int main(void)
{
    int client_sock;
    struct sockaddr_in server_addr;
    char server_message[BUF_SIZE];
    char *client_message;
    size_t buf_size = BUF_SIZE;

    client_sock = socket_create(&server_addr);
    if (client_sock < 0)
    {
        printf("Unable to create socket\n");
        return EXIT_FAILURE;
    }
    printf("Socket created successfully\n");

    if(connect(client_sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0)
    {
        printf("Unable to connect\n");
        return -1;
    }
    printf("Connected with server successfully\n");

    client_message = malloc(buf_size * sizeof(*client_message));
    if (NULL == client_message)
    {
        return EXIT_FAILURE;
    }

    if (SIG_ERR == signal(SIGINT, sigint_handler))
    {
        printf("Error handling signal\n");
        return EXIT_FAILURE;
    }

    while (1)
    {
        printf("Enter message: ");
        getline(&client_message, &buf_size, stdin);
        memset(server_message,'\0',sizeof(server_message));

        if(send(client_sock, client_message, strlen(client_message), 0) < 0)
        {
            printf("Error: Unable to send message\n");
            return EXIT_FAILURE;
        }

        if(recv(client_sock, server_message, sizeof(server_message), 0) < 0)
        {
            printf("Error while receiving server's msg\n");
            return EXIT_FAILURE;
        }

        printf("Server's response: %s\n",server_message);
   }

    shutdown(client_sock, 2);

    return EXIT_SUCCESS;
}
