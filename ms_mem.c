#include "ms_mem.h"

static void *ms_mem_pool_pcalloc_large(ms_mem_pool_t *pool, size_t size);
static void *ms_mem_pool_pcalloc_small(ms_mem_pool_t *pool, size_t size);

/***********************************************************
 * @Func   : ms_mem_pool_create()
 * @Author : lwp
 * @Brief  : 创建内存池。
 * @Param  : [in] size : 小块内存可分配的最大内存
 * @Return : NULL : 失败
 *           pool : 成功
 * @Note   : 
 ***********************************************************/
ms_mem_pool_t *ms_mem_pool_create(size_t size)
{
    int rev;
    size_t nalloc;
    ms_mem_pool_t *pool = NULL;

    nalloc = size + sizeof(ms_mem_pool_t);
    rev = posix_memalign((void **)&pool, MS_MEM_POOL_ALIGNMENT, nalloc);
    if (rev)
    {
        ms_errlog(MS_ERRLOG_ERR, rev, MEM_TAG "posix_memalign() failed");
        return NULL;
    }

    memset(pool, 0, nalloc);
    pool->small.last = (char *)pool + sizeof(ms_mem_pool_t);
    pool->small.end = (char *)pool + nalloc;
    pool->small.next = NULL;
    pool->small.failed = 0;

    pool->large = NULL;
    pool->current = pool;
    pool->max = size;

    return pool;
}
// @ms_mem_pool_create() ok

/***********************************************************
 * @Func   : ms_mem_pool_pcalloc()
 * @Author : lwp
 * @Brief  : 从内存池 pool 以内存对齐方式分配并初始化 size 字节的内存。
 * @Param  : [in] pool
 * @Param  : [in] size
 * @Return : NULL : 失败
 *           !NULL : 成功
 * @Note   : 
 ***********************************************************/
void *ms_mem_pool_pcalloc(ms_mem_pool_t *pool, size_t size)
{
    // 若申请的 size 小于 pool->max 则从小块内存链中分配
    if (size <= pool->max)
    {
        return ms_mem_pool_pcalloc_small(pool, size);
    }
    // 若申请的 size 大于 pool->max 则从大块内存链中分配
    else
    {
        return ms_mem_pool_pcalloc_large(pool, size);
    }
}
// @ms_mem_pool_pcalloc() ok

/***********************************************************
 * @Func   : ms_mem_pool_destory()
 * @Author : lwp
 * @Brief  : 销毁 pool 内存池。
 * @Param  : [in] pool
 * @Return : NONE
 * @Note   : 
 ***********************************************************/
void ms_mem_pool_destory(ms_mem_pool_t **pool_ptr)
{
    ms_mem_pool_small_t *d, *n;
    ms_mem_pool_large_t *l;
    ms_mem_pool_t *pool = *pool_ptr;

    if (NULL == pool)
    {
        return;
    }

    // 释放大块内存的存储空间
    for (l = pool->large; l; l = l->next)
    {
        if (l->alloc)
        {
            free(l->alloc);
            l->alloc = NULL;
        }
    }

    // 释放大块内存的管理结构体，释放小块内存存储空间和管理结构体
    for (d = &(pool->small), n = d->next; /* void */; d = n, n = d->next)
    {
        free(d);
        d = NULL;

        if (n == NULL)
        {
            break;
        }
    }

    *pool_ptr = NULL;
}
// @ms_mem_pool_destory() ok

/***********************************************************
 * @Func   : ms_mem_pool_reset()
 * @Author : lwp
 * @Brief  : 重置 pool 内存池。
 * @Param  : [in] pool
 * @Return : NONE
 * @Note   : 
 ***********************************************************/
void ms_mem_pool_reset(ms_mem_pool_t *pool)
{
    ms_mem_pool_large_t *l;
    ms_mem_pool_small_t *d;

    // 释放大块内存的存储空间
    for (l = pool->large; l; l = l->next)
    {
        if (l->alloc)
        {
            free(l->alloc);
            l->alloc = NULL;
        }
    }

    // 重置小块内存的指针位置
    pool->small.last = (char *)pool + sizeof(ms_mem_pool_t);
    pool->small.failed = 0;
    pool->current = pool;
    pool->large = NULL;
    for (d = pool->small.next; d; d = d->next)
    {
        d->last = (char *)d + sizeof(ms_mem_pool_small_t);
        d->failed = 0;
    }
}
// @ms_mem_pool_reset() ok

/***********************************************************
 * @Func   : ms_mem_pool_test()
 * @Author : lwp
 * @Brief  : 打印内存池堆栈信息。
 * @Param  : [in] pool
 * @Return : NONE
 * @Note   : 
 ***********************************************************/
void ms_mem_pool_test(ms_mem_pool_t *pool)
{
    ms_mem_pool_small_t *d;
    ms_mem_pool_large_t *l;

    printf("[----------- pool infor -----------]\n"
           "sizeof(ms_mem_pool_t)       : \"%lud\"\n"
           "sizeof(ms_mem_pool_small_t) : \"%lud\"\n"
           "sizeof(ms_mem_pool_large_t) : \"%lud\"\n"
           "  pool           : \"%p\"\n"
           "  pool->current  : \"%p\"\n"
           "  pool->max      : \"%lud\"\n",
           sizeof(ms_mem_pool_t),
           sizeof(ms_mem_pool_small_t),
           sizeof(ms_mem_pool_large_t),
           pool,
           pool->current,
           pool->max);

    for (d = &(pool->small); d; d = d->next)
    {
        printf("      ->small    : \"%p\"\n"
               "        ->last   : \"%p\"\n"
               "        ->end    : \"%p\"\n"
               "        ->used   : \"%lud\"\n"
               "        ->unused : \"%lud\"\n"
               "        ->failed : \"%lud\"\n"
               "        ->next   : \"%p\"\n",
               d,
               d->last,
               d->end,
               (pool->max - (d->end - d->last)),
               (d->end - d->last),
               d->failed,
               d->next);
    }

    for (l = pool->large; l; l = l->next)
    {
        printf("      ->large   : \"%p\"\n"
               "        ->alloc : \"%p\"\n"
               "        ->next  : \"%p\"\n",
               l,
               l->alloc,
               l->next);
    }
    printf("[----------------------------------]\n");
}
// @ms_mem_pool_test() ok

/***********************************************************
 * @Func   : ms_mem_pool_pcalloc_small()
 * @Author : lwp
 * @Brief  : 从内存池 pool 小块内存中以内存对齐方式分配并初始化 size 字节的内存。
 * @Param  : [in] pool
 * @Param  : [in] size
 * @Return : NULL : 失败
 *           !NULL : 成功
 * @Note   : 
 ***********************************************************/
static void *ms_mem_pool_pcalloc_small(ms_mem_pool_t *pool, size_t size)
{
    int rev;
    size_t nalloc;
    void *p = NULL;
    ms_mem_pool_small_t *d = NULL;
    ms_mem_pool_small_t *n = NULL;

    // 从已经存在的小块内存链中查找空闲内存
    d = (ms_mem_pool_small_t *)pool->current;
    do
    {
        // 内存对齐
        p = align_ptr(d->last, MS_MEM_POOL_ALIGNMENT);

        // 若当前内存块空闲内存充足
        if ((size_t)(d->end - (char *)p) >= size)
        {
            d->last = (char *)p + size;
            memset(p, 0, size);
            ms_errlog(MS_ERRLOG_DEBUG, 0, MEM_TAG
                    "alloc mem form a existed small mem block");
            return p;
        }
        // 若当前内存块内存不足，则移动到下一个内存块
        else
        {
            ms_errlog(MS_ERRLOG_DEBUG, 0, MEM_TAG
                    "try alloc form next small mem block");
            d = d->next;
        }
    } while (d);

    // 当前小块内存链中内存不足，新开辟一个小块内存
    ms_errlog(MS_ERRLOG_DEBUG, 0, MEM_TAG "try alloc form new small mem block");

    nalloc = sizeof(ms_mem_pool_small_t) + pool->max;
    rev = posix_memalign((void **)&n, MS_MEM_POOL_ALIGNMENT, nalloc);
    if (rev)
    {
        ms_errlog(MS_ERRLOG_ERR, rev, MEM_TAG "posix_memalign() failed");
        return NULL;
    }

    memset(n, 0, nalloc);
    n->last = (char *)n + sizeof(ms_mem_pool_small_t);
    n->end = (char *)n + nalloc;
    n->next = NULL;
    n->failed = 0;

    // 分配第 8 个 ms_mem_pool_small_t 时(包括 pool->small)，开始移动 current。
    // 第 6 次分配失败，current 才会后移，每次 current 后移一个 ms_mem_pool_small_t 结点
    for (d = (ms_mem_pool_small_t *)pool->current; d->next; d = d->next)
    {
        if (d->failed++ > 4)
        {
            pool->current = d->next;
            ms_errlog(MS_ERRLOG_DEBUG, 0, MEM_TAG "change current point");
        }
    }
    // 将新内存块插入到小块内存链表的尾部
    d->next = n;

    // 内存对齐后空间不足
    p = align_ptr(n->last, MS_MEM_POOL_ALIGNMENT);
    if ((size_t)(n->end - (char *)p) < size)
    {
        ms_errlog(MS_ERRLOG_ERR, 0,
                MEM_TAG "after memory alignment, memory not enough");
        return NULL;
    }
    // 内存对齐后内存充足
    else
    {
        ms_errlog(MS_ERRLOG_DEBUG, 0,
                MEM_TAG "after memory alignment, memory enough");
        n->last = (char *)p + size;
        return p;
    }
}
// @ms_mem_pool_pcalloc_small() ok

/***********************************************************
 * @Func   : ms_mem_pool_pcalloc_large()
 * @Author : lwp
 * @Brief  : 从内存池 pool 大块内存中以内存对齐方式分配并初始化 size 字节的内存
 * @Param  : [in] pool
 * @Param  : [in] size
 * @Return : NULL : 失败
 *           !NULL : 成功
 * @Note   : 
 ***********************************************************/
static void *ms_mem_pool_pcalloc_large(ms_mem_pool_t *pool, size_t size)
{
    int rev;
    void *t;
    void *p;
    ms_mem_pool_large_t *l;

    rev = posix_memalign(&t, MS_MEM_POOL_ALIGNMENT, size);
    if (rev)
    {
        ms_errlog(MS_ERRLOG_ERR, rev, MEM_TAG "posix_memalign() failed");
        return NULL;
    }
    memset(t, 0, size);

    ms_errlog(MS_ERRLOG_DEBUG, 0, MEM_TAG
            "try alloc large struct form small mem block");

    // 从小块内存中为大块内存链分配管理结构体
    p = ms_mem_pool_pcalloc_small(pool, sizeof(ms_mem_pool_large_t));
    if (NULL == p)
    {
        free(t);
        return NULL;
    }

    // 插入大块内存链首部
    l = (ms_mem_pool_large_t *)p;
    l->alloc = (char *)t;
    l->next = pool->large;
    pool->large = l;

    return t;
}
// @ms_mem_pool_pcalloc_large() ok
