
#include"assert.h"
#include"mem.h"
#include"gles_2d_t_program.h"
#include"gles_matrix.h"
#include "gles_utils.h"

#define ATTRIBUTE_POSITION  0
#define ATTRIBUTE_TEXTCOORD 1

#define T gles_2d_t_program_t

struct T{
    GLuint  pid;        /** GLSL程序句柄*/
    GLuint  upmat;      /** 投影矩阵 uniform*/
    GLuint  ummat;      /** 地图矩阵 uniform*/
    GLuint  uomat;      /** 对象矩阵 uniform*/
    GLuint  utexture;   /** 贴图对象 uniform*/
};

static const char _txtvshader[] = 
"#version 300 es                                            \n"
"layout(location = 0) in vec4  aPosition;                   \n"
"layout(location = 1) in vec2  aTexCoord;                   \n"
"out vec2 vTexCoord;                                        \n"
"uniform mat4   uProjMat;                                   \n"
"uniform mat4   uMapMat;                                    \n"
"uniform mat4   uObjMat;                                    \n"
"void main( void ) {                                        \n"
"    vTexCoord = aTexCoord;                                 \n"
"    gl_Position = uProjMat*uMapMat*uObjMat*aPosition;      \n"
"}\n";
static const char _txtfshader[] = 
"#version 300 es                                            \n"
"precision mediump float;                                   \n"
"in vec2 vTexCoord;                                         \n"
"uniform sampler2D uTex;                                    \n"
"layout(location = 0) out vec4 outColor;                    \n"
"void main() {                                              \n"
"   outColor = texture(uTex, vTexCoord);                    \n"
"}\n";

static GLfloat _i_mat[16];

T        
gles_2d_t_program_new()
{
    T gtp;

    gtp = ALLOC(sizeof(*gtp));
    gtp->pid = gles_create_program(_txtvshader, _txtfshader);
    assert(gtp->pid);

    glUseProgram(gtp->pid);
    gtp->upmat = glGetUniformLocation(gtp->pid, "uProjMat");
    gtp->ummat = glGetUniformLocation(gtp->pid, "uMapMat");
    gtp->uomat = glGetUniformLocation(gtp->pid, "uObjMat");
    gtp->utexture= glGetUniformLocation(gtp->pid, "uTex");

    gles_matrix_identity_4(_i_mat, 0);

    return gtp;
}

void     
gles_2d_t_program_free(T *gtp)
{
    assert(gtp);
    assert(*gtp);
    
    glDeleteProgram((*gtp)->pid);
    FREE(*gtp);
}

void     
gles_2d_t_program_bind_texture
(T gtp, GLuint tid)
{
    assert(gtp);

    glUseProgram(gtp->pid);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tid);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
}


void     
gles_2d_t_program_draw_arrays
(T gtp, GLenum mode, const GLfloat *vbuf, const GLfloat *tbuf, GLuint vcnt)
{
    glUseProgram(gtp->pid);
    glEnableVertexAttribArray(ATTRIBUTE_POSITION);
    glEnableVertexAttribArray(ATTRIBUTE_TEXTCOORD);

    glUniform1i(gtp->utexture, 0);
    glVertexAttribPointer(ATTRIBUTE_POSITION, 2, GL_FLOAT, GL_FALSE, 0, vbuf);
    glVertexAttribPointer(ATTRIBUTE_TEXTCOORD, 2, GL_FLOAT, GL_FALSE, 0, tbuf);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, vcnt);
}

void   
gles_2d_t_program_set_matrices_p
(T gtp, const GLfloat *pmat)
{
    assert(gtp);

    glUseProgram(gtp->pid);
    glUniformMatrix4fv(gtp->upmat, 1, 0, pmat);
    glUniformMatrix4fv(gtp->ummat, 1, 0, _i_mat);
    glUniformMatrix4fv(gtp->uomat, 1, 0, _i_mat);
}

void     
gles_2d_t_program_set_matrices_m
(T gtp, const GLfloat *mmat)
{
    assert(gtp);

    glUseProgram(gtp->pid);
    glUniformMatrix4fv(gtp->ummat, 1, 0, mmat);
    glUniformMatrix4fv(gtp->uomat, 1, 0, _i_mat);
}

void     
gles_2d_t_program_set_matrices_o
(T gtp, const GLfloat *omat)
{
    assert(gtp);

    glUseProgram(gtp->pid);
    glUniformMatrix4fv(gtp->uomat, 1, 0, omat);
}

void     
gles_2d_t_program_reset_matrices_o
(T gtp)
{
    assert(gtp);

    glUniformMatrix4fv(gtp->uomat, 1, 0, _i_mat);
}
#undef ATTRIBUTE_POSITION
#undef ATTRIBUTE_TEXTCOORD
