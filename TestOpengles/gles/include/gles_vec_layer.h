#ifndef GLES_VEC_LAYER_INCLUDE
#define GLES_VEC_LAYER_INCLUDE

#include"gles_2d_c_program.h"
#include"gles_map.h"
#import <OpenGLES/ES3/gl.h>
#import <OpenGLES/ES3/glext.h>
#define T gles_vec_layer_t

typedef struct T *T;


/**
 * 现在流程有了变动, 剥离了数据库版的地图文件
 * 从自定义的vbf文件加载顶点数据
 */
extern T            gles_vec_layer_new(gles_map_t map, const char *vbfile);

extern void         gles_vec_layer_free(T *gvl);

extern void         gles_vec_layer_render(T gvl, gles_2d_c_program_t gcp);

/**
 * 现在地图的宽高, 方向角度, 在vbf中了
 * gles_map_t 需要从vec layer层来读取
 */
extern int          gles_vec_layer_width(T gvl);
extern int          gles_vec_layer_height(T gvl);
extern int          gles_vec_layer_azimuth(T gvl);

#undef T
#endif /*GLES_VEC_LAYER_INCLUDE*/
