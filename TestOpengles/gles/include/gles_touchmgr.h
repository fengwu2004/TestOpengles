#ifndef GLES_TOUCHMGR_INCLUDE
#define GLES_TOUCHMGR_INCLUDE

#import <OpenGLES/ES3/gl.h>
#import <OpenGLES/ES3/glext.h>
#include "gles_map.h"

#define T gles_touchmgr_t


typedef struct T *T;

extern T    gles_touchmgr_new(gles_map_t map);

extern void gles_touchmgr_free(T *tm);

/**
 * 禁用启用触摸
 */
extern void gles_touchmgr_enable(T tm, int enable);

/**
 * 传递触摸事件
 */
extern int  gles_touchmgr_ontouch(T tm, int action, float x0, float y0, float x1, float y1);

#undef T
#endif /*GLES_TOUCHMGR_INCLUDE*/
