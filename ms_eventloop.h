// 事件循环: 每个 eventloop 拥有单独的内存池，拥有单独的定时器
#ifndef _MS_EVENTLOOP_H
#define _MS_EVENTLOOP_H

#ifdef __cpluscplus
extern "C"
{
#endif

#include "ms_head.h"
#include "ms_conf.h"

#include "ms_mem.h"    // 内存池
#include "ms_epoll.h"  // epoll 异步 I/O
#include "ms_rbtree.h" // 定时器

#define MS_EVENTS_DEFAULT_SIZE 1024
#define MS_MAX_PROCE_TIMEOUT_EVENTS_PER_LOOP 100000

#define MS_EVENTLOOP_NONE 0
#define MS_EVENTLOOP_ALL  0xffffffff

#define ELP_TAG "[EVENTLOOP] "

typedef struct epoll_event       ms_event_epoll_t;
typedef struct ms_event_loop_s   ms_event_loop_t;
typedef struct ms_event_file_s   ms_event_file_t;
typedef struct ms_event_timer_s  ms_event_timer_t;
typedef struct ms_event_rbtree_s ms_event_rbtree_t;

typedef void ms_event_timer_proc(ms_event_loop_t *evlop, void *data);
typedef void ms_event_file_proc(ms_event_loop_t *evlop, int sockfd,
        uint32_t mask, void *data);

// 事件结构体
struct ms_event_file_s {
    uint32_t            mask;  // 事件掩码
    ms_event_file_proc *rproc; // 处理可读事件的回调函数
    ms_event_file_proc *wproc; // 处理可写事件的回调函数
    void               *data;  // 回调函数的 data 参数
};

// 红黑树管理结构体
struct ms_event_rbtree_s {
    ms_rbtree_t      *tree;     // 根结点
    ms_rbtree_node_t *sentinel; // 哨兵结点
};

// 定时器结点
struct ms_event_timer_s {
    ms_rbtree_node_t     node; // 红黑树结点
    ms_event_timer_proc *proc; // 处理超时事件的回调函数
    ms_event_timer_t    *next; // 下一个定时器结点，空闲红黑树结点以链表存储
    void                *data; // 回调函数的 data 参数
};

// eventloop 结构体
struct ms_event_loop_s {
    ms_mem_pool_t     *pool;   // 内存池指针
    ms_event_file_t   *files;  // 存储文件属性的数组
    ms_event_epoll_t  *events; // 存储注册事件的数组
    ms_event_rbtree_t *timer;  // 定时器管理结构体
    ms_event_timer_t  *free;   // 存储空闲定时器结点的链表
    int                epfd;   // epfd 文件句柄
    int                size;   // events 与 files 的最大容量
    int                stop;   // eventloop 停止的标志
    void              *data1;  // 待定
    void              *data2;  // 待定
    void              *data3;  // 待定
    void              *data4;  // 待定
    void              *data5;  // 待定
    void              *data6;  // 待定
    void              *data7;  // 待定
    void              *data8;  // 待定
};

ms_event_loop_t *ms_eventloop_create(int event_size, int pool_size,
        int data_size);
void ms_eventloop_destory(ms_event_loop_t *evlop);

int ms_eventloop_file_add(ms_event_loop_t *evlop, int sockfd,
        uint32_t mask, const ms_event_file_proc *proc, void *data);
int ms_eventloop_file_mod(ms_event_loop_t *evlop, int sockfd,
        uint32_t mask, const ms_event_file_proc *proc, void *data);
void ms_eventloop_file_del(ms_event_loop_t *evlop, int sockfd, uint32_t mask);

ms_event_timer_t *ms_eventloop_timer_add(ms_event_loop_t *evlop, int ms,
        const ms_event_timer_proc *proc, void *data);
void ms_eventloop_timer_del(ms_event_loop_t *evlop, ms_event_timer_t *timer);

void ms_eventloop_main(ms_event_loop_t *evlop, int maxtimeout);
void ms_eventloop_stop(ms_event_loop_t *evlop);

#ifdef __cpluscplus
}
#endif

#endif
