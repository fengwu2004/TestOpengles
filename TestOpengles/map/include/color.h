#ifndef COLOR_INCLUDE
#define COLOR_INCLUDE

/**
 * 一些颜色相关的辅助方法
 */


/**
 * 用argb分量构造颜色数据
 */
unsigned int    color_argb(
                    unsigned int a,
                    unsigned int r,
                    unsigned int g,
                    unsigned int b);

/**
 * 不用alpha分量
 */
unsigned int    color_rgb(
                    unsigned int r,
                    unsigned int g,
                    unsigned int b);

/**
 * 获取4个分量
 */
unsigned int    color_alpha(unsigned int color);
unsigned int    color_red(unsigned int color);
unsigned int    color_green(unsigned int color);
unsigned int    color_blue(unsigned int color);


/**
 * 有些库比如OpenGL, Cario等, 用的浮点分量0-1.0
 * 提供浮点分量方法
 */
double          color_alpha_range(unsigned int color);
double          color_red_range(unsigned int color);
double          color_green_range(unsigned int color);
double          color_blue_range(unsigned int color);



#endif /*COLOR_INCLUDE*/
