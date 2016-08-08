
#include"assert.h"
#include"mem.h"
#include"gles_2d_c_program.h"
#include"gles_matrix.h"
#include "gles_utils.h"
#define T gles_2d_c_program_t

#define ATTRIBUTE_POSITION  0
#define ATTRIBUTE_COLOR     1

struct T{
    GLuint  pid;        /** GLSL程序句柄*/
    GLuint  upmat;      /** 投影矩阵 uniform*/
    GLuint  ummat;      /** 地图矩阵 uniform*/
    GLuint  uomat;      /** 对象矩阵 uniform*/
};

static const char _vertexshader[] =
"#version 300 es                                    \n"
"layout(location = 0)in vec4    aPosition;          \n"
"layout(location = 1)in vec4    aColor;             \n"
"uniform mat4                   uProjMat;           \n"
"uniform mat4                   uMapMat;            \n"
"uniform mat4                   uObjMat;            \n"
"out vec4                       vColor;             \n"
"void main() {                                      \n"
"  gl_Position = uProjMat*uMapMat*uObjMat*aPosition;\n"
"  vColor = aColor;                                 \n"
"}";

static const char _fragmentshader[] =
"#version 300 es            \n"
"precision mediump float;   \n"
"in vec4 vColor;            \n"
"out vec4 FragColor;        \n"
"void main() {              \n"
"  FragColor = vColor;\n"
"}";

static GLfloat _i_mat[16];

T        
gles_2d_c_program_new()
{
    T gcp;

    gcp = ALLOC(sizeof(*gcp));
    gcp->pid = gles_create_program(_vertexshader, _fragmentshader);
    assert(gcp->pid);

    glUseProgram(gcp->pid);
    gcp->upmat = glGetUniformLocation(gcp->pid, "uProjMat");
    gcp->ummat = glGetUniformLocation(gcp->pid, "uMapMat");
    gcp->uomat = glGetUniformLocation(gcp->pid, "uObjMat");

    gles_matrix_identity_4(_i_mat, 0);

    return gcp;
}

void     
gles_2d_c_program_free
(T *gcp)
{
    assert(gcp);
    assert(*gcp);
    
    glDeleteProgram((*gcp)->pid);
    FREE(*gcp);
}

void     
gles_2d_c_program_vbo_draw_triangle
(T gcp, GLuint vboid, GLuint vcnt)
{
    assert(gcp);

    glUseProgram(gcp->pid);
    glEnableVertexAttribArray(ATTRIBUTE_POSITION);
    glEnableVertexAttribArray(ATTRIBUTE_COLOR);

    glDisable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);

    glBindBuffer(GL_ARRAY_BUFFER, vboid);
    glVertexAttribPointer(ATTRIBUTE_POSITION, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glVertexAttribPointer(ATTRIBUTE_COLOR,    4, GL_FLOAT, GL_FALSE, 0, (void*)(vcnt * 2 * sizeof(float)));
    glDrawArrays(GL_TRIANGLES, 0, vcnt);
    glBindBuffer(GL_ARRAY_BUFFER, 0 );
    glEnable(GL_CULL_FACE);
}

void    
gles_2d_c_program_draw_arrays
(T gcp, GLenum mode, const GLfloat *vbuf, const GLfloat *cbuf, GLuint vcnt)
{
    glUseProgram(gcp->pid);
    glEnableVertexAttribArray(ATTRIBUTE_POSITION);
    glEnableVertexAttribArray(ATTRIBUTE_COLOR);

    glVertexAttribPointer(ATTRIBUTE_POSITION, 2, GL_FLOAT, GL_FALSE, 0, vbuf);
    glVertexAttribPointer(ATTRIBUTE_COLOR,    3, GL_FLOAT, GL_FALSE, 0, cbuf);
    glDrawArrays(mode, 0, vcnt);
}

void     
gles_2d_c_program_draw_elements
(T gcp, GLenum mode, const GLfloat *vbuf, const GLfloat *cbuf, const GLushort *ibuf, GLuint vcnt)
{
    glUseProgram(gcp->pid);
    glEnableVertexAttribArray(ATTRIBUTE_POSITION);
    glEnableVertexAttribArray(ATTRIBUTE_COLOR);

    glVertexAttribPointer(ATTRIBUTE_POSITION, 2, GL_FLOAT, GL_FALSE, 0, vbuf);
    glVertexAttribPointer(ATTRIBUTE_COLOR,    3, GL_FLOAT, GL_FALSE, 0, cbuf);
    glDrawElements(mode, vcnt, GL_UNSIGNED_SHORT, ibuf);
}

void     
gles_2d_c_program_draw_elements_vao
(T gcp, GLenum mode, GLuint vaoid, GLuint vcnt)
{
    glUseProgram(gcp->pid);
    glBindVertexArray(vaoid);
    glDrawElements(mode, vcnt, GL_UNSIGNED_SHORT, (void*)0);

    /**
     * 注意, 这里一定要解绑, 否则后面所有的绘制都会挂
     */
    glBindVertexArray(0);
}

void     
gles_2d_c_program_set_matrices_p
(T gcp, const GLfloat *pmat)
{
    glUseProgram(gcp->pid);
    glUniformMatrix4fv(gcp->upmat, 1, 0, pmat);
    glUniformMatrix4fv(gcp->ummat, 1, 0, _i_mat);
    glUniformMatrix4fv(gcp->uomat, 1, 0, _i_mat);
}

void     
gles_2d_c_program_set_matrices_m
(T gcp, const GLfloat *mmat)
{
    glUseProgram(gcp->pid);
    glUniformMatrix4fv(gcp->ummat, 1, 0, mmat);
    glUniformMatrix4fv(gcp->uomat, 1, 0, _i_mat);
}

void     
gles_2d_c_program_set_matrices_o
(T gcp, const GLfloat *omat)
{
    glUseProgram(gcp->pid);
    glUniformMatrix4fv(gcp->uomat, 1, 0, omat);
}


#undef ATTRIBUTE_POSITION
#undef ATTRIBUTE_COLOR 
