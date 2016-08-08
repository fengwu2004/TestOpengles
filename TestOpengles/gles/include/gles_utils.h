#ifndef GLES_UTILS_INCLUDE
#define GLES_UTILS_INCLUDE

#import <OpenGLES/ES3/gl.h>
#import <OpenGLES/ES3/glext.h>
/**
 * 一些零碎的工具性方法
 */


/**
 * 编译着色器
 */
extern GLuint  gles_compile_shader(GLenum type, const char* glsl);

/**
 * 链接着色器
 */
extern GLuint  gles_link_shader(GLuint vertshaderid, GLuint fragshaderid);

/**
 * 创建着色程序
 */
extern GLuint  gles_create_program(const char* vertexglsl, const char* fragmentglsl);
    
/**
 * 打印GL信息和错误检查
 */
extern void    gles_print_info();
extern void    gles_check_error(const char* op);


/**
 * 从32位整型颜色拆解出用浮点数表示的颜色分量
 */
extern float   gles_color_red(int color);
extern float   gles_color_green(int color);
extern float   gles_color_blue(int color);
extern float   gles_color_alpha(int color);

#endif /*GLES_UTILS_INCLUDE*/
