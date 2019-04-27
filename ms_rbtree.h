#ifndef _MS_RBTREE_H
#define _MS_RBTREE_H

#ifdef __cpluscplus
extern "C"
{
#endif

#include "ms_head.h"
#include "ms_conf.h"

#define ms_rbt_red(node)              ((node)->color = 1)
#define ms_rbt_black(node)            ((node)->color = 0)
#define ms_rbt_is_red(node)           ((node)->color)
#define ms_rbt_is_black(node)         (!ms_rbt_is_red(node))
#define ms_rbt_copy_color(n1, n2)     (n1->color = n2->color)
#define ms_rbtree_sentinel_init(node) ms_rbt_black(node)

typedef struct ms_rbtree_s ms_rbtree_t;
typedef struct ms_rbtree_node_s ms_rbtree_node_t;

typedef void (*ms_rbtree_insert_pt) (ms_rbtree_node_t *root,
        ms_rbtree_node_t *node, ms_rbtree_node_t *sentinel);

// 红黑树结点
struct ms_rbtree_node_s {
    ms_rbtree_node_t *left;   // 左孩子
    ms_rbtree_node_t *right;  // 右孩子
    ms_rbtree_node_t *parent; // 父结点
    uintptr_t         key;    // key
    char              color;  // 颜色
    char             *data;   // 数据
};

// 红黑树结构体
struct ms_rbtree_s {
    ms_rbtree_node_t   *root;     // 根结点
    ms_rbtree_node_t   *sentinel; // 哨兵结点
    ms_rbtree_insert_pt insert;   // 插入结点时用的排序方式
};

void ms_rbtree_init(ms_rbtree_t *tree, ms_rbtree_node_t *sentinel,
        ms_rbtree_insert_pt insert);

void ms_rbtree_insert(ms_rbtree_t *tree, ms_rbtree_node_t *node);
void ms_rbtree_delete(ms_rbtree_t *tree, ms_rbtree_node_t *node);

void ms_rbtree_insert_value(ms_rbtree_node_t *t, ms_rbtree_node_t *n,
        ms_rbtree_node_t *sentinel);
void ms_rbtree_insert_timer_value(ms_rbtree_node_t *t, ms_rbtree_node_t *n,
        ms_rbtree_node_t *sentinel);

ms_rbtree_node_t *ms_rbtree_min(ms_rbtree_node_t *node,
        ms_rbtree_node_t *sentinel);
ms_rbtree_node_t *ms_rbtree_max(ms_rbtree_node_t *node,
        ms_rbtree_node_t *sentinel);

ms_rbtree_node_t *ms_rbtree_next_up(ms_rbtree_t *tree, ms_rbtree_node_t *node);
ms_rbtree_node_t *ms_rbtree_next_down(ms_rbtree_t *tree, ms_rbtree_node_t *node);

#ifdef __cpluscplus
}
#endif

#endif
