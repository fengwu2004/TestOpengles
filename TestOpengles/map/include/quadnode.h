#ifndef QUAD_NODE_INCLUDE
#define QUAD_NODE_INCLUDE

#include"arena.h"
#include"seq.h"
#include"envelop.h"


/**
 * 空间树节点
 */

#define T quadnode_t

typedef struct T *T;


/**
 * 新建空间树节点
 */
extern  T       quadnode_new(arena_t arena, int depth,
                    double left, double top, double right, double bottom);
extern  T       quadnode_new_space(arena_t arena, int depth, envelop_t space);

/**
 * 对空间树节点插入和移除要素
 * 返回插入的节点的深度
 */
extern  int     quadnode_insert(T qn, void *obj, double left, double top, double right, double bottom);

/**
 * 对空间树节点做快速检定
 */
extern  int     quadnode_fast_contains_check(T qn, double x, double y);
extern  int     quadnode_fast_intersect_check(T qn, envelop_t checkbox);

/**
 * 空间搜索
 */
extern  int     quadnode_search(T qn, envelop_t searchbox, 
                        double maxdistance, int maxcount, seq_t retlist);

/**
 * 节点渲染
 */
extern  void    quadnode_render(T qn,
                                envelop_t range,
                                void (*render)(void *obj, envelop_t e, void *cl), 
                                void *cl);
extern  void    quadnode_render_depth(T qn, int depth,
                                envelop_t range,
                                void (*render)(void *obj, envelop_t e, void *cl), 
                                void *cl);

#undef T
#endif /*QUAD_NODE_INCLUDE*/
