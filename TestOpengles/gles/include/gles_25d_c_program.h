#ifndef GLES_25D_C_PROGRAM_INCLUDE
#define GLES_25D_C_PROGRAM_INCLUDE


#import <OpenGLES/ES3/gl.h>
#import <OpenGLES/ES3/glext.h>
#define T gles_25d_c_program_t

typedef struct T *T;

extern T        gles_25d_c_program_new();

extern void     gles_25d_c_program_free(T *gcp);

/**
 * 渲染存放在VBO对象中的三角形集合
 */
extern void     gles_25d_c_program_vbo_draw_triangle(T gcp, GLuint vboid, GLuint vcnt); 


/**
 * 调用GLDrawElements方法, 通用地渲染图元
 */
extern void     gles_25d_c_program_draw_elements(T gcp, GLenum mode, const GLfloat *vbuf, const GLfloat *cbuf, const GLushort *ibuf, GLuint vcnt);

/**
 * 调用GLDrawElements方法, 渲染VAO
 */
extern void     gles_25d_c_program_draw_elements_vao(T gcp, GLenum mode, GLuint vaoid, GLuint vcnt);

extern void     gles_25d_c_program_set_matrix_projection(T g3cp, const GLfloat *pmat);

extern void     gles_25d_c_program_set_matrix_view(T g3cp, const GLfloat *vmat);

extern void     gles_25d_c_program_set_matrix_world(T g3cp, const GLfloat *wmat);

extern void     gles_25d_c_program_set_matrix_object(T g3cp, const GLfloat *omat);

extern void     gles_25d_c_program_set_height(T gcp, const GLfloat height);

#undef T
#endif /*GLES_25D_C_PROGRAM_INCLUDE*/
