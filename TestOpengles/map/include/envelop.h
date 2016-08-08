#ifndef ENVELOP_INCLUDE
#define ENVELOP_INCLUDE

#include"arena.h"


/**
 * 提供一些正交矩形范围的检测方法
 */

#define T envelop_t

typedef struct T *T;

/**
 * 构造矩形范围, 使用内存池, 传入矩形边界
 */
extern  T       envelop_new(arena_t arena, double left, double top, 
                            double right, double bottom);

/**
 * 构造包围一个点的矩形范围, 在堆上分配, 需要手动释放
 */
extern  T       envelop_new_point(double x, double y);

/**
 * 构造矩形范围, 堆上分配, 需要手动释放
 */
extern  T       envelop_new_box(double left, double top, 
                                double right, double bottom);

/**
 * 释放堆上分配的矩形范围, 传入内存池分配的矩形范围是已检查的异常
 */
extern  void    envelop_free(T *e);


/**
 * 矩形范围是否包含某点
 */
extern  int     envelop_contains_point(T e, double x, double y);

/**
 * 是否包含target范围
 */
extern  int     envelop_contains(T e, T target);

/**
 * 两个范围之间距离的平方
 */
extern  double  envelop_distance_sq(T e, T target);

/**
 * 两个矩形之间是否相交
 */
extern  int     envelop_intersect(T e, T target);

/**
 * 两个矩形之间是否相交 数值版
 */
extern  int     envelop_intersect_box(T e, double left, double top, 
                                        double right, double bottom);

/**
 * 访问矩形范围的数据
 */
extern  double  envelop_left(T e);
extern  double  envelop_top(T e);
extern  double  envelop_right(T e);
extern  double  envelop_bottom(T e);
extern  double  envelop_width(T e);
extern  double  envelop_height(T e);
extern  double  envelop_center_x(T e);
extern  double  envelop_center_y(T e);


#undef T
#endif /*ENVELOP_INCLUDE*/
