#include "ms_rbtree.h"

static inline void ms_rbtree_left_rotate(ms_rbtree_node_t **root,
        ms_rbtree_node_t *sentinel, ms_rbtree_node_t *node);
static inline void ms_rbtree_right_rotate(ms_rbtree_node_t **root,
        ms_rbtree_node_t *sentinel, ms_rbtree_node_t *node);

/***********************************************************
 * @Func   : ms_rbtree_init()
 * @Author : lwp
 * @Brief  : 初始化红黑树。
 * @Param  : [in] tree : 红黑树
 * @Param  : [in] sentinel : 哨兵结点
 * @Param  : [in] insert : 插入新结点时使用的排序方式
 * @Return : NONE
 * @Note   : 
 ***********************************************************/
void ms_rbtree_init(ms_rbtree_t *tree, ms_rbtree_node_t *sentinel,
        ms_rbtree_insert_pt insert)
{
    ms_rbtree_sentinel_init(sentinel);
    tree->root = sentinel;
    tree->sentinel = sentinel;
    tree->insert = insert;
}
// @ms_rbtree_init() ok

/***********************************************************
 * @Func   : ms_rbtree_insert()
 * @Author : lwp
 * @Brief  : 向红黑树 tree 中插入新结点 node。
 * @Param  : [in] tree
 * @Param  : [in] node
 * @Return : NONE
 * @Note   : 
 *         + 无
 * 插入后  |
 * 新结点  |                                   + 插入左支 <------+
 * 是否有 -+     + 父黑                        |                 |
 * 父结点  |     |                   + 叔为黑 -+                 |
 *         |     |                   |         |                 |
 *         + 有 -+       + 父位左支 -+         + 插入右支 <------+
 *               |       |           |                           |
 *               |       |           + 叔为红 <-------+          |
 *               + 父红 -+                            | (相同)   + (互为镜像)
 *                       |           + 叔为红 <-------+          |
 *                       |           |                           |
 *                       + 父位右支 -+         + 插入左支 <------+
 *                                   |         |                 |
 *                                   + 叔为黑 -+                 |
 *                                             |                 |
 *                                             + 插入右支 <------+
 ***********************************************************/
void ms_rbtree_insert(ms_rbtree_t *tree, ms_rbtree_node_t *node)
{
    ms_rbtree_node_t **root, *temp, *sentinel;

    root = &tree->root;
    sentinel = tree->sentinel;

    // 当红黑树为空时，新插入的结点作为根结点，直接涂黑
    if (*root == sentinel)
    {
        node->parent = NULL;
        node->left = sentinel;
        node->right = sentinel;
        ms_rbt_black(node);
        *root = node;
        return;
    }

    // 按照自定义的排序方式，将 node 结点插入红黑树中的指定位置，并涂红
    tree->insert(*root, node, sentinel);

    /* 插入新结点前该红黑树为平衡状态，插入新结点后要重新平衡红黑树
     * 当新插入结点的父结点是红色时 */
    while (node != *root && ms_rbt_is_red(node->parent))
    {
        // 父结点位于左支时
        if (node->parent == node->parent->parent->left)
        {
            temp = node->parent->parent->right;

            // 叔叔结点为红时
            if (ms_rbt_is_red(temp))
            {
                ms_rbt_black(node->parent);
                ms_rbt_black(temp);
                ms_rbt_red(node->parent->parent);
                node = node->parent->parent;
            }
            // 叔叔结点为黑时
            else
            {
                // 当插入右支时，先对父结点进行左旋
                if (node == node->parent->right)
                {
                    node = node->parent;
                    ms_rbtree_left_rotate(root, sentinel, node);
                }

                ms_rbt_black(node->parent);
                ms_rbt_red(node->parent->parent);
                ms_rbtree_right_rotate(root, sentinel, node->parent->parent);
            }
        }
        // 父结点位于右支时
        else
        {
            temp = node->parent->parent->left;

            // 叔叔结点为红时
            if (ms_rbt_is_red(temp))
            {
                ms_rbt_black(node->parent);
                ms_rbt_black(temp);
                ms_rbt_red(node->parent->parent);
                node = node->parent->parent;
            }
            // 叔叔结点为黑时
            else
            {
                // 当插入左支时，先对父结点进行右旋
                if (node == node->parent->left)
                {
                    node = node->parent;
                    ms_rbtree_right_rotate(root, sentinel, node);
                }

                ms_rbt_black(node->parent);
                ms_rbt_red(node->parent->parent);
                ms_rbtree_left_rotate(root, sentinel, node->parent->parent);
            }
        }
    }

    // 根结点为黑
    ms_rbt_black(*root);
}
// @ms_rbtree_insert()

/***********************************************************
 * @Func   : ms_rbtree_delete()
 * @Author : lwp
 * @Brief  : 从红黑树 tree 中删除 node 结点。
 * @Param  : [in] tree
 * @Param  : [in] node
 * @Return : NONE
 * @Note   : 
 *           subst : 删除的结点
 *           temp  : 补位的结点
 ***********************************************************/
void ms_rbtree_delete(ms_rbtree_t *tree, ms_rbtree_node_t *node)
{
    char red;
    ms_rbtree_node_t **root, *sentinel, *subst, *temp, *w;

    root = &tree->root;
    sentinel = tree->sentinel;

    // node 无左子，右子补位
    if (node->left == sentinel)
    {
        subst = node;
        temp = node->right;
    }
    // node 无右子，左子补位
    else if (node->right == sentinel)
    {
        subst = node;
        temp = node->left;
    }
    // node 两子均在，后继结点补位
    else
    {
        subst = ms_rbtree_min(node->right, sentinel);
        if (subst->left != sentinel)
        {
            temp = subst->left;
        }
        else
        {
            temp = subst->right;
        }
    }

    // 若要删除的为根结点
    if (subst == *root)
    {
        *root = temp;
        ms_rbt_black(temp);

        node->left = NULL;
        node->right = NULL;
        node->parent = NULL;
        node->key = 0;

        return;
    }

    red = ms_rbt_is_red(subst);

    // 设置 p[subst] 到 temp 的指针
    if (subst == subst->parent->left)
    {
        subst->parent->left = temp;
    }
    else
    {
        subst->parent->right = temp;
    }

    // 设置 p[temp] 到 p[subst] 的指针
    if (subst == node)
    {
        temp->parent = subst->parent;
    }
    else
    {
        // subst 为 node 右子
        if (subst->parent == node)
        {
            temp->parent = subst;
        }
        else
        {
            temp->parent = subst->parent;
        }

        // subst 替代 node 的指针关系
        subst->left = node->left;
        subst->right = node->right;
        subst->parent = node->parent;
        ms_rbt_copy_color(subst, node);

        // 设置 p[node] 到 subst 的指针
        if (node == *root)
        {
            *root = subst;
        }
        else
        {
            if (node == node->parent->left)
            {
                node->parent->left = subst;
            }
            else
            {
                node->parent->right = subst;
            }
        }

        // 设置 p[l[subst]] 到 subst 的指针
        if (subst->left != sentinel)
        {
            subst->left->parent = subst;
        }

        // 设置 p[r[subst]] 到 subst 的指针
        if (subst->right != sentinel)
        {
            subst->right->parent = subst;
        }
    }

    node->left = NULL;
    node->right = NULL;
    node->parent = NULL;
    node->key = 0;

    // 删除红色结点不影响红黑树的性质
    if (red)
    {
        return;
    }

    while (temp != *root && ms_rbt_is_black(temp))
    {
        // 位于左支
        if (temp == temp->parent->left)
        {
            w = temp->parent->right;

            // 兄弟结点为红
            if (ms_rbt_is_red(w))
            {
                ms_rbt_black(w);
                ms_rbt_red(temp->parent);
                ms_rbtree_left_rotate(root, sentinel, temp->parent);
                w = temp->parent->right;
            }

            // 兄弟结点两子为黑
            if (ms_rbt_is_black(w->left) && ms_rbt_is_black(w->right))
            {
                ms_rbt_red(w);
                temp = temp->parent;
            }
            else
            {
                // 兄弟结点右子为黑
                if (ms_rbt_is_black(w->right))
                {
                    ms_rbt_black(w->left);
                    ms_rbt_red(w);
                    ms_rbtree_right_rotate(root, sentinel, w);
                    w = temp->parent->right;
                }

                ms_rbt_copy_color(w, temp->parent);
                ms_rbt_black(temp->parent);
                ms_rbt_black(w->right);
                ms_rbtree_left_rotate(root, sentinel, temp->parent);
                temp = *root;
            }
        }
        // 位于右支
        else
        {
            w = temp->parent->left;

            // 兄弟结点为红
            if (ms_rbt_is_red(w))
            {
                ms_rbt_black(w);
                ms_rbt_red(temp->parent);
                ms_rbtree_right_rotate(root, sentinel, temp->parent);
                w = temp->parent->left;
            }

            // 兄弟结点两子为黑
            if (ms_rbt_is_black(w->left) && ms_rbt_is_black(w->right))
            {
                ms_rbt_red(w);
                temp = temp->parent;
            }
            else
            {
                // 兄弟结点左子为黑
                if (ms_rbt_is_black(w->left))
                {
                    ms_rbt_black(w->right);
                    ms_rbt_red(w);
                    ms_rbtree_left_rotate(root, sentinel, w);
                    w = temp->parent->left;
                }

                ms_rbt_copy_color(w, temp->parent);
                ms_rbt_black(temp->parent);
                ms_rbt_black(w->left);
                ms_rbtree_right_rotate(root, sentinel, temp->parent);
                temp = *root;
            }
        }
    }

    ms_rbt_black(temp);
}
// @ms_rbtree_delete() ok

/***********************************************************
 * @Func   : ms_rbtree_insert_value()
 * @Author : lwp
 * @Brief  : 插入排序的回调函数：按照结点 key 的大小确定插入位置，key 允许重复。
 * @Param  : [in] t : 向 t 子树中插入新结点，通常令 t = root
 * @Param  : [in] n : 新插入的结点
 * @Param  : [in] sentinel : 哨兵结点
 * @Return : NONE
 * @Note   : 
 ***********************************************************/
void ms_rbtree_insert_value(ms_rbtree_node_t *t, ms_rbtree_node_t *n,
        ms_rbtree_node_t *sentinel)
{
    ms_rbtree_node_t **p;

    // 将新结点插入最底层
    for ( ;; )
    {
        p = (n->key < t->key) ? &t->left : &t->right;
        if (*p == sentinel)
        {
            break;
        }

        t = *p;
    }

    *p = n;
    n->parent = t;
    n->left = sentinel;
    n->right = sentinel;

    // 新插入结点默认涂红
    ms_rbt_red(n);
}
// @ms_rbtree_insert_value() ok

/***********************************************************
 * @Func   : ms_rbtree_insert_timer_value()
 * @Author : lwp
 * @Brief  : 插入排序的回调函数：按照结点 key 的大小确定插入位置，key 允许重复。
 * @Param  : [in] t : 向 t 子树中插入新结点，通常令 t = root
 * @Param  : [in] n : 新插入的结点
 * @Param  : [in] sentinel : 哨兵结点
 * @Return : NONE
 * @Note   : 用于定时器，key 为时间值，区分正负
 ***********************************************************/
void ms_rbtree_insert_timer_value(ms_rbtree_node_t *t, ms_rbtree_node_t *n,
        ms_rbtree_node_t *sentinel)
{
    ms_rbtree_node_t **p;

    // 将新结点插入最底层
    for ( ;; )
    {
        /* 1) are spread in small range, usually several minutes,
         * 2) and overflow each 49 days, if milliseconds are stored in 32 bits.
         * The comparison takes into account that overflow.
         * (2^32 - 1) / 1000 / 3600 / 24 = 49.710 */
        p = ((intptr_t)(n->key - t->key) < 0) ? &t->left : &t->right;
        if (*p == sentinel)
        {
            break;
        }

        t = *p;
    }

    *p = n;
    n->parent = t;
    n->left = sentinel;
    n->right = sentinel;

    // 新插入结点默认涂红
    ms_rbt_red(n);
}
// @ms_rbtree_insert_timer_value() ok

/***********************************************************
 * @Func   : ms_rbtree_min()
 * @Author : lwp
 * @Brief  : 获取 node 子树中 key 最小的结点。
 * @Param  : [in] node
 * @Param  : [in] sentinel : 哨兵结点
 * @Return : 返回 node 子树中 key 最小结点
 * @Note   : 调用 ms_rbtree_min(ptree->root, ptree->sentinel) 前要确保红黑树非空
 *           if (ptree->root == ptree->sentinel) return;
 ***********************************************************/
ms_rbtree_node_t *ms_rbtree_min(ms_rbtree_node_t *node,
        ms_rbtree_node_t *sentinel)
{
    while (node->left != sentinel)
    {
        node = node->left;
    }

    return node;
}
// @ms_rbtree_min() ok

/***********************************************************
 * @Func   : ms_rbtree_max()
 * @Author : lwp
 * @Brief  : 获取 node 子树中 key 最大的结点。
 * @Param  : [in] node
 * @Param  : [in] sentinel : 哨兵结点
 * @Return : 返回 node 子树中 key 最大的结点
 * @Note   : 调用 ms_rbtree_min(ptree->root, ptree->sentinel) 前要确保红黑树非空
 *           if (ptree->root == ptree->sentinel) return;
 ***********************************************************/
ms_rbtree_node_t *ms_rbtree_max(ms_rbtree_node_t *node,
        ms_rbtree_node_t *sentinel)
{
    while (node->right != sentinel)
    {
        node = node->right;
    }

    return node;
}
// @ms_rbtree_max() ok

/***********************************************************
 * @Func   : ms_rbtree_next_up()
 * @Author : lwp
 * @Brief  : 查找 key 比 node 结点略大的结点。
 * @Param  : [in] tree
 * @Param  : [in] node
 * @Return : 返回 key 比 node 结点略大的结点
 * @Note   : 
 ***********************************************************/
ms_rbtree_node_t *ms_rbtree_next_up(ms_rbtree_t *tree, ms_rbtree_node_t *node)
{
    ms_rbtree_node_t *root, *sentinel, *parent;
    sentinel = tree->sentinel;

    // 若当前结点拥有右孩子，则比当前结点大的最小结点必在其右子树中
    if (node->right != sentinel)
    {
        return ms_rbtree_min(node->right, sentinel);
    }

    // 若当前结点没有右孩子，则当前结点位于最底层
    root = tree->root;
    for ( ;; )
    {
        if (node == root)
        {
            return NULL;
        }

        parent = node->parent;
        if (node == parent->left)
        {
            return parent;
        }

        /*   a
         *  /    Correct order should be: B, C, A. Assume we are at C,
         * b     so node->right is empty, node != root, and node != parent->left.
         *  \    we should return A, and this is what happens in the code.
         *   c                                                               */
        node = parent;
    }
}
// @ms_rbtree_next_up() ok

/***********************************************************
 * @Func   : ms_rbtree_next_down()
 * @Author : lwp
 * @Brief  : 查找 key 比 node 结点略小的结点。
 * @Param  : [in] tree
 * @Param  : [in] node
 * @Return : 返回 key 比 node 结点略小的结点
 * @Note   : 
 ***********************************************************/
ms_rbtree_node_t *ms_rbtree_next_down(ms_rbtree_t *tree, ms_rbtree_node_t *node)
{
    ms_rbtree_node_t *root, *sentinel, *parent;
    sentinel = tree->sentinel;

    // 若当前结点拥有左孩子，则比当前结点小的最大结点必在其左子树中
    if (node->left != sentinel)
    {
        return ms_rbtree_max(node->left, sentinel);
    }

    // 若当前结点没有左孩子，则当前结点位于最底层
    root = tree->root;
    for ( ;; )
    {
        if (node == root)
        {
            return NULL;
        }

        parent = node->parent;
        if (node == parent->right)
        {
            return parent;
        }

        node = parent;
    }
}
// @ms_rbtree_next_down() ok

/***********************************************************
 * @Func   : ms_rbtree_left_rotate()
 * @Author : lwp
 * @Brief  : 左旋。
 * @Param  : [in] root
 * @Param  : [in] sentinel
 * @Param  : [in] node
 * @Return : NONE
 * @Note   :     ||        ||
 *             [temp]    [node]
 *            //              \\
 *       [node]    <=左旋=     [temp]
 *            \\              //
 *             [targ]    [targ]
 ***********************************************************/
static inline void ms_rbtree_left_rotate(ms_rbtree_node_t **root,
        ms_rbtree_node_t *sentinel, ms_rbtree_node_t *node)
{
    ms_rbtree_node_t *temp;
    temp = node->right;

    // 设置 node <=> targ 间的指针
    node->right = temp->left;
    if (temp->left != sentinel)
    {
        temp->left->parent = node;
    }

    // 设置 temp <=> parent 间的指针
    temp->parent = node->parent;
    if (node == *root)
    {
        *root = temp;
    }
    else if (node == node->parent->left)
    {
        node->parent->left = temp;
    }
    else
    {
        node->parent->right = temp;
    }

    // 设置 temp <=> node 间的指针
    temp->left = node;
    node->parent = temp;
}
// @ms_rbtree_left_rotate() ok

/***********************************************************
 * @Func   : ms_rbtree_right_rotate()
 * @Author : lwp
 * @Brief  : 右旋。
 * @Param  : [in] root
 * @Param  : [in] sentinel
 * @Param  : [in] node
 * @Return : NONE
 * @Note   :     ||        ||
 *             [node]    [temp]
 *            //              \\
 *       [temp]     =右旋=>    [node]
 *            \\              //
 *             [targ]    [targ]
 ***********************************************************/
static inline void ms_rbtree_right_rotate(ms_rbtree_node_t **root,
        ms_rbtree_node_t *sentinel, ms_rbtree_node_t *node)
{
    ms_rbtree_node_t *temp;
    temp = node->left;

    // 设置 node <=> targ 间的指针
    node->left = temp->right;
    if (temp->right != sentinel)
    {
        temp->right->parent = node;
    }

    // 设置 temp <=> parent 间的指针
    temp->parent = node->parent;
    if (node == *root)
    {
        *root = temp;
    }
    else if (node == node->parent->left)
    {
        node->parent->left = temp;
    }
    else
    {
        node->parent->right = temp;
    }

    // 设置 temp <=> node 间的指针
    temp->right = node;
    node->parent = temp;
}
// @ms_rbtree_right_rotate() ok
