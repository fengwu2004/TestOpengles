#ifndef GLES_UNIT_LAYER_INCLUDE
#define GLES_UNIT_LAYER_INCLUDE

#include"gles_font.h"
#include"gles_2d_c_program.h"
#include"gles_2d_t_program.h"
#include"gles_map.h"
#import <OpenGLES/ES3/gl.h>
#import <OpenGLES/ES3/glext.h>
#define T gles_unit_layer_t

typedef struct T *T;


extern T            gles_unit_layer_new(gles_map_t map, GLuint tid, int mwidth, int mheight);

extern void         gles_unit_layer_free(T *gul);

/**
 * 传递GL Surface的视图尺寸变更
 * 因为Units 渲染对这个很敏感, 而GL Surface表面尺寸的确定可能晚于
 * Units覆盖层初始化, 所以需要用这个调用来确保
 */
extern void         gles_unit_view_size(T gul, int vwidth, int vheight);

extern void         gles_unit_layer_render(T gul, 
                        gles_2d_c_program_t gcp, 
                        gles_2d_t_program_t gtp, 
                        gles_font_t  font, const GLfloat *mmat, const GLfloat *inmmat);

/**
 * 设置可见性
 */
extern void         gles_unit_set_visible(T gul, int visible);

/**
 * 检查空间树深度
 */
extern int          gles_unit_quad_tree_depth(T gul);

/**
 * 插入Icon unit
 */
extern void         gles_unit_insert_icon(T gul, int id, int type, double left, double top, double right, double bottom);

/**
 * 插入Text unit
 */
extern void         gles_unit_insert_text(T gul, int id, double left, double top, double right, double bottom, const char *text);

/**
 * 查找Text和Icon unit
 */
extern int          gles_unit_find_text(T gul, float x, float y, float range);
extern int          gles_unit_find_icon(T gul, float x, float y, float range);

#undef T
#endif /*GLES_UNIT_LAYER_INCLUDE*/
