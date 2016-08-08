#ifndef GLES_TEXTURE_PNG_INCLUDE
#define GLES_TEXTURE_PNG_INCLUDE


#define T gles_texture_png_t

typedef struct T *T;

extern T            gles_texture_png_new(const char *filepath);

/**
 * 释放贴图对象
 */
extern void         gles_texture_png_free(T *tx);

/**
 * 将路径指定的png图片直接加载到显存中的贴图对象中
 */
extern void         gles_texture_png_load(T tx);

/**
 * 获取贴图对象的高,宽, id
 */
extern unsigned int gles_texture_png_width(T tx);
extern unsigned int gles_texture_png_height(T tx);
extern unsigned int gles_texture_png_tid(T tx);

#undef T
#endif /*GLES_TEXTURE_PNG_INCLUDE*/
