#ifndef QUAD_TREE_INCLUDE
#define QUAD_TREE_INCLUDE

#include"arena.h"
#include"seq.h"
#include"envelop.h"
#include"quadnode.h"

/**
 * 空间树
 */

#define T quadtree_t

typedef struct T *T;

/**
 * 新建空间树
 */
extern  T           quadtree_new(arena_t arena, 
                        double left, double top, double right, double bottom);
extern  T           quadtree_new_space(arena_t arena, envelop_t extent);


/**
 * 主要用于释放空间树中在堆上分配的稀疏数组
 */
extern  void        quadtree_free(T qt);


/**
 * 使空间树退化成列表
 * 用于OpenGL矢量底图等对象多, 又不需要空间索引的图层
 */
extern void         quadtree_degenerate(T qt);

/**
 * 返回树的深度
 */
extern int          quadtree_depth(T qt);

/**
 * 访问空间树中的要素
 */
extern  void       *quadtree_get(T qt, int id);
extern  int         quadtree_get_count(T qt);
extern  void       *quadtree_get_by_index(T qt, int index);
extern  void        quadtree_insert(T qt, int id, void *obj, double left, double top, double right, double bottom);

/**
 * 快速检定
 */
extern  int         quadtree_fast_contains_check(T qt, double x, double y);
extern  int         quadtree_fast_intersect_check(T qt, 
                        double left, double top, double right, double bottom);
extern  int         quadtree_fast_intersect_check_space(T qt, envelop_t e);


/**
 * 空间搜索
 */
extern  seq_t       quadtree_search(T qt, 
                        double left, double top, double right, double bottom, 
                        double maxdistance, int maxcount, seq_t retlist);
extern  seq_t       quadtree_search_space(T qt, envelop_t searchbox, 
                        double maxdistance, int maxcount, seq_t retlist);


/**
 * 空间树渲染
 */
extern  void        quadtree_render(T qt,
                                double left, double top, 
                                double right, double bottom,
                                void (*render)(void *obj, envelop_t e, void *cl), 
                                void *cl);
extern  void        quadtree_render_depth(T qt, int depth,
                                double left, double top, 
                                double right, double bottom,
                                void (*render)(void *obj, envelop_t e, void *cl), 
                                void *cl);

#undef T
#endif /*QUAD_TREE_INCLUDE*/
