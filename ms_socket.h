#ifndef _MS_SOCKET_H
#define _MS_SOCKET_H

#ifdef __cpluscplus
extern "C"
{
#endif

#include "ms_head.h"
#include "ms_conf.h"

#include "ms_errlog.h"

int ms_socket_create(int domain, int type, int protocol);

int ms_socket_inetpton(int af, const char *src, void *dst);
const char *ms_socket_inetntop(int af, const void *src);

int ms_socket_reuseport(int sockfd);
int ms_socket_reuseaddr(int sockfd);

int ms_socket_tcpnodelay(int sockfd, int tcpnodelay);
int ms_socket_tcpcork(int sockfd);

int ms_socket_keepalive(int sockfd, int kepidl, int kepint, int kepcut);
int ms_socket_blocking(int sockfd, int block);
int ms_socket_settimeout(int sockfd, int timeout);

int ms_socket_bind(int sockfd, const char *ip, int port);
int ms_socket_listen(int sockfd, int backlog);
int ms_socket_connect(int sockfd, const char *ip, int port);
int ms_socket_accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
ssize_t ms_socket_write(int sockfd, const void *buf, size_t len);
ssize_t ms_socket_read(int sockfd, void *buf, size_t len);
void ms_socket_close(int sockfd);

int ms_socket_create_listenfd(const char *ip, int port, int backlog);

ssize_t ms_socket_recvfrom(int sockfd, void *buf, size_t buflen, int flags,
        struct sockaddr *addr, socklen_t *addrlen);
ssize_t ms_socket_sendto(int sockfd, const void *buf, size_t buflen, int flags,
        const struct sockaddr *addr, socklen_t addrlen);

#ifdef __cpluscplus
}
#endif

#endif
