#include <sys/socket.h>
#include <arpa/inet.h>

#include "socket.h"

int socket_create(struct sockaddr_in *server_addr)
{
    int socket_desc;

    socket_desc = socket(FAMILY, COM_TYPE, PROTOCOL);

    server_addr->sin_family = FAMILY;
    server_addr->sin_port = htons(PORT);
    server_addr->sin_addr.s_addr = inet_addr(LOCAL_HOST);

    return socket_desc;
}
