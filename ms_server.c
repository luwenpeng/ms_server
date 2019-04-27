#include "ms_server.h"

static void ms_server_conn_close(ms_event_loop_t *evlop, ms_conn_t *conn);
static void ms_server_acceable_handler(ms_event_loop_t *evlop, int sockfd,
        uint32_t mask, void *data);
static void ms_server_readable_handler(ms_event_loop_t *evlop, int sockfd,
        uint32_t mask, void *data);
static void ms_server_writeable_handler(ms_event_loop_t *evlop, int sockfd,
        uint32_t mask, void *data);

void *ms_server_worker_cycle(ms_cycle_t *cycle)
{
    ms_errlog(MS_ERRLOG_STATUS, 0, "worker process \"%P\" run", getpid());

    // 创建 evlop
    cycle->evlop = ms_eventloop_create(cycle->max_evnlop_size,
            cycle->max_mempol_size, sizeof(ms_conn_t));
    if (NULL == cycle->evlop)
    {
        goto end;
    }

    // 注册监听套接字建议使用 EPOLLET
    if (ms_eventloop_file_add(cycle->evlop, cycle->listenfd, EPOLLIN | EPOLLET,
                (const ms_event_file_proc *)ms_server_acceable_handler, cycle)
            == MS_ERROR)
    {
        goto end;
    }

    ms_eventloop_main(cycle->evlop, cycle->max_epwt_timeout);

end:
    // 销毁 evlop
    ms_eventloop_destory(cycle->evlop);

    ms_errlog(MS_ERRLOG_STATUS, 0, "worker process \"%P\" exit", getpid());
    return NULL;
}

static void ms_server_conn_close(ms_event_loop_t *evlop, ms_conn_t *conn)
{
    ms_errlog(MS_ERRLOG_INFO, 0, "close fd \"%d\"", conn->fd);

    // 删除 rtimeout_timer
    if (conn->rtimeout_timer != NULL)
    {
        ms_eventloop_timer_del(evlop, conn->rtimeout_timer);
        conn->rtimeout_timer = NULL;
    }

    // 删除 stimeout_timer
    if (conn->stimeout_timer != NULL)
    {
        ms_eventloop_timer_del(evlop, conn->stimeout_timer);
        conn->stimeout_timer = NULL;
    }

    // 删除该连接注册的所有事件
    ms_eventloop_file_del(evlop, conn->fd, MS_EVENTLOOP_ALL);

    // 关闭 fd
    ms_socket_close(conn->fd);
}

static void ms_server_acceable_handler(ms_event_loop_t *evlop, int sockfd,
        uint32_t mask, void *data)
{
    int clientfd;
    ms_conn_t *conn;
    ms_cycle_t *cycle = (ms_cycle_t *)data;
    struct sockaddr_in addr;
    socklen_t addrlen = sizeof(addr);

    while (memset(&addr, 0, sizeof(addr)) &&
            (clientfd = ms_socket_accept(sockfd, (struct sockaddr *)&addr, &addrlen)) > 0)
    {
        ms_errlog(MS_ERRLOG_INFO, 0, "new client \"%d\" from \"%s\":\"%d\"",
                clientfd, inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));

        // 判断客户端数量
        if (clientfd >= evlop->size)
        {
            ms_errlog(MS_ERRLOG_ERR, 0, "eventlop event queue full");
            ms_socket_close(clientfd);
            continue;
        }

        // 设置非阻塞
        if (ms_socket_blocking(clientfd, 0) == MS_ERROR)
        {
            ms_socket_close(clientfd);
            continue;
        }

        // 设置 tcpnodelay
        if (cycle->tcpnodelay)
        {
            if (ms_socket_tcpnodelay(clientfd, cycle->tcpnodelay) == MS_ERROR)
            {
                ms_socket_close(clientfd);
                continue;
            }
        }

        // 设置 keepalive
        if (cycle->keepalive)
        {
            if (ms_socket_keepalive(clientfd, cycle->keepidle, cycle->keepintl,
                        cycle->keepcout) == MS_ERROR)
            {
                ms_socket_close(clientfd);
                continue;
            }
        }

        // 获取该 fd 对应的 conn 结构体，并初始化
        conn = (ms_conn_t *)evlop->files[clientfd].data;
        memset(conn, 0, sizeof(ms_conn_t));
        conn->rtimeout_timer = NULL;
        conn->stimeout_timer = NULL;
        conn->buffsize = ms_min(sizeof(conn->rebuff), sizeof(conn->sebuff));
        conn->sendsize = 0;
        conn->cycle = cycle;
        conn->addr.port = ntohs(addr.sin_port);
        strcpy(conn->addr.ip, inet_ntoa(addr.sin_addr));
        conn->fd = clientfd;

        // 设置接收超时
        if (cycle->rtimeout_handler && cycle->max_read_timeout > 0)
        {
            conn->rtimeout_timer = ms_eventloop_timer_add(evlop,
                    cycle->max_read_timeout,
                    (const ms_event_timer_proc *)cycle->rtimeout_handler, conn);
            if (conn->rtimeout_timer == NULL)
            {
                ms_errlog(MS_ERRLOG_ERR, 0, "ms_eventloop_timer_add() failed");
                ms_socket_close(clientfd);
                continue;
            }
        }

        // 注册读事件
        if (ms_eventloop_file_add(evlop, clientfd, EPOLLIN | MS_SEVENT_MODE,
                    (const ms_event_file_proc *)ms_server_readable_handler,
                    conn) == MS_ERROR)
        {
            ms_server_conn_close(evlop, conn);
            continue;
        }
    }
}

static void ms_server_readable_handler(ms_event_loop_t *evlop, int sockfd,
        uint32_t mask, void *data)
{
    ssize_t rev = 0;
    ms_conn_t *conn = (ms_conn_t *)data;

    // 初始化接收 & 发送缓冲区
    memset(conn->rebuff, 0, conn->buffsize);
    memset(conn->sebuff, 0, conn->buffsize);

    // 初始化要发送的数据的长度
    conn->sendsize = 0;

    // 删除 rtimeout_timer
    if (conn->rtimeout_timer)
    {
        ms_eventloop_timer_del(evlop, conn->rtimeout_timer);
        conn->rtimeout_timer = NULL;
    }

    // 边缘模式下要一直读，直到返回 EAGAIN
    rev = ms_socket_read(sockfd, conn->rebuff, conn->buffsize - 1);
    if (rev == MS_ERROR || rev == 0)
    {
        goto end;
    }

    // 请求的数据过多
    if (rev >= (conn->buffsize - 1))
    {
        ms_errlog(MS_ERRLOG_ERR, 0, "recv buff full, connect would be close");
        goto end;
    }

    // 处理请求
    if (conn->cycle->proce_handler(conn, rev) == MS_ERROR)
    {
        goto end;
    }

    // 待发送的数据过多
    if (conn->sendsize >= (conn->buffsize - 1))
    {
        ms_errlog(MS_ERRLOG_ERR, 0, "send buff full, connect would be close");
        goto end;
    }

    // 重置为写事件
    if (ms_eventloop_file_mod(evlop, sockfd, EPOLLOUT | MS_SEVENT_MODE,
                (const ms_event_file_proc *)ms_server_writeable_handler, conn)
            == MS_ERROR)
    {
        goto end;
    }

    // 设置发送超时
    if (conn->cycle->stimeout_handler && conn->cycle->max_send_timeout > 0)
    {
        conn->stimeout_timer = ms_eventloop_timer_add(evlop,
                conn->cycle->max_send_timeout,
                (const ms_event_timer_proc *)conn->cycle->stimeout_handler,
                conn);
        if (conn->stimeout_timer == NULL)
        {
            ms_errlog(MS_ERRLOG_ERR, 0, "ms_eventloop_timer_add() failed");
            goto end;
        }
    }

    return;
end:
    ms_server_conn_close(evlop, conn);
}

static void ms_server_writeable_handler(ms_event_loop_t *evlop, int sockfd,
        uint32_t mask, void *data)
{
    ms_conn_t *conn = (ms_conn_t *)data;

    // 删除 stimeout_timer
    if (conn->stimeout_timer)
    {
        ms_eventloop_timer_del(evlop, conn->stimeout_timer);
        conn->stimeout_timer = NULL;
    }

    // 发送数据
    if (ms_socket_write(sockfd, conn->sebuff, conn->sendsize) != conn->sendsize)
    {
        goto end;
    }

#ifdef DEBUG_SWITCH
    ms_errlog(MS_ERRLOG_STATUS, 0, "send : data [\"%s\"], len [\"%d\"]",
            conn->sebuff, conn->sendsize);
#endif

    // 重置为读事件
    if (ms_eventloop_file_mod(evlop, sockfd, EPOLLIN | MS_SEVENT_MODE,
                (const ms_event_file_proc *)ms_server_readable_handler, conn)
            == MS_ERROR)
    {
        goto end;
    }

    // 设置接收超时
    if (conn->cycle->rtimeout_handler && conn->cycle->max_read_timeout > 0)
    {
        conn->rtimeout_timer = ms_eventloop_timer_add(evlop,
                conn->cycle->max_read_timeout,
                (const ms_event_timer_proc *)conn->cycle->rtimeout_handler,
                conn);
        if (conn->rtimeout_timer == NULL)
        {
            ms_errlog(MS_ERRLOG_ERR, 0, "ms_eventloop_timer_add() failed");
            goto end;
        }
    }

    return;
end:
    ms_server_conn_close(evlop, conn);
}
