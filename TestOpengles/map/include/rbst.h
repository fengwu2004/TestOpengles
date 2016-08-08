#ifndef RBST_INCLUDE
#define RBST_INCLUDE

#include"arena.h"

/**
 * 区间二叉搜索树
 * range binary search tree
 */
#define T rbst_t

typedef struct T *T;

extern  T           rbst_new();

/**
 * 主要用于释放空间树中在堆上分配的空间
 */
extern  void        rbst_free(T *rbst);


/**
 * 用于在区域中寻找缺口
 * 如果找到缺口, has设置为1, 函数返回插入节点指针
 * 如果没有缺口, has设置为0, 函数返回NULL
 */
extern void         *rbst_gap_search(T rbst, int *has, float start, float end);

/**
 * 插入缺口, 传入 rbst_gap_search返回的位置匿名指针
 * 如果插入传入的start, end和搜索时不同导致无法插入, 会导致断言
 */
extern void         rbst_gap_insert(T rbst, void *pos, float start, float end);

/**
 * 重置区域搜索树
 */
extern void         rbst_reset(T rbst);

#undef T
#endif /*RBST_INCLUDE*/
