#ifndef MOVER_INCLUDE
#define MOVER_INCLUDE


#include"vec2.h"

#define T mover_t

typedef struct T *T;


/**
 * 经典力学运动体对象
 * 用于动画
 */
extern T            mover_new_def(float x, float y);
extern T            mover_new(float x, float y, float maxspeed);


/**
 * 设置/获取,最大速度限制
 */
extern void         mover_set_maxspeed(T mv, float maxspeed);
extern float        mover_get_maxspeed(T mv);

/**
 * 设置/获取运动体的位置
 */
extern void         mover_set_location(T mv, float x, float y);
extern struct vec2  mover_get_location(T mv);

/**
 * 更新运动体
 */
extern void         mover_update(T mv);

/**
 * 在运动体上施加一个力
 */
extern void         mover_apply_force(T mv, struct vec2 force);


/**
 * 搜索目标位置
 */
extern void         mover_seek(T mv, struct vec2 target);

/**
 * 逃离目标位置
 */
extern void         mover_flee(T mv, struct vec2 target);

/**
 * 到达目标位置, 与目标距离小于等于dist时 返回1, 否则返回0
 */
extern int          mover_arrive(T mv, struct vec2 target, float dist);
#undef T
#endif /*MOVER_INCLUDE*/
