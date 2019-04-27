#include "ms_server.h"

#include "ms_acclog.h"
#include "ms_config.h"
#include "ms_daemon.h"
#include "ms_rlimit.h"
#include "ms_signal.h"

static ms_cycle_t  g_cycle;
static ms_cycle_t *cycle = &g_cycle;
static pid_t workers_pid[MS_MAX_WORKERS] = { 0 };
static char http_head[] = "HTTP/1.1 200 OK\r\nContent-Length: %d\r\n\r\n%s";

static void master_exit_signal_handler(int signal);
static void master_reopen_signal_handler(int signal);
static void worker_exit_signal_handler(int signal);
static void worker_reopen_signal_handler(int signal);
static void ms_server_rtimeout_handler(ms_event_loop_t *evlop, ms_conn_t *conn);
static void ms_server_stimeout_handler(ms_event_loop_t *evlop, ms_conn_t *conn);
static int ms_server_proce_handler(ms_conn_t *conn, ssize_t recvlen);

// 信号及其对应的 handler，最后一个信号设置为 -1
static ms_signal_t signals_st[] = {
    { SIGINT , master_reopen_signal_handler },
    { SIGQUIT, master_exit_signal_handler   },
    { SIGUSR1, worker_reopen_signal_handler },
    { SIGUSR2, worker_exit_signal_handler   },
    { -1     , NULL                         }
};

// 调用进程注册其关心的信号，阻塞未在 worker_signals[] 中的信号。
// 注意 : SIGKILL 和 SIGSTOP 信号不能被捕获，阻塞，忽视。最后一个信号设置为 -1。
static int master_signals[] = { SIGINT,  SIGQUIT, -1 };
static int worker_signals[] = { SIGUSR1, SIGUSR2, -1 };

static ms_conf_item_t ms_sys_conf[] = {
    { "server_ip"        , { 0 }, check_ipv4  },
    { "server_port"      , { 0 }, check_port  },
    { "workers"          , { 0 }, check_num   },
    { "listen_backlog"   , { 0 }, check_num   },
    { "pid_log"          , { 0 }, check_file  },
    { "access_log"       , { 0 }, check_file  },
    { "error_log"        , { 0 }, check_file  },
    { "log_level"        , { 0 }, check_level },
    { "is_daemon"        , { 0 }, check_num   },
    { "is_tcpnodelay"    , { 0 }, check_num   },
    { "is_keepalive"     , { 0 }, check_num   },
    { "keepidle"         , { 0 }, check_num   },
    { "keepintl"         , { 0 }, check_num   },
    { "keepcout"         , { 0 }, check_num   },
    { "max_mempool_size" , { 0 }, check_num   },
    { "max_open_files"   , { 0 }, check_num   },
    { "max_events_size"  , { 0 }, check_num   },
    { "max_epoll_timeout", { 0 }, check_num   },
    { "max_read_timeout" , { 0 }, check_num   },
    { "max_send_timeout" , { 0 }, check_num   }
};

int main(int argc, char **argv)
{
    pid_t pid = 0;

    if (argc != 2)
    {
        printf("Usage: %s conf_file\n", argv[0]);
        return MS_ERROR;
    }

    // 初始化 errlist
    if (ms_errno_init() == MS_ERROR)
    {
        goto end;
    }

    // 解析配置文件
    if (ms_config_parse(argv[1], ms_sys_conf,
                sizeof(ms_sys_conf) / sizeof(ms_conf_item_t)) == MS_ERROR)
    {
        goto end;
    }

    cycle->listenfd         = -1;
    cycle->evlop            = NULL; // evlop 结构体指针
    cycle->server_ip        =      ms_config_get_value("server_ip");
    cycle->server_port      = atoi(ms_config_get_value("server_port"));
    cycle->workers          = atoi(ms_config_get_value("workers"));
    cycle->backlog          = atoi(ms_config_get_value("listen_backlog"));
    cycle->pidlog           =      ms_config_get_value("pid_log");
    cycle->accesslog        =      ms_config_get_value("access_log");
    cycle->errorlog         =      ms_config_get_value("error_log");
    cycle->loglevel         = (log_level_t)atoi(ms_config_get_value("log_level"));
    cycle->daemon           = atoi(ms_config_get_value("is_daemon"));
    cycle->tcpnodelay       = atoi(ms_config_get_value("is_tcpnodelay"));
    cycle->keepalive        = atoi(ms_config_get_value("is_keepalive"));
    cycle->keepidle         = atoi(ms_config_get_value("keepidle"));          // 首次 KeepAlive 探测前 TCP 的空闭时间，秒
    cycle->keepintl         = atoi(ms_config_get_value("keepintl"));          // 两次 KeepAlive 探测间的时间间隔，秒
    cycle->keepcout         = atoi(ms_config_get_value("keepcout"));          // 断开前 KeepAlive 探测的次数
    cycle->max_mempol_size  = atoi(ms_config_get_value("max_mempool_size"));  // 内存池可分配的最大小块内存
    cycle->max_openfd_size  = atoi(ms_config_get_value("max_open_files"));    // 进程最大打开文件数
    cycle->max_evnlop_size  = atoi(ms_config_get_value("max_events_size"));   // evlop 的最大容量
    cycle->max_epwt_timeout = atoi(ms_config_get_value("max_epoll_timeout")); // epollwait 的最大超时时间，毫秒
    cycle->max_read_timeout = atoi(ms_config_get_value("max_read_timeout"));  // 接收超时时间，毫秒 0 代表不启用
    cycle->max_send_timeout = atoi(ms_config_get_value("max_send_timeout"));  // 发送超时时间，毫秒 0 代表不启用

    cycle->rtimeout_handler = ms_server_rtimeout_handler; // 接收超时的回调函数
    cycle->stimeout_handler = ms_server_stimeout_handler; // 发送超时的回调函数
    cycle->proce_handler    = ms_server_proce_handler;    // 处理请求的回调函数

    // 初始化 errlog
    if (ms_errlog_init(cycle->errorlog, cycle->loglevel) == MS_ERROR)
    {
        goto end;
    }
    ms_errlog(MS_ERRLOG_STATUS, 0, "================== start ==================");

    // 初始化 acclog
    if (ms_acclog_init(cycle->accesslog) == MS_ERROR)
    {
        goto end;
    }

    // 判断是否已有实例在运行
    pid = ms_daemon_get_pid(cycle->pidlog);
    if (pid != MS_ERROR)
    {
        ms_errlog_stderr(0, "server [%s] \"%P\" is running", argv[0], pid);
        goto end;
    }

    // 设置进程打开文件数
    if (ms_rlimit_set(cycle->max_openfd_size) == MS_ERROR)
    {
        goto end;
    }

    // 初始化信号 handler
    if (ms_signal_init(signals_st) == MS_ERROR)
    {
        goto end;
    }

    // 创建监听套接字
    cycle->listenfd = ms_socket_create_listenfd(cycle->server_ip,
            cycle->server_port, cycle->backlog);
    if (cycle->listenfd == MS_ERROR)
    {
        goto end;
    }

    // 守护进程
    if (cycle->daemon)
    {
        if (ms_daemon() == MS_ERROR)
        {
            goto end;
        }
    }

    // 记录 pid
    if (ms_daemon_set_pid(cycle->pidlog, getpid()) == MS_ERROR)
    {
        goto end;
    }

    // 创建多个 worker 进程
    if (cycle->workers <= 0)
        cycle->workers = 1;
    if (cycle->workers > MS_MAX_WORKERS)
        cycle->workers = MS_MAX_WORKERS;
    for (int i = 0; i < cycle->workers; i++)
    {
        pid = fork();
        switch (pid)
        {
            // 失败
            case -1:
                ms_errlog(MS_ERRLOG_ERR, errno, "fork() failed");
                goto end;
            // 子进程
            case 0:
                // 子进程注册其关心的信号
                if (ms_signal_register(worker_signals) == MS_ERROR)
                {
                    exit(1);
                }
                ms_server_worker_cycle(cycle);
                exit(1);
            // 父进程
            default:
                workers_pid[i] = pid;
                break;
        }
    }

    // master 进程注册其关注的信号
    if (ms_signal_register(master_signals) == MS_ERROR)
    {
        // TODO
    }

    // 等待 worker 进程退出
    for (int i = 0; i < cycle->workers; i++)
    {
        do {
            pid = wait(NULL);
            if (pid == -1 && errno != EINTR)
            {
                ms_errlog(MS_ERRLOG_ERR, errno, "wait() failed");
            }
        } while (pid == -1);
    }

    // 清空 pid 文件
    ms_daemon_clean_pid(cycle->pidlog);

end:
    // 关闭监听套接字
    ms_socket_close(cycle->listenfd);

    // 关闭 acclog
    ms_acclog_close();

    // 关闭 errlog
    ms_errlog(MS_ERRLOG_STATUS, 0, "================== exit ==================");
    ms_errlog_stdout("================== exit ==================");
    ms_errlog_close();

    // 销毁 errlist
    ms_errno_destory();

    return MS_OK;
}

static void master_exit_signal_handler(int signal)
{
    ms_errlog(MS_ERRLOG_STATUS, 0,
            "master recv \"%s\" exit signal, notify worker ...",
            ms_signal_toname(signal));

    for (int i = 0; i < cycle->workers; i++)
    {
        if (kill(workers_pid[i], SIGUSR2) == -1)
        {
            ms_errlog(MS_ERRLOG_ERR, errno, "kill() failed");
        }
    }
}

static void master_reopen_signal_handler(int signal)
{
    ms_errlog(MS_ERRLOG_STATUS, 0,
            "master recv \"%s\" reopen signal, notify worker ...",
            ms_signal_toname(signal));

    for (int i = 0; i < cycle->workers; i++)
    {
        if (kill(workers_pid[i], SIGUSR1) == -1)
        {
            ms_errlog(MS_ERRLOG_ERR, errno, "kill() failed");
        }
    }

    ms_errlog_reopen();
    ms_acclog_reopen();
}

static void worker_exit_signal_handler(int signal)
{
    ms_errlog(MS_ERRLOG_STATUS, 0, "worker recv \"%s\", stop eventloop",
            ms_signal_toname(signal));

    ms_eventloop_stop(cycle->evlop);
}

static void worker_reopen_signal_handler(int signal)
{
    ms_errlog(MS_ERRLOG_STATUS, 0,
            "worker recv \"%s\", reopen errorlog, accesslog",
            ms_signal_toname(signal));

    ms_errlog_reopen();
    ms_acclog_reopen();
}

static void ms_server_rtimeout_handler(ms_event_loop_t *evlop, ms_conn_t *conn)
{
    ms_errlog(MS_ERRLOG_ERR, 0, "clientfd \"%d\" read timeout", conn->fd);
    ms_eventloop_file_del(evlop, conn->fd, MS_EVENTLOOP_ALL);
    ms_socket_close(conn->fd);
}

static void ms_server_stimeout_handler(ms_event_loop_t *evlop, ms_conn_t *conn)
{
    ms_errlog(MS_ERRLOG_ERR, 0, "clientfd \"%d\" send timeout", conn->fd);
    ms_eventloop_file_del(evlop, conn->fd, MS_EVENTLOOP_ALL);
    ms_socket_close(conn->fd);
}

// MS_OK:成功； MS_ERROR:失败，底层会直接关闭该连接
static int ms_server_proce_handler(ms_conn_t *conn, ssize_t recvlen)
{
    char *p = NULL;

    // 处理请求，设置响应
    // TODO
    p = ms_str_snprintf(conn->sebuff, conn->buffsize - 1, http_head, recvlen,
            conn->rebuff);
    conn->sendsize = p - conn->sebuff;
    ms_acclog("fd:%05d %s<->%05d relen:%d selen:%d",
            conn->fd, conn->addr.ip, conn->addr.port,
            recvlen, conn->sendsize);

    // 合法请求
    return MS_OK;

    // 非法请求
    return MS_ERROR;
}
