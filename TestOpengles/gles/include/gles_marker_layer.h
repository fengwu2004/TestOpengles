#ifndef GLES_MARKER_LAYER_INCLUDE
#define GLES_MARKER_LAYER_INCLUDE

#import <OpenGLES/ES3/gl.h>
#import <OpenGLES/ES3/glext.h>
#include"gles_2d_t_program.h"
#include"gles_map.h"
//#include"gles_texture_android.h"

#define T gles_marker_layer_t

typedef struct T *T;


extern T            gles_marker_layer_new(gles_map_t map, int mwidth, int mheight);

extern void         gles_marker_layer_free(T *gml);


extern void         gles_marker_layer_render(T gml, gles_2d_t_program_t gtp, const GLfloat *mmat);

/**
 * 设置整层的可见性
 */
extern void         gles_marker_layer_set_visible(T gml, int visible);

/**
 * 查找marker, 返回marker id, 如果没找到返回-1
 */
extern int          gles_marker_layer_find(T gml, float x, float y);

/**
 * 插入Marker
 */
//extern int          gles_marker_layer_insert_android(T gml, gles_texture_android_t txa, int mclass,
//                                                float x, float y, float offsetx, float offsety, float scale);


/**
 * 删除某类所有的marker
 */
extern void         gles_marker_layer_remove_class(T gml, int mclass);

/**
 * 移除某Marker
 */
extern void         gles_marker_remove(T gml, int markerid);

/**
 * 更新某Marker位置, 如果该Marker已经不存在, 返回0, 否则返回1
 */
extern int          gles_marker_update_position(T gml, int markerid, float x, float y);

/**
 * 清除所有的Marker
 */
extern void         gles_marker_clean(T gml);

#undef T
#endif /*GLES_MARKER_LAYER_INCLUDE*/
