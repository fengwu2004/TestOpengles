
#ifndef GLES_MAP_ANIMATION_INCLUDE
#define GLES_MAP_ANIMATION_INCLUDE

#import <OpenGLES/ES3/gl.h>
#import <OpenGLES/ES3/glext.h>
#include "gles_map.h"

#define T gles_map_animation_t


typedef struct T *T;

extern T    gles_map_animation_new(gles_map_t map);

extern void gles_map_animation_free(T *gma);

/**
 * 每帧渲染前做动画相关数据的更新
 */
extern void gles_map_animation_update(T gma);

/**
 * 清除正在执行的动画和动画序列中未决的动画
 */
extern void gles_map_animation_clean(T gma);

/**
 * 返回是否有动画正在执行的标志
 */
extern int  gles_map_animation_flag(T gma);

/**
 * 将一个旋转归零动画放入动画序列
 */
extern void gles_map_animation_rotate_reset(T gma);

/**
 * 将一个旋转动画放入动画序列
 */
extern void gles_map_animation_rotate(T gma, float rotate, float x, float y);

/**
 * 将一个缩放动画放入动画序列
 */
extern void gles_man_animation_zoom_by(T gma, float rate); 

/**
 * 将一个平移动画放入动画序列
 */
extern void gles_map_animation_translate(T gma, float tx, float ty);

/**
 * 将一个平移旋转融合动画放入动画序列
 */
extern void gles_map_animation_rotate_translate(T gma, float rotate, float sx, float sy, float ex, float ey);


#undef T
#endif /*GLES_MAP_ANIMATION_INCLUDE*/
