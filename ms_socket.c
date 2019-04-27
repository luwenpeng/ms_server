#include "ms_socket.h"

/***********************************************************
 * @Func   : ms_socket_create()
 * @Author : lwp
 * @Brief  : 创建 socket 套接字。
 * @Param  : [in] domain
 * @Param  : [in] type
 * @Param  : [in] protocol
 * @Return : MS_ERROR : 失败
 *           sockfd   : 大于 0 的文件句柄
 * @Note   : 
 ***********************************************************/
int ms_socket_create(int domain, int type, int protocol)
{
    int sockfd = -1;

    sockfd = socket(domain, type, protocol);
    if (sockfd == -1)
    {
        ms_errlog(MS_ERRLOG_ERR, errno, "socket() failed");
        return MS_ERROR;
    }

    return sockfd;
}
// @ms_socket_create() ok

/***********************************************************
 * @Func   : ms_socket_inetpton()
 * @Author : lwp
 * @Brief  : 将 src 中 "ddd.ddd.ddd.ddd" 或 "x:x:x:x:x:x:x:x"
 *           格式的地址转换成数值型存储到 dst 中。
 * @Param  : [in] af : AF_INET/AF_INET6
 * @Param  : [in] src : ddd.ddd.ddd.ddd/x:x:x:x:x:x:x:x
 * @Param  : [in] dst
 * @Return : MS_ERROR : 失败
 *           MS_OK    : 成功
 * @Note   : 
 ***********************************************************/
int ms_socket_inetpton(int af, const char *src, void *dst)
{
    int rv = 0;

    rv = inet_pton(af, src, dst);
    if (rv == -1)
    {
        ms_errlog(MS_ERRLOG_ERR, errno, "inet_pton() failed");
        return MS_ERROR;
    }
    if (rv == 0)
    {
        ms_errlog(MS_ERRLOG_ERR, 0,
                "inet_pton() failed (0: invalid src addr \"%s\")", src);
        return MS_ERROR;
    }

    return MS_OK;
}
// @ms_socket_inetpton() ok

/***********************************************************
 * @Func   : ms_socket_inetntop()
 * @Author : lwp
 * @Brief  : 将 src 中数值型格式的地址转换成 "ddd.ddd.ddd.ddd"
 *           或 "x:x:x:x:x:x:x:x" 格式。
 * @Param  : [in] af : AF_INET/AF_INET6
 * @Param  : [in] src
 * @Return : NULL : 失败
 *           !NULL : "ddd.ddd.ddd.ddd" 或 "x:x:x:x:x:x:x:x" 格式的地址
 * @Note   : 
 ***********************************************************/
const char *ms_socket_inetntop(int af, const void *src)
{
    static char dst[256] = { 0 };
    const char *p = inet_ntop(af, src, dst, sizeof(dst));
    if (p == NULL)
    {
        ms_errlog(MS_ERRLOG_ERR, errno, "inet_ntop() failed");
        return NULL;
    }

    return p;
}
// @ms_socket_inetntop() ok

/***********************************************************
 * @Func   : ms_socket_reuseport()
 * @Author : lwp
 * @Brief  : 设置端口复用。
 * @Param  : [in] sockfd
 * @Return : MS_ERROR : 失败
 *           MS_OK    : 成功
 * @Note   : 
 ***********************************************************/
int ms_socket_reuseport(int sockfd)
{
    int reuseport = 1;

    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEPORT, (const void *)&reuseport,
                sizeof(reuseport)) == -1)
    {
        ms_errlog(MS_ERRLOG_ERR, errno, "setsockopt(SO_REUSEPORT) failed");
        return MS_ERROR;
    }

    return MS_OK;
}
// @ms_socket_reuseport() ok

/***********************************************************
 * @Func   : ms_socket_reuseaddr()
 * @Author : lwp
 * @Brief  : 设置地址复用。
 * @Param  : [in] sockfd
 * @Return : MS_ERROR : 失败
 *           MS_OK    : 成功
 * @Note   : 
 ***********************************************************/
int ms_socket_reuseaddr(int sockfd)
{
    int reuseaddr = 1;

    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (const void *)&reuseaddr,
                sizeof(reuseaddr)) == -1)
    {
        ms_errlog(MS_ERRLOG_ERR, errno, "setsockopt(SO_REUSEADDR) failed");
        return MS_ERROR;
    }

    return MS_OK;
}
// @ms_socket_reuseaddr() ok

/***********************************************************
 * @Func   : ms_socket_tcpnodelay()
 * @Author : lwp
 * @Brief  : 是否启用 tcpnodelay。
 * @Param  : [in] sockfd
 * @Param  : [in] tcpnodelay : 1,开启，0,关闭
 * @Return : MS_ERROR : 失败
 *           MS_OK    : 成功
 * @Note   : 
 * tcpnodelay = 1; 禁用 Nagle 算法，意味着即使只有少量数据，也会尽快发送。
 * tcpnodelay = 0; 数据被缓冲，直到有足够的数量发送，避免频发小数据包，导致网络利用率低。
 *                 与 TCP_CORK 和 sendfile() 一起使用，用于优化吞吐量。
 ***********************************************************/
int ms_socket_tcpnodelay(int sockfd, int tcpnodelay)
{
    if (tcpnodelay)
        tcpnodelay = 1;
    else
        tcpnodelay = 0;

    if (setsockopt(sockfd, IPPROTO_TCP, TCP_NODELAY, (const void *)&tcpnodelay,
                sizeof(tcpnodelay)) == -1)
    {
        ms_errlog(MS_ERRLOG_ERR, errno, "setsockopt(TCP_NODELAY) failed");
        return MS_ERROR;
    }

    return MS_OK;
}
// @ms_socket_tcpnodelay() ok

/***********************************************************
 * @Func   : ms_socket_tcpcork()
 * @Author : lwp
 * @Brief  : 不要发送部分帧，用于优化吞吐量。
 * @Param  : [in] sockfd
 * @Return : MS_ERROR : 失败
 *           MS_OK    : 成功
 * @Note   : 与 TCP_delay 和 sendfile() 一起使用，用于优化吞吐量。
 ***********************************************************/
int ms_socket_tcpcork(int sockfd)
{
    int cork = 1;

    if (setsockopt(sockfd, IPPROTO_TCP, TCP_CORK, (const void *)&cork,
                sizeof(cork)) == -1)
    {
        ms_errlog(MS_ERRLOG_ERR, errno, "setsockopt(TCP_CORK) failed");
        return MS_ERROR;
    }

    return MS_OK;
}
// @ms_socket_tcpcork() ok

/***********************************************************
 * @Func   : ms_socket_keepalive()
 * @Author : lwp
 * @Brief  : 在面向连接的套接字上开启 KeepAlive。
 * @Param  : [in] sockfd
 * @Param  : [in] kepidl : 首次 KeepAlive 探测前 TCP 的空闭时间，缺省值: 7200(s)
 * @Param  : [in] kepint : 两次 KeepAlive 探测间的时间间隔，缺省值: 75(s)
 * @Param  : [in] kepcut : 断开 KeepAlive 前探测的次数，缺省值: 9(次)
 * @Return : MS_ERROR : 失败
 *           MS_OK    : 成功
 * @Note   : 
 ***********************************************************/
int ms_socket_keepalive(int sockfd, int kepidl, int kepint, int kepcut)
{
    int kepalv = 1;

    // 在面向连接的套接字上发送保持活动的消息
    if (setsockopt(sockfd, SOL_SOCKET, SO_KEEPALIVE, (void *)&kepalv,
                sizeof(kepalv)) == -1)
    {
        ms_errlog(MS_ERRLOG_ERR, errno, "setsockopt(SO_KEEPALIVE) failed");
        return MS_ERROR;
    }

    // 首次 KeepAlive 探测前 TCP 的空闭时间
    if (setsockopt(sockfd, IPPROTO_TCP, TCP_KEEPIDLE, (void *)&kepidl,
                sizeof(kepidl)) == -1)
    {
        ms_errlog(MS_ERRLOG_ERR, errno, "setsockopt(TCP_KEEPIDLE) failed");
        return MS_ERROR;
    }

    // 两次 KeepAlive 探测间的时间间隔
    if (setsockopt(sockfd, IPPROTO_TCP, TCP_KEEPINTVL, (void *)&kepint,
                sizeof(kepint)) == -1)
    {
        ms_errlog(MS_ERRLOG_ERR, errno, "setsockopt(TCP_KEEPINTVL) failed");
        return MS_ERROR;
    }

    // 断开前 KeepAlive 探测的次数
    if (setsockopt(sockfd, IPPROTO_TCP, TCP_KEEPCNT, (void *)&kepcut,
                sizeof(kepcut)) == -1)
    {
        ms_errlog(MS_ERRLOG_ERR, errno, "setsockopt(TCP_KEEPCNT) failed");
        return MS_ERROR;
    }

    return MS_OK;
}
// @ms_socket_keepalive() ok

/***********************************************************
 * @Func   : ms_socket_blocking()
 * @Author : lwp
 * @Brief  : 将 sockfd 设置为阻塞/非阻塞。
 * @Param  : [in] sockfd
 * @Param  : [in] block
 * @Return : MS_ERROR : 失败
 *           MS_OK    : 成功
 * @Note   : flags & ~O_NONBLOCK 阻塞
 *           flags | O_NONBLOCK 非阻塞
 ***********************************************************/
int ms_socket_blocking(int sockfd, int block)
{
    int op = -1;

    op = fcntl(sockfd, F_GETFL);
    if (op == -1)
    {
        ms_errlog(MS_ERRLOG_ERR, errno, "fcntl(F_GETFL) failed");
        return MS_ERROR;
    }

    if (block)
    {
        op &=  ~O_NONBLOCK;
    }
    else
    {
        op |= O_NONBLOCK;
    }

    if (fcntl(sockfd, F_SETFL, op) == -1)
    {
        ms_errlog(MS_ERRLOG_ERR, errno, "fcntl(F_SETFL) failed");
        return MS_ERROR;
    }

    return MS_OK;
}
// @ms_socket_blocking() ok

/***********************************************************
 * @Func   : ms_socket_settimeout()
 * @Author : lwp
 * @Brief  : 设置阻塞套接字的发送/接收超时时间。
 * @Param  : [in] sockfd
 * @Param  : [in] timeout : 单位毫秒 [0, 2147483647]
 * @Return : MS_ERROR : 失败
 *           MS_OK    : 成功
 * @Note   : 
 ***********************************************************/
int ms_socket_settimeout(int sockfd, int timeout)
{
    struct timeval tv;
    tv.tv_sec = timeout / 1000;
    tv.tv_usec = (timeout % 1000) * 1000;

    if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(struct timeval))
            == -1)
    {
        ms_errlog(MS_ERRLOG_ERR, errno, "setsockopt(SO_RCVTIMEO) failed");
        return MS_ERROR;
    }

    if (setsockopt(sockfd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(struct timeval))
            == -1)
    {
        ms_errlog(MS_ERRLOG_ERR, errno, "setsockopt(SO_SNDTIMEO) failed");
        return MS_ERROR;
    }

    return MS_OK;
}
// @ms_socket_settimeout() ok

/***********************************************************
 * @Func   : ms_socket_bind()
 * @Author : lwp
 * @Brief  : 将 sockfd 与 ip:port 绑定。
 * @Param  : [in] sockfd
 * @Param  : [in] ip
 * @Param  : [in] port
 * @Return : MS_ERROR : 失败
 *           MS_OK    : 成功
 * @Note   : 
 ***********************************************************/
int ms_socket_bind(int sockfd, const char *ip, int port)
{
    struct sockaddr_in addr;

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = inet_addr(ip);
    // addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if ((bind(sockfd, (const struct sockaddr *)&addr, sizeof(addr))) == -1)
    {
        ms_errlog(MS_ERRLOG_ERR, errno, "bind() failed");
        return MS_ERROR;
    }

    return MS_OK;
}
// @ms_socket_bind() ok

/***********************************************************
 * @Func   : ms_socket_listen()
 * @Author : lwp
 * @Brief  : 监听。
 * @Param  : [in] sockfd
 * @Param  : [in] backlog
 * @Return : MS_ERROR : 失败
 *           MS_OK    : 成功
 * @Note   : 
 ***********************************************************/
int ms_socket_listen(int sockfd, int backlog)
{
    if (listen(sockfd, backlog) == -1)
    {
        ms_errlog(MS_ERRLOG_ERR, errno, "listen() failed");
        return MS_ERROR;
    }

    return MS_OK;
}
// @ms_socket_listen() OK

/***********************************************************
 * @Func   : ms_socket_connect()
 * @Author : lwp
 * @Brief  : 使用 sockfd 去连接 ip:port。
 * @Param  : [in] sockfd
 * @Param  : [in] ip
 * @Param  : [in] port
 * @Return : MS_ERROR : 失败
 *           MS_OK    : 成功
 * @Note   : 
 ***********************************************************/
int ms_socket_connect(int sockfd, const char *ip, int port)
{
    int rev = -1;
    struct sockaddr_in addr;

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = inet_addr(ip);

    rev = connect(sockfd, (const struct sockaddr *)&addr, sizeof(addr));
    if (rev == -1 && errno != EINPROGRESS)
    // if (rev == -1)
    {
        ms_errlog(MS_ERRLOG_ERR, errno, "connect() failed");
        return MS_ERROR;
    }

    return MS_OK;
}
// @ms_socket_connect() ok

/***********************************************************
 * @Func   : ms_socket_accept()
 * @Author : lwp
 * @Brief  : 接受客户端连接。
 * @Param  : [in] sockfd : listenfd
 * @Param  : [in/out] add
 * @Param  : [in/out] addrlen
 * @Return : MS_ERROR : 失败
 *           clientfd : 成功
 * @Note   : 
 ***********************************************************/
int ms_socket_accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen)
{
    int clientfd = -1;

    while (1)
    {
        clientfd = accept(sockfd, addr, addrlen);
        if (clientfd == -1)
        {
            // 信号中断
            if (errno == EINTR)
            {
                continue;
            }
            else if (errno == EAGAIN || errno == EWOULDBLOCK)
            {
                break;
            }
            else
            {
                ms_errlog(MS_ERRLOG_ERR, errno, "accept() failed");
                return MS_ERROR;
            }
        }
        break;
    }

    return clientfd;
}
// @ms_socket_accept() ok

/***********************************************************
 * @Func   : ms_socket_write()
 * @Author : lwp
 * @Brief  : 发送数据。
 * @Param  : [in] sockfd
 * @Param  : [in] buf
 * @Param  : [in] len
 * @Return : MS_ERROR : 失败
 *           clientfd : 成功
 * @Note   : 
 ***********************************************************/
ssize_t ms_socket_write(int sockfd, const void *buf, size_t len)
{
    ssize_t rev = -1;
    void *ptr = (void *)buf;
    size_t size = len;
    size_t total = 0;

    while (1)
    {
        rev = write(sockfd, ptr, size);
        if (-1 == rev)
        {
            if (errno == EINTR)
            {
                continue;
            }
            else if (errno == EAGAIN || errno == EWOULDBLOCK)
            {
                break;
            }
            else
            {
                ms_errlog(MS_ERRLOG_ERR, errno, "write() failed");
                return MS_ERROR;
            }
        }

        ptr += rev;
        size -= rev;
        total += rev;

        if (rev == 0 || size == 0)
        {
            break;
        }
    }
    ms_errlog(MS_ERRLOG_INFO, 0, "write len \"%z\"", rev);

    return total;
}
// @ms_socket_write() ok

/***********************************************************
 * @Func   : ms_socket_read()
 * @Author : lwp
 * @Brief  : 接收数据。
 * @Param  : [in] sockfd
 * @Param  : [in/out] buf
 * @Param  : [in] len
 * @Return : MS_ERROR : 失败
 *           clientfd : 成功
 * @Note   : 
 ***********************************************************/
ssize_t ms_socket_read(int sockfd, void *buf, size_t len)
{
    ssize_t rev = -1;
    void *ptr = buf;
    size_t size = len;
    size_t total = 0;

    while (1)
    {
        rev = read(sockfd, ptr, size);
        if (-1 == rev)
        {
            if (errno == EINTR)
            {
                continue;
            }
            else if (errno == EAGAIN || errno == EWOULDBLOCK)
            {
                break;
            }
            else
            {
                ms_errlog(MS_ERRLOG_ERR, errno, "read() failed");
                return MS_ERROR;
            }
        }

        ptr += rev;
        size -= rev;
        total += rev;

        if (rev == 0 || total == len)
        {
            break;
        }
    }
    ms_errlog(MS_ERRLOG_INFO, 0, "read len \"%z\"", total);

    return total;
}
// @ms_socket_read() ok

/***********************************************************
 * @Func   : ms_socket_close()
 * @Author : lwp
 * @Brief  : 关闭 sockfd 文件句柄。
 * @Param  : [in] sockfd
 * @Return : NONE
 * @Note   : 
 ***********************************************************/
void ms_socket_close(int sockfd)
{
    if (sockfd > 0)
        close(sockfd);
}
// @ms_socket_close() ok

/***********************************************************
 * @Func   : ms_socket_create_listenfd()
 * @Author : lwp
 * @Brief  : 创建监听套接字，设置地址复用，设置端口复用，设置非阻塞，绑定并监听。
 * @Param  : [in] ip
 * @Param  : [in] port
 * @Param  : [in] backlog
 * @Return : MS_ERROR : 失败
 *           listenfd : 成功
 * @Note   : 
 ***********************************************************/
int ms_socket_create_listenfd(const char *ip, int port, int backlog)
{
    int listenfd = -1;

    if ((listenfd = ms_socket_create(AF_INET, SOCK_STREAM, 0)) == MS_ERROR)
    {
        goto end;
    }

    if (ms_socket_reuseaddr(listenfd) == MS_ERROR)
    {
        goto end;
    }

    // 必须在 bind() 之前调用
    if (ms_socket_reuseport(listenfd) == MS_ERROR)
    {
        goto end;
    }

    if (ms_socket_bind(listenfd, ip, port) == MS_ERROR)
    {
        goto end;
    }

    if (ms_socket_listen(listenfd, backlog) == MS_ERROR)
    {
        goto end;
    }

    // 在 listen() 之后设置非阻塞，参照 redis
    if (ms_socket_blocking(listenfd, 0) == MS_ERROR)
    {
        goto end;
    }

    return listenfd;
end:
    ms_socket_close(listenfd);
    return MS_ERROR;
}
// @ms_socket_create_listenfd() ok

/***********************************************************
 * @Func   : ms_socket_recvfrom()
 * @Author : lwp
 * @Brief  : 接收数据。
 * @Param  : [in] sockfd
 * @Param  : [in/out] buff
 * @Param  : [in] buflen
 * @Param  : [in] flags
 * @Param  : [in/out] addr
 * @Param  : [in/out] addrlen
 * @Return : MS_ERROR : 失败
 *           recvlen  : 接收到数据的长度
 * @Note   : 
 ***********************************************************/
ssize_t ms_socket_recvfrom(int sockfd, void *buf, size_t buflen, int flags,
        struct sockaddr *addr, socklen_t *addrlen)
{
    ssize_t recvlen = 0;

    recvlen = recvfrom(sockfd, buf, buflen, flags, addr, addrlen);
    if (recvlen == -1)
    {
        ms_errlog(MS_ERRLOG_ERR, errno, "recvfrom() failed");
        return MS_ERROR;
    }

    return recvlen;
}
// @ms_socket_recvfrom() ok

/***********************************************************
 * @Func   : ms_socket_sendto()
 * @Author : lwp
 * @Brief  : 发送数据。
 * @Param  : [in] sockfd
 * @Param  : [in] buf
 * @Param  : [in] buflen
 * @Param  : [in] flags
 * @Param  : [in] addr
 * @Param  : [in] addrlen
 * @Return : MS_ERROR : 失败
 *           sendlen  : 发送的数据长度
 * @Note   : 
 ***********************************************************/
ssize_t ms_socket_sendto(int sockfd, const void *buf, size_t buflen, int flags,
        const struct sockaddr *addr, socklen_t addrlen)
{
    ssize_t sendlen = 0;

    sendlen = sendto(sockfd, buf, buflen, flags, addr, addrlen);
    if (sendlen == -1)
    {
        ms_errlog(MS_ERRLOG_ERR, errno, "sendto() failed");
        return MS_ERROR;
    }

    return sendlen;
}
// @ms_socket_sendto() ok
