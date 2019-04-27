#ifndef _MS_SERVER_H
#define _MS_SERVER_H

#ifdef __cpluscplus
extern "C"
{
#endif

#include "ms_head.h"
#include "ms_conf.h"

#include "ms_socket.h"
#include "ms_eventloop.h"

#define MS_MAX_WORKERS 48

/*******************************************************************************
 * clientfd 的 epoll 运行模式 (listenfd 的 epoll 运行模式为 EPOLLET)
 ******************************************************************************/

//#define MS_SEVENT_MODE 0            // 水平触发
#define MS_SEVENT_MODE EPOLLET      // 边缘触发
//#define MS_SEVENT_MODE EPOLLONESHOT

typedef struct ms_addr_s  ms_addr_t;
typedef struct ms_conn_s  ms_conn_t;
typedef struct ms_cycle_s ms_cycle_t;

typedef int proc_handler(ms_conn_t *conn, ssize_t recvlen);
typedef void error_handler(ms_event_loop_t *evnlop, ms_conn_t *conn);

struct ms_addr_s {
    char ip[32];
    int  port;
};

struct ms_conn_s {
    ms_event_timer_t *rtimeout_timer;          // 接收超时的定时器
    ms_event_timer_t *stimeout_timer;          // 发送超时的定时器

    char              sebuff[MS_MAX_BUF_SIZE]; // 发送缓冲区
    char              rebuff[MS_MAX_BUF_SIZE]; // 接收缓冲区

    ssize_t           buffsize;                // 发送/接收缓冲区的容量
    ssize_t           sendsize;                // 待发送数据的长度

    ms_cycle_t       *cycle;                   // 配置信息
    ms_addr_t         addr;                    // 当前连接的地址信息

    int               fd;                      // 该连接对应的文件句柄
};

struct ms_cycle_s {
    char            *server_ip;
    int              server_port;

    int              workers;
    int              listenfd;
    int              backlog;

    char            *pidlog;
    char            *accesslog;
    char            *errorlog;
    log_level_t      loglevel;

    int              daemon;          // 是否后台运行
    int              tcpnodelay;      // 是否开启 tcpnodelay
    int              keepalive;       // 是否开启 keepalive

    int              keepidle;        // 首次 KeepAlive 探测前 TCP 的空闭时间，秒
    int              keepintl;        // 两次 KeepAlive 探测间的时间间隔，秒
    int              keepcout;        // 断开前 KeepAlive 探测的次数

    int              max_mempol_size; // 小块内存池的容量
    int              max_openfd_size; // 进程最大打开文件数
    int              max_evnlop_size; // eventloop 的容量

    int              max_epwt_timeout; // epoll_wait() 最大超时事件，毫秒
    uintptr_t        max_read_timeout; // 接收超时时间，毫秒
    uintptr_t        max_send_timeout; // 发送超时时间，毫秒

    error_handler   *rtimeout_handler; // 接收超时的回调函数
    error_handler   *stimeout_handler; // 发送超时的回调函数
    proc_handler    *proce_handler;    // 处理请求的回调函数

    ms_event_loop_t *evlop;
};

void *ms_server_worker_cycle(ms_cycle_t *cycle);

#ifdef __cpluscplus
}
#endif

#endif
