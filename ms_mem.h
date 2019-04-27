// 原则：1.每次内存分配均进行内存对齐
//       2.每次内存分配均进行初始化
//       3.小块内存在重置内存池时被复用
//       4.大块内存在重置内存池时被释放

#ifndef _MS_MEM_H
#define _MS_MEM_H

#ifdef __cpluscplus
extern "C"
{
#endif

#include "ms_head.h"
#include "ms_conf.h"

#include "ms_errlog.h"

#define MS_MEM_POOL_ALIGNMENT 16 // 必须是 2 的幂和 sizeof(void *) 的倍数
#define MS_MEM_POOL_DEFAULT_SIZE 4096
#define MEM_TAG "[MEM] "

// 内存对齐
#define align_ptr(p, a) \
    (char *) (((intptr_t) (p) + ((intptr_t) a - 1)) & ~((intptr_t) a - 1))

// 大块内存管理结构体
typedef struct ms_mem_pool_large_s ms_mem_pool_large_t;
struct ms_mem_pool_large_s {
    char                *alloc; // 指向大块内存的指针
    ms_mem_pool_large_t *next;  // 下一个大块内存管理结构体
};

// 小块内存管理结构体
typedef struct ms_mem_pool_small_s ms_mem_pool_small_t;
struct ms_mem_pool_small_s {
    char                *last;   // 可用内存起始地址
    char                *end;    // 可用内存终止地址
    ms_mem_pool_small_t *next;   // 下一个小块内存管理结构
    size_t               failed; // 当前内存块分配失败的次数
};

// 内存池管理结构体
typedef struct ms_mem_pool_s {
    ms_mem_pool_small_t  small;   // 小块内存
    ms_mem_pool_large_t *large;   // 大内存块
    void                *current; // 当前可用小块内存
    size_t               max;     // 小内存块可分配的最大内存
    size_t               temp;    // 无效字段，用于内存对齐
} ms_mem_pool_t;

ms_mem_pool_t *ms_mem_pool_create(size_t size);
void ms_mem_pool_destory(ms_mem_pool_t **pool_ptr);

void *ms_mem_pool_pcalloc(ms_mem_pool_t *pool, size_t size);
void ms_mem_pool_reset(ms_mem_pool_t *pool);

void ms_mem_pool_test(ms_mem_pool_t *pool);

#ifdef __cpluscplus
}
#endif

#endif
