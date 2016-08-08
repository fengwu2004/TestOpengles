#ifndef GLES_FONT_INCLUDE
#define GLES_FONT_INCLUDE

#import <OpenGLES/ES3/gl.h>
#import <OpenGLES/ES3/glext.h>

#define T gles_font_t

typedef struct T *T;

struct bounds_t{
    float left;
    float top;
    float right;
    float bottom;
};

extern T            gles_font_new(const char *filepath, int resolution);

extern void         gles_font_free(T *fft);

extern void         gles_font_load(T fft);

/**
 * 设置字库内部着色器程序的投影矩阵
 */
extern void         gles_font_pmat(T fft, const GLfloat *pmat);

/**
 * 渲染文字
 */
extern void         gles_font_drawtext(T fft,
                    const char *str, int color, float x, float y);

/**
 * 按指定的特别投影矩阵渲染文字
 */
extern void         gles_font_drawtext_p(T fft, const GLfloat *pmat, 
                    const char *str, int color, float x, float y);

/**
 * 按指定的地图变换矩阵来渲染文字
 */
extern void         gles_font_drawtext_m(T fft, const GLfloat *mmat, 
                    const char *str, int color, float x, float y);


/**
 * 设置 重置 地图变换矩阵
 */
extern void         gles_font_set_mmat(T fft, const GLfloat *mmat);
extern void         gles_font_reset_mat(T fft);

/**
 * 获取字体尺寸
 */
extern float        gles_font_size(T fft);

/**
 * 获取字形贴图对象 id
 */
extern unsigned int gles_font_tid(T fft);

/**
 *
 * 查询如果使用本字体渲染 某文本
 * 所需要的范围
 *
 * 这里有毒, 我的ndkr10的编译器, 如b变量叫bounds就会出现奇怪的问题
 */
extern int          gles_font_text_bounds(T fft, const char *str, struct bounds_t * b, float x, float y);


#undef T
#endif /*GLES_FONT_INCLUDE*/
