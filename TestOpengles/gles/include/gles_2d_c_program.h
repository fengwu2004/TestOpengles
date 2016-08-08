#ifndef GLES_2D_C_PROGRAM_INCLUDE
#define GLES_2D_C_PROGRAM_INCLUDE

/**
 * 颜色填充类着色器程序对象化封装
 */

#import <OpenGLES/ES3/gl.h>
#import <OpenGLES/ES3/glext.h>

#define T gles_2d_c_program_t

typedef struct T *T;

extern T        gles_2d_c_program_new();

extern void     gles_2d_c_program_free(T *gcp);

/**
 * 渲染存放在VBO对象中的三角形集合
 */
extern void     gles_2d_c_program_vbo_draw_triangle(T gcp, GLuint vboid, GLuint vcnt); 

/**
 * 调用GLDrawArrays方法通用地渲染图元
 */
extern void     gles_2d_c_program_draw_arrays(T gcp, GLenum mode, const GLfloat *vbuf, const GLfloat *cbuf, GLuint vcnt);

/**
 * 调用GLDrawElements方法, 通用地渲染图元
 */
extern void     gles_2d_c_program_draw_elements(T gcp, GLenum mode, const GLfloat *vbuf, const GLfloat *cbuf, const GLushort *ibuf, GLuint vcnt);

/**
 * 调用GLDrawElements方法, 渲染VAO
 */
extern void     gles_2d_c_program_draw_elements_vao(T gcp, GLenum mode, GLuint vaoid, GLuint vcnt);
/**
 * 设置投影矩阵
 */
extern void     gles_2d_c_program_set_matrices_p(T gcp, const GLfloat *pmat);

/**
 * 设置地图变换矩阵
 */
extern void     gles_2d_c_program_set_matrices_m(T gcp, const GLfloat *mmat);

/**
 * 设置对象变换矩阵
 */
extern void     gles_2d_c_program_set_matrices_o(T gcp, const GLfloat *omat);

#undef T
#endif /*GLES_2D_C_PROGRAM_INCLUDE*/
