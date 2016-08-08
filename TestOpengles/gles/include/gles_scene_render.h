#ifndef GLES_SCENE_RENDER_INCLUDE
#define GLES_SCENE_RENDER_INCLUDE

#import <OpenGLES/ES3/gl.h>
#import <OpenGLES/ES3/glext.h>
#define T gles_scene_render_t


typedef struct T *T;

/**
 * 创建渲染器实例
 *
 */
extern T                    gles_scene_render_new             ();

/**
 * 初始化渲染器
 */
extern void                 gles_scene_render_init            (T gsr);

/**
 * 释放资源
 */
extern void                 gles_scene_render_free            (T *gsr);


/**
 * 通知渲染器 GLSurface的尺寸变动
 */
extern void                 gles_scene_render_resize          (T gsr, int width, int height);

/**
 * 渲染一帧
 */
extern void                 gles_scene_render_render          (T gsr);


/**
 * 处理触摸事件
 */
extern int                  gles_scene_render_ontouch         (T gsr, int action, float x0, float y0, float x1, float y1);

/**
 * 测试时, 由UI发送一些自定义的事件到渲染器
 * 简化接口
 */
extern void                 gles_scene_render_event           (T gsr, int event, float x, float y);


/**
 * 设置和访问场景的世界矩阵
 */
extern void                 gles_scene_render_set_world_matrix       (T gsr, GLfloat *wmat);
extern const GLfloat       *gles_scene_render_get_world_matrix       (T gsr);


#undef T
#endif /*GLES_MAP_RENDER_INCLUDE*/
