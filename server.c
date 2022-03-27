#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>

#include "socket.h"

#define BUF_SIZE 1024

static void sigint_handler(int sig_num)
{
    printf("\nExiting\n");
    _exit(0);
}

struct thread_attr
{
    int socket;
    pthread_mutex_t *lock;
    int *msg_cnt;
};

static struct thread_attr *thread_attr_create(int client_sock, int *msg_cnt, pthread_mutex_t *lock)
{
    struct thread_attr *thread_attr;

    thread_attr = malloc(sizeof(*thread_attr));
    if (NULL == thread_attr)
    {
        return NULL;
    }
    memset(thread_attr, 0, sizeof(*thread_attr));

    thread_attr->msg_cnt = msg_cnt;
	thread_attr->socket = client_sock;
    thread_attr->lock = lock;

    return thread_attr;
}

static void *connection_handler(void *thread_attr)
{
    char server_message[BUF_SIZE], client_message[BUF_SIZE];
    struct thread_attr *local_attr;

    local_attr = (struct thread_attr *)thread_attr;

    printf("In thread: %d\n", pthread_self());
    pthread_detach(pthread_self());

    memset(client_message, '\0', sizeof(client_message));
    memset(server_message, '\0', sizeof(server_message));

    while (recv(local_attr->socket, client_message, sizeof(client_message), 0) > 0)
    {
        printf("Msg from client: %s\n", client_message);
        snprintf(server_message, BUF_SIZE, "Server received message: %s", client_message);
        if (send(local_attr->socket, server_message, strlen(server_message), 0) < 0)
        {
            printf("Error: Could not send\n");
            return NULL;
        }

        pthread_mutex_lock(local_attr->lock);
        printf("Message count: %d\n", ++*local_attr->msg_cnt);
        pthread_mutex_unlock(local_attr->lock);
    }

    free(thread_attr);
}

int main(void)
{
    int server_sock, client_sock, client_size, *new_sock, msg_cnt;
    struct sockaddr_in server_addr, client_addr;
    struct thread_attr *thread_attr;
	pthread_t new_thread;
    pthread_mutex_t lock;

    server_sock = socket_create(&server_addr);
    if (server_sock < 0)
    {
        printf("Error: Unable to create socket\n");
        return EXIT_FAILURE;
    }
    printf("Socket created successfully\n");

    if(bind(server_sock, (struct sockaddr*)&server_addr, sizeof(server_addr))<0)
    {
        printf("Error: Unable to bind to port\n");
        return EXIT_FAILURE;
    }
    printf("Socket bound successfully\n");

    if(listen(server_sock, 10) < 0)
    {
        printf("Error while listening\n");
        return EXIT_FAILURE;
    }
    printf("\nListening for incoming connections.....\n");

    if (SIG_ERR == signal(SIGINT, sigint_handler))
    {
        printf("Error handling signal\n");
        return EXIT_FAILURE;
    }

    if (0 != pthread_mutex_init(&lock, NULL))
    {
        printf("Error initialising mutex\n");
        return EXIT_FAILURE;
    }

    client_size = sizeof(client_addr);
    msg_cnt = 0;
    while (client_sock = accept(server_sock, (struct sockaddr*)&client_addr, &client_size))
    {
        printf("Client connected at IP: %s and port: %i\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

        if(pthread_create(&new_thread, NULL, connection_handler, (void*)thread_attr_create(client_sock, &msg_cnt, &lock)))
		{
			printf("Error: Could not create thread\n");
			return EXIT_FAILURE;
		}
    }

    shutdown(client_sock, 2);
    shutdown(server_sock, 2);

    return EXIT_SUCCESS;
}
