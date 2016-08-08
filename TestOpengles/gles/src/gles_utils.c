#include<stdlib.h>
#include<string.h>
#include<time.h>
#include<math.h>
#include"assert.h"
#ifdef ANDROID_IDRN
#include"error_functions.h"
#endif
#include"gles_utils.h"

static void _print_gl_string(const char *name, GLenum s);



GLuint 
gles_compile_shader
(GLenum type, const char* glsl)
{
    GLuint shader = glCreateShader(type);
    
    if (shader) {
        glShaderSource(shader, 1, &glsl, NULL);
        glCompileShader(shader);
        
        GLint compiled = 0;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
        if (!compiled) {
            GLint infoLen = 0;
            glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLen);
            if (infoLen) {
                char* buf = (char*) malloc(infoLen);
                if (buf) {
                    glGetShaderInfoLog(shader, infoLen, NULL, buf);
                    #ifdef ANDROID_IDRN
                    log_e("Could not compile shader %d:\n%s\n", type, buf);
                    #endif
                    free(buf);
                }
                glDeleteShader(shader);
                shader = 0;
            }
        }
    }
    return shader;
}


GLuint 
gles_link_shader
(GLuint vertshaderid, GLuint fragshaderid)
{
    assert(vertshaderid);
    assert(fragshaderid);

    GLuint program = glCreateProgram();
    if (program) {
        glAttachShader(program, vertshaderid);
        glAttachShader(program, fragshaderid);
        
        glLinkProgram(program);
        
        GLint linkstatus = GL_FALSE;
        glGetProgramiv(program, GL_LINK_STATUS, &linkstatus);
        if (linkstatus != GL_TRUE) {
            GLint length = 0;
            glGetProgramiv(program, GL_INFO_LOG_LENGTH, &length);
            if (length) {
                char* buf = (char*) malloc(length);
                if (buf) {
                    glGetProgramInfoLog(program, length, NULL, buf);
//                    log_e("Could not link program:\n%s\n", buf);
                    free(buf);
                }
            }
            glDeleteProgram(program);
            program = 0;
        }
    }
    return program;
}

GLuint 
gles_create_program
(const char* vertexglsl, const char* fragmentglsl)
{
    GLuint vsid = gles_compile_shader(GL_VERTEX_SHADER, vertexglsl);
    GLuint fsid = gles_compile_shader(GL_FRAGMENT_SHADER, fragmentglsl);
    return gles_link_shader(vsid, fsid);
}
    
void 
gles_print_info()
{
	_print_gl_string("Version",	GL_VERSION);
	_print_gl_string("Vendor",		GL_VENDOR);
	_print_gl_string("Renderer",	GL_RENDERER);
	_print_gl_string("Extensions", GL_EXTENSIONS);
	_print_gl_string("GL Shading Language", GL_SHADING_LANGUAGE_VERSION);
}

void
gles_check_error
(const char* op)
{
    GLint error;
    for (error = glGetError(); error; error= glGetError()) {
//        log_i("after %s() glError (0x%x)\n", op, error);
    }
}

float   
gles_color_red
(int color)
{
    return ((color&0x00FF0000)>>16)/255.0f;
}


float   
gles_color_green
(int color)
{
    return ((color&0x0000FF00)>>8)/255.0f;
}


float   
gles_color_blue
(int color)
{
    return (color&0x000000FF)/255.0f;
}


float   
gles_color_alpha
(int color)
{
    return ((color&0xFF000000)>>24)/255.0f;
}


static void _print_gl_string(const char *name, GLenum s)
{
//    log_i("GL %s = %s\n", name, (const char *) glGetString(s));
}

