#ifndef GLES_TEXTURE_ANDROID_INCLUDE
#define GLES_TEXTURE_ANDROID_INCLUDE

/**
 * 为了支援从Android的Bitmap类直接创建贴图
 * 而实现的特别支持类
 */

#include <jni.h>

#define T gles_texture_android_t

typedef struct T *T;

extern T            gles_texture_android_new(JNIEnv *env, jobject bitmap);

/**
 * 释放贴图对象
 */
extern void         gles_texture_android_free(T *txa);

/**
 * 在GL 线程中加载
 */
extern int          gles_texture_android_load(T txa);

/**
 * 获取贴图对象的高,宽, tid
 */
extern int          gles_texture_android_width(T txa);
extern int          gles_texture_android_height(T txa);
extern unsigned int gles_texture_android_tid(T txa);

#undef T


#endif /*GLES_TEXTURE_ANDROID_INCLUDE*/
