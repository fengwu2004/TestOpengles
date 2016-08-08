#ifndef GLES_3D_C_PROGRAM_INCLUDE
#define GLES_3D_C_PROGRAM_INCLUDE

/**
 * 颜色填充类3D着色器程序对象化封装
 */

#import <OpenGLES/ES3/gl.h>
#import <OpenGLES/ES3/glext.h>
#define T gles_3d_c_program_t

typedef struct T *T;

extern T        gles_3d_c_program_new();

extern void     gles_3d_c_program_free(T *g3cp);

/**
 * 渲染存放在VBO对象中的三角形集合
 */
extern void     gles_3d_c_program_vbo_draw_triangle(T g3cp, GLuint vboid, GLuint vcnt); 

/**
 * 调用GLDrawArrays方法通用地渲染图元
 */
extern void     gles_3d_c_program_draw_arrays(T g3cp, GLenum mode, const GLfloat *vbuf, const GLfloat *cbuf, GLuint vcnt);

/**
 * 调用GLDrawElements方法, 通用地渲染图元
 */
extern void     gles_3d_c_program_draw_elements(T g3cp, GLenum mode, const GLfloat *vbuf, const GLfloat *cbuf, const GLushort *ibuf, GLuint vcnt);

/**
 * 调用GLDrawElements方法, 渲染VAO
 */
extern void     gles_3d_c_program_draw_elements_vao(T g3cp, GLenum mode, GLuint vaoid, GLuint vcnt);
/**
 * 设置投影矩阵
 */
extern void     gles_3d_c_program_set_matrix_projection(T g3cp, const GLfloat *pmat);

extern void     gles_3d_c_program_set_matrix_view(T g3cp, const GLfloat *vmat);

extern void     gles_3d_c_program_set_matrix_world(T g3cp, const GLfloat *wmat);

extern void     gles_3d_c_program_set_matrix_object(T g3cp, const GLfloat *omat);

#undef T
#endif /*GLES_3D_C_PROGRAM_INCLUDE*/
