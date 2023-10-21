#ifndef _CONN_H
#define _CONN_H

#include <sys/types.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <stdio.h>

#define NOME_SOCKET "farm.sck"

typedef struct sockaddr Sockaddr;
typedef struct sockaddr_un Sockaddr_un;

int fai_socket(int, int, int, Sockaddr_un*);

ssize_t readn(int, void*, size_t);

ssize_t writen(int, void*, size_t);

#endif // _CONN_H
