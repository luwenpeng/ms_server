#include "ms_epoll.h"

/***********************************************************
 * @Func   : ms_epoll_create()
 * @Author : lwp
 * @Brief  : 创建 epollfd。
 * @Param  : [in] size
 * @Return : MS_ERROR : 失败
 *           epollfd : 成功
 * @Note   : 
 ***********************************************************/
int ms_epoll_create(int size)
{
    int epollfd = -1;

    epollfd = epoll_create(size);
    if (epollfd == -1)
    {
        ms_errlog(MS_ERRLOG_ERR, errno, "epoll_create() failed");
        return MS_ERROR;
    }

    return epollfd;
}
// @ms_epoll_create() ok

/***********************************************************
 * @Func   : ms_epoll_ctl()
 * @Author : lwp
 * @Brief  : 对 fd 执行操作。
 * @Param  : [in] epfd
 * @Param  : [in] op
 * @Param  : [in] fd
 * @Param  : [in] event
 * @Return : MS_ERROR : 失败
 *           MS_OK    : 成功
 * @Note   : 
 ***********************************************************/
int ms_epoll_ctl(int epfd, int op, int fd, struct epoll_event *event)
{
    if (epoll_ctl(epfd, op, fd, event) == -1)
    {
        ms_errlog(MS_ERRLOG_ERR, errno,
                "epoll_ctl() op \"%d\" sockfd \"%d\" on epfd \"%d\" failed",
                op, fd, epfd);
        return MS_ERROR;
    }

    return MS_OK;
}
// @ms_epoll_ctl() ok

/***********************************************************
 * @Func   : ms_epoll_wait()
 * @Author : lwp
 * @Brief  : 阻塞等待关注的事件发生。
 * @Param  : [in] epfd
 * @Param  : [in] events
 * @Param  : [in] maxevents
 * @Param  : [in] timeout
 * @Return : MS_ERROR : 失败
 *           nfds     : 成功
 * @Note   : nfds 为 0 代表超时
 ***********************************************************/
int ms_epoll_wait(int epfd, struct epoll_event *events, int maxevents,
        int timeout)
{
    int nfds;

    nfds = epoll_wait(epfd, events, maxevents, timeout);
    if (nfds == -1)
    {
        ms_errlog(MS_ERRLOG_ERR, errno, "epoll_wait() on epfd \"%d\" failed",
                epfd);
        return MS_ERROR;
    }

    ms_errlog(MS_ERRLOG_INFO, 0, "epoll_wait() on epfd \"%d\" return \"%d\"",
            epfd, nfds);

    return nfds;
}
// @ms_epoll_wait() ok


/***********************************************************
 * @Func   : ms_epoll_close()
 * @Author : lwp
 * @Brief  : 关闭 epollfd 文件句柄。
 * @Param  : [in] epfd
 * @Return : NONE
 * @Note   : 
 ***********************************************************/
void ms_epoll_close(int epfd)
{
    if (epfd > 0)
    {
        close(epfd);
    }
}
// @ms_epoll_close() ok
