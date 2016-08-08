#ifndef GLES_2D_T_PROGRAM_INCLUDE
#define GLES_2D_T_PROGRAM_INCLUDE

/**
 * 贴图着色器的对象化封装
 */

#import <OpenGLES/ES3/gl.h>
#import <OpenGLES/ES3/glext.h>
#define T gles_2d_t_program_t

typedef struct T *T;

extern T        gles_2d_t_program_new();

extern void     gles_2d_t_program_free(T *gtp);

/**
 * 绑定贴图对象
 */
extern void     gles_2d_t_program_bind_texture(T gtp, GLuint tid);

/**
 * 传入顶点缓存和贴图坐标缓存
 * 调用GLDrawArrays来渲染简单的图元
 */
extern void     gles_2d_t_program_draw_arrays(T gtp, GLenum mode, const GLfloat *vbuf, const GLfloat *tbuf, GLuint vcnt);

/**
 * 设置投影矩阵
 */
extern void     gles_2d_t_program_set_matrices_p(T gtp, const GLfloat *pmat);

/**
 * 设置地图变换矩阵
 */
extern void     gles_2d_t_program_set_matrices_m(T gtp, const GLfloat *mmat);

/**
 * 设置/重置(单位矩阵) 对象变换矩阵
 */
extern void     gles_2d_t_program_set_matrices_o(T gtp, const GLfloat *omat);
extern void     gles_2d_t_program_reset_matrices_o(T gtp);


#undef T
#endif /*GLES_2D_T_PROGRAM_INCLUDE*/
