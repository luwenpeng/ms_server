#include "ms_eventloop.h"

static void ms_eventloop_timer_process(ms_event_loop_t *evlop);

/***********************************************************
 * @Func   : ms_eventloop_create()
 * @Author : lwp
 * @Brief  : 创建 eventloop。
 * @Param  : [in] event_size
 * @Param  : [in] pool_size : 内存池小块内存的大小
 * @Param  : [in] data_size : evlop->files[i]->data 的大小
 * @Return : NULL : 失败
 *           evlop : 成功
 * @Note   : 
 ***********************************************************/
ms_event_loop_t *ms_eventloop_create(int event_size, int pool_size,
        int data_size)
{
    int poolsize = -1;
    int eventsize = -1;
    void *pdata = NULL;
    ms_mem_pool_t *pool = NULL;
    ms_event_loop_t *evlop = NULL;

    // 若 xxx_size 小于默认值则使用默认值
    eventsize = event_size > MS_EVENTS_DEFAULT_SIZE ?
        event_size : MS_EVENTS_DEFAULT_SIZE;
    poolsize = pool_size > MS_EVENTS_DEFAULT_SIZE ?
        pool_size : MS_EVENTS_DEFAULT_SIZE;

    // 创建内存池
    pool = ms_mem_pool_create(poolsize);
    if (NULL == pool)
    {
        goto end;
    }

    // 创建 evlop
    evlop = (ms_event_loop_t *)ms_mem_pool_pcalloc(pool,
            sizeof(ms_event_loop_t));
    if (NULL == evlop)
    {
        goto end;
    }
    evlop->pool = pool;

    // 创建 events 数组
    evlop->events = (ms_event_epoll_t *)ms_mem_pool_pcalloc(evlop->pool,
            eventsize * sizeof(ms_event_epoll_t));
    if (NULL == evlop->events)
    {
        goto end;
    }

    // 创建 files 数组
    evlop->files = (ms_event_file_t *)ms_mem_pool_pcalloc(evlop->pool,
            eventsize * sizeof(ms_event_file_t));
    if (NULL == evlop->files)
    {
        goto end;
    }

    // 创建 evlop->files[i]->data
    if (data_size > 0)
    {
        pdata = ms_mem_pool_pcalloc(evlop->pool, eventsize * data_size);
        if (NULL == pdata)
        {
            goto end;
        }
    }

    // 初始化 files
    for (int i = 0; i < eventsize; i++)
    {
        evlop->files[i].mask = MS_EVENTLOOP_NONE;
        evlop->files[i].rproc = NULL;
        evlop->files[i].wproc = NULL;
        if (data_size > 0)
        {
            evlop->files[i].data = pdata;
            pdata = (char *)pdata + data_size;
        }
        else
        {
            evlop->files[i].data = NULL;
        }
    }

    // 创建定时器管理结构体并初始化
    evlop->timer = (ms_event_rbtree_t *)ms_mem_pool_pcalloc(evlop->pool,
            sizeof(ms_event_rbtree_t));
    evlop->timer->tree = (ms_rbtree_t *)ms_mem_pool_pcalloc(evlop->pool,
            sizeof(ms_rbtree_t));
    evlop->timer->sentinel = (ms_rbtree_node_t *)ms_mem_pool_pcalloc(evlop->pool,
            sizeof(ms_rbtree_node_t));
    if (NULL == evlop->timer || NULL == evlop->timer->tree ||
            NULL == evlop->timer->sentinel)
    {
        goto end;
    }
    ms_rbtree_init(evlop->timer->tree, evlop->timer->sentinel,
            ms_rbtree_insert_timer_value);

    // 创建 epfd
    evlop->epfd = ms_epoll_create(eventsize);
    if (MS_ERROR == evlop->epfd)
    {
        goto end;
    }

    evlop->size = eventsize; // events 与 files 的最大容量
    evlop->stop = 0; // evlop 停止的标志
    evlop->free = NULL; // 初始空闲定时器回收链表
    evlop->data1 = NULL;
    evlop->data2 = NULL;
    evlop->data3 = NULL;
    evlop->data4 = NULL;
    evlop->data5 = NULL;
    evlop->data6 = NULL;
    evlop->data7 = NULL;
    evlop->data8 = NULL;

    ms_errlog(MS_ERRLOG_INFO, 0, ELP_TAG
            "create eventloop \"%p\" size \"%d\" epfd \"%06d\"",
            evlop, evlop->size, evlop->epfd);
    return evlop;

end:
    ms_mem_pool_destory(&pool);
    return NULL;
}
// @ms_eventloop_create() ok

/***********************************************************
 * @Func   : ms_eventloop_destory()
 * @Author : lwp
 * @Brief  : 销毁 eventloop。
 * @Param  : [in] evlop
 * @Return : NULL
 * @Note   : 
 ***********************************************************/
void ms_eventloop_destory(ms_event_loop_t *evlop)
{
    ms_mem_pool_t *pool = NULL;
    ms_event_file_t *file = NULL;

    if (NULL == evlop)
    {
        return;
    }

    ms_errlog(MS_ERRLOG_INFO, 0, ELP_TAG
            "destory eventloop \"%p\" size \"%d\" epfd \"%06d\"",
            evlop, evlop->size, evlop->epfd);

    // 关闭文件句柄
    for (int i = 0; i < evlop->size; i++)
    {
        file = &(evlop->files[i]);
        if (file->mask != MS_EVENTLOOP_NONE)
        {
            close(i);
        }
    }

    // 关闭 epfd
    close(evlop->epfd);

    // 销毁内存池
    pool = evlop->pool;
    ms_mem_pool_destory(&pool);
}
// @ms_eventloop_destory() ok

/***********************************************************
 * @Func   : ms_eventloop_file_add()
 * @Author : lwp
 * @Brief  : 向 eventloop 中添加事件。
 * @Param  : [in] evlop
 * @Param  : [in] sockfd
 * @Param  : [in] mask
 * @Param  : [in] proc
 * @Param  : [in] data
 * @Return : MS_ERROR : 失败
 *           MS_OK    : 成功
 * @Note   : 
 ***********************************************************/
int ms_eventloop_file_add(ms_event_loop_t *evlop, int sockfd,
        uint32_t mask, const ms_event_file_proc *proc, void *data)
{
    int op = -1;
    ms_event_file_t *file = NULL;
    struct epoll_event ev = { 0 };

    // 参数检测
    if (sockfd >= evlop->size)
    {
        ms_errlog(MS_ERRLOG_ERR, 0, ELP_TAG "eventloop event queue full");
        return MS_ERROR;
    }

    // 获取 sockfd 文件句柄对应的数据结构
    file = &(evlop->files[sockfd]);

    // 先注册 epoll
    op = (file->mask == MS_EVENTLOOP_NONE) ? EPOLL_CTL_ADD : EPOLL_CTL_MOD;
    ev.data.fd = sockfd;
    ev.events = file->mask | mask;
    if (ms_epoll_ctl(evlop->epfd, op, sockfd, &ev) == MS_ERROR)
    {
        return MS_ERROR;
    }

    // 后设置 files 列表属性
    file->data = data;
    file->mask = file->mask | mask;
    if (file->mask & EPOLLIN)
    {
        file->rproc = proc;
    }

    if (file->mask & EPOLLOUT)
    {
        file->wproc = proc;
    }

    ms_errlog(MS_ERRLOG_INFO, 0, ELP_TAG
            "add fd \"%06d\" mask \"%010uD\" on epfd \"%06d\", handler \"%p\"",
            sockfd, file->mask, evlop->epfd, proc);

    return MS_OK;
}
// @ms_eventloop_file_add() ok

/***********************************************************
 * @Func   : ms_eventloop_file_mod()
 * @Author : lwp
 * @Brief  : 重置 eventloop 中 sockfd 对应的事件掩码。
 * @Param  : [in] evlop
 * @Param  : [in] sockfd
 * @Param  : [in] mask
 * @Param  : [in] proc
 * @Param  : [in] data
 * @Return : MS_ERROR : 失败
 *           MS_OK    : 成功
 * @Note   : 
 ***********************************************************/
int ms_eventloop_file_mod(ms_event_loop_t *evlop, int sockfd,
        uint32_t mask, const ms_event_file_proc *proc, void *data)
{
    int op = -1;
    ms_event_file_t *file = NULL;
    struct epoll_event ev = { 0 };

    // 参数检测
    if (sockfd >= evlop->size)
    {
        ms_errlog(MS_ERRLOG_ERR, 0, ELP_TAG "eventloop event queue full");
        return MS_ERROR;
    }

    // 获取 sockfd 文件句柄对应的数据结构
    file = &(evlop->files[sockfd]);

    // 先注册 epoll
    op = (file->mask == MS_EVENTLOOP_NONE) ? EPOLL_CTL_ADD : EPOLL_CTL_MOD;
    ev.data.fd = sockfd;
    ev.events = mask;
    if (ms_epoll_ctl(evlop->epfd, op, sockfd, &ev) == MS_ERROR)
    {
        return MS_ERROR;
    }

    // 后设置 files 列表属性
    file->data = data;
    file->mask = mask;
    if (file->mask & EPOLLIN)
    {
        file->rproc = proc;
    }

    if (file->mask & EPOLLOUT)
    {
        file->wproc = proc;
    }

    ms_errlog(MS_ERRLOG_INFO, 0, ELP_TAG
            "mod fd \"%06d\" mask \"%010uD\" on epfd \"%06d\", handler \"%p\"",
            sockfd, file->mask, evlop->epfd, proc);

    return MS_OK;
}
// @ms_eventloop_file_mod() ok

/***********************************************************
 * @Func   : ms_eventloop_file_del()
 * @Author : lwp
 * @Brief  : 从 eventloop 中删除事件。
 * @Param  : [in] evlop
 * @Param  : [in] sockfd
 * @Param  : [in] mask
 * @Return : MS_ERROR : 失败
 *           MS_OK    : 成功
 * @Note   : 
 ***********************************************************/
void ms_eventloop_file_del(ms_event_loop_t *evlop, int sockfd, uint32_t mask)
{
    ms_event_file_t *file = NULL;
    struct epoll_event ev = { 0 };

    // 范围检测
    if (sockfd >= evlop->size)
    {
        return;
    }

    // 状态检测
    file = &(evlop->files[sockfd]);
    if (file->mask == MS_EVENTLOOP_NONE)
    {
        return;
    }

    // 先设置 epoll_ctl()
    ev.data.fd = sockfd;
    ev.events = file->mask & (~mask);
    if (ev.events == MS_EVENTLOOP_NONE)
    {
        ms_epoll_ctl(evlop->epfd, EPOLL_CTL_DEL, sockfd, &ev);
    }
    else
    {
        ms_epoll_ctl(evlop->epfd, EPOLL_CTL_MOD, sockfd, &ev);
    }

    // 后更新 file 文件属性
    file->mask = file->mask & (~mask);

    ms_errlog(MS_ERRLOG_INFO, 0, ELP_TAG
            "del fd \"%06d\" mask \"%010uD\" on epfd \"%06d\"",
            sockfd, mask, evlop->epfd);
}
// @ms_eventloop_file_del() ok

/***********************************************************
 * @Func   : ms_eventloop_timer_add()
 * @Author : lwp
 * @Brief  : 向 eventloop 中添加一个定时器。
 * @Param  : [in] evlop
 * @Param  : [in] ms : 超时时间(毫秒)
 * @Param  : [in] proc : 超时回调函数
 * @Param  : [in] data : 超时回调函数的参数
 * @Return : NULL : 失败
 *           timer : 成功
 * @Note   : 
 ***********************************************************/
ms_event_timer_t *ms_eventloop_timer_add(ms_event_loop_t *evlop, int ms,
        const ms_event_timer_proc *proc, void *data)
{
    ms_event_timer_t *timer = NULL;

    // 从空闲定时器链表中取一个定时器
    if (evlop->free != NULL)
    {
        timer = evlop->free;
        evlop->free = timer->next;
    }
    // 从内存池中分配一个新的定时器
    else
    {
        timer = (ms_event_timer_t *)ms_mem_pool_pcalloc(evlop->pool,
                sizeof(ms_event_timer_t));
        if (NULL == timer)
        {
            return NULL;
        }
    }
    memset(timer, 0, sizeof(ms_event_rbtree_t));

    // 设置定时器属性
    timer->node.key = ms_time_ms() + ms;
    timer->proc = proc;
    timer->data = data;
    timer->next = NULL;

    // 将定时器加入红黑树中
    ms_rbtree_insert(evlop->timer->tree, &(timer->node));

    ms_errlog(MS_ERRLOG_INFO, 0, ELP_TAG
            "add timer \"%p\" on epfd \"%06d\", timeout \"%d\"",
            timer, evlop->epfd, ms);

    return timer;
}
// @ms_eventloop_timer_add() ok

/***********************************************************
 * @Func   : ms_eventloop_timer_del()
 * @Author : lwp
 * @Brief  : 从 eventloop 中删除一个定时器。
 * @Param  : [in] evlop
 * @Param  : [in] timer
 * @Return : NONE
 * @Note   : 
 ***********************************************************/
void ms_eventloop_timer_del(ms_event_loop_t *evlop, ms_event_timer_t *timer)
{
    // 将定时器从红黑树中删除
    ms_rbtree_delete(evlop->timer->tree, &(timer->node));

    // 将定时器加入空闲链表，准备重复利用
    timer->next = evlop->free;
    evlop->free = timer;

    ms_errlog(MS_ERRLOG_INFO, 0, ELP_TAG "del timer \"%p\" on epfd \"%06d\"",
            timer, evlop->epfd);
}
// @ms_eventloop_timer_del() ok

/***********************************************************
 * @Func   : ms_eventloop_main()
 * @Author : lwp
 * @Brief  : eventloop 主循环。
 * @Param  : [in] evlop
 * @Param  : [in] maxtimeout : epoll_wait() 最大阻塞时间
 * @Return : NONE
 * @Note   : 
 ***********************************************************/
void ms_eventloop_main(ms_event_loop_t *evlop, int maxtimeout)
{
    int nfds;
    uint32_t mask;
    int sockfd;
    int timeout;
    ms_event_file_t *file;
    ms_rbtree_node_t *rbtree_node;

    if (maxtimeout < 0)
    {
        maxtimeout = -1;
    }

    while (!evlop->stop)
    {
        timeout = maxtimeout;

        // 若红黑树中存在定时器取最小的超时时间作为 timeout 的值
        if (evlop->timer->tree->root != evlop->timer->tree->sentinel)
        {
            rbtree_node = ms_rbtree_min(evlop->timer->tree->root,
                    evlop->timer->tree->sentinel);

            // timeout = -1
            if (timeout < 0)
            {
                // 可正可负
                timeout = rbtree_node->key - ms_time_ms();
            }
            // timeout >= 0
            else
            {
                // 可正可负
                timeout = ms_min((int64_t)(rbtree_node->key - ms_time_ms()),
                    (int64_t)maxtimeout);
            }

            // 当前事件已经超时，epoll_wait() 立即返回
            if (timeout < 0)
            {
                timeout = 0;
            }
        }
        /*
        else
        {
            // 使用 maxtimeout 作为最大超时时间
            // 若 maxtimeout = 0; => timeout =  0; => 立即返回
            // 若 maxtimeout > 0; => timeout >  0; => 阻塞 timeout 毫秒
            // 若 maxtimeout < 0; => timeout = -1; => 永久阻塞
        }
        */

        ms_errlog(MS_ERRLOG_INFO, 0, ELP_TAG "use timeout \"%d\"", timeout);

        nfds = ms_epoll_wait(evlop->epfd, evlop->events, evlop->size, timeout);
        // 处理读写事件
        for (int i = 0; i < nfds; i++)
        {
            sockfd = evlop->events[i].data.fd;
            mask = evlop->events[i].events;

            file = &(evlop->files[sockfd]);
            ms_errlog(MS_ERRLOG_INFO, 0, ELP_TAG
                    "get fd \"%06d\" mask \"%010uD\" on epfd \"%06d\"",
                    sockfd, mask, evlop->epfd);

            if (file->mask & mask & EPOLLIN)
            {
                ms_errlog(MS_ERRLOG_INFO, 0, ELP_TAG "run rproc() \"%p\"",
                        file->rproc);
                file->rproc(evlop, sockfd, mask, file->data);
            }

            if (file->mask & mask & EPOLLOUT)
            {
                ms_errlog(MS_ERRLOG_INFO, 0, ELP_TAG "run wproc() \"%p\"",
                        file->wproc);
                file->wproc(evlop, sockfd, mask, file->data);
            }
        }

        // 处理超时事件
        ms_eventloop_timer_process(evlop);
    }

    ms_errlog(MS_ERRLOG_DEBUG, 0, ELP_TAG "stop eventloop");
}
// @ms_eventloop_main() ok

/***********************************************************
 * @Func   : ms_eventloop_stop()
 * @Author : lwp
 * @Brief  : 停止 eventloop。
 * @Param  : [in] NONE
 * @Return : NONE
 * @Note   : 
 ***********************************************************/
void ms_eventloop_stop(ms_event_loop_t *evlop)
{
    evlop->stop = 1;
}
// ms_eventloop_stop() ok

/***********************************************************
 * @Func   : ms_eventloop_timer_process()
 * @Author : lwp
 * @Brief  : 处理超时事件。
 * @Param  : [in] evlop
 * @Return : NONE
 * @Note   : 
 ***********************************************************/
static void ms_eventloop_timer_process(ms_event_loop_t *evlop)
{
    ms_rbtree_node_t *rbtree_node = NULL;
    ms_event_timer_t *timer = NULL;
    uint32_t i = 0;

    while (evlop->timer->tree->root != evlop->timer->tree->sentinel)
    {
        rbtree_node = ms_rbtree_min(evlop->timer->tree->root,
                evlop->timer->tree->sentinel);
        if (rbtree_node->key <= ms_time_ms())
        {
            timer = (ms_event_timer_t *)rbtree_node;

            // 将定时器从 evlop 中删除
            ms_eventloop_timer_del(evlop, timer);

            ms_errlog(MS_ERRLOG_INFO, 0,
                    ELP_TAG "run timer \"%p\" on epfd \"%06d\"",
                    timer, evlop->epfd);

            // 处理超时事件
            timer->proc(evlop, timer->data);
            if ((i++) > MS_MAX_PROCE_TIMEOUT_EVENTS_PER_LOOP)
            {
                ms_errlog(MS_ERRLOG_ERR, 0,
                    ELP_TAG "timeout events too much(perloop > %D), please check config",
                    MS_MAX_PROCE_TIMEOUT_EVENTS_PER_LOOP);
                break;
            }
        }
        else
        {
            // 剩余的定时器未超时
            break;
        }
    }
}
// @ms_eventloop_timer_process() ok
