#ifndef GLES_SCENE_TOUCHMGR_INCLUDE
#define GLES_SCENE_TOUCHMGR_INCLUDE

#import <OpenGLES/ES3/gl.h>
#import <OpenGLES/ES3/glext.h>
#include "gles_scene_render.h"

#define T gles_scene_touchmgr_t


typedef struct T *T;

extern T    gles_scene_touchmgr_new(gles_scene_render_t gsr);

extern void gles_scene_touchmgr_free(T *tm);

extern int  gles_scene_touchmgr_ontouch(T tm, int action, float x0, float y0, float x1, float y1);

#undef T
#endif /*GLES_SCENE_TOUCHMGR_INCLUDE*/
