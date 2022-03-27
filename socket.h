#ifndef _SOCKET_H_
#define _SOCKET_H_

int socket_create(struct sockaddr_in *server_addr);

#define FAMILY AF_INET
#define COM_TYPE SOCK_STREAM
#define PORT 2000
#define PROTOCOL IPPROTO_TCP
#define LOCAL_HOST "127.0.0.1"

#endif /* __SOCKET__ */
