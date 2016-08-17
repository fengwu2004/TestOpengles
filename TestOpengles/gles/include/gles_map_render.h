#ifndef GLES_MAP_RENDER_INCLUDE
#define GLES_MAP_RENDER_INCLUDE

#import <OpenGLES/ES3/gl.h>
#import <OpenGLES/ES3/glext.h>
#include "vec2.h"
#include"routeresult.h"
#include"gles_texture_png.h"
//#include"gles_texture_android.h"

#define T gles_map_render_t

#define ZOOM_FIT_WIDTH  0
#define ZOOM_FIT_HEIGHT 1
#define ZOOM_FIT_FIT    2

typedef struct T *T;

/**
 * 创建渲染器实例
 *
 * folder   默认目录    渲染器会从该目录加载默认字体文件和默认图标贴图
 */
extern T                    gles_map_render_new             (const char* folder);

/**
 * 释放资源
 */
extern void                 gles_map_render_free            (T *gmr);

/**
 * 初始化,可以在初始化时指定显示的地图, 也可以传空, 不渲染地图
 */
extern void                 gles_map_render_init            (T gmr);


/**
 * 通知渲染器 GLSurface的尺寸变动
 */
extern void                 gles_map_render_resize          (T gmr, int width, int height);

/**
 * 渲染一帧
 */
extern void                 gles_map_render_render          (T gmr, float azimuth);


/**
 * 设置要渲染的地图
 */
extern void                 gles_map_render_setmap          (T gmr, const char *vbfile);

/**
 * 获取render当前map的旋转角度
 *
 */
extern float                gles_map_render_rotate_degree   (T gmr);

/**
 * 获取预先加载的贴图
 */
extern gles_texture_png_t   gles_map_render_get_texture     (T gmr, int index);

/**
 * 获取导航状态
 */
extern void                 gles_map_render_get_navistatus  (T gmr, struct nav_status *ns);

/**
 * 处理触摸事件
 */
/**
 * 传入触摸事件
 * action 的取值:
 * TOUCH_ACTION_DOWN 单点按下
 * TOUCH_ACTION_MOVE 单点/多点移动
 * TOUCH_ACTION_UP   单点抬起
 * TOUCH_MULTI_DOWN  多点按下
 * TOUCH_MULTI_UP    多点抬起
 *
 * 单点事件的时候, 只需填充x0, y0
 * 多点事件时, x0, y0, x1, y1都要填充, 目前也只支持两点手势
 * 如果有必要再扩展
 */
extern int                  gles_map_render_ontouch         (T gmr, int action, float x0, float y0, float x1, float y1);

/**
 * 测试时, 由UI发送一些自定义的事件到渲染器
 * 简化接口
 */
extern void                 gles_map_render_event           (T gmr, int event, float x, float y);

/**
 * marker相关接口
 */
//extern int                  gles_map_render_add_marker_android(T gmr, gles_texture_android_t txa, int mclass,
//                                                float x, float y, float offsetx, float offsety, float scale);
extern void                 gles_map_render_markers_visible(T gmr, int visible);
extern void                 gles_map_render_remove_marker(T gmr, int markerid);
extern void                 gles_map_render_remove_marker_class(T gmr, int mclass);
extern void                 gles_map_render_clean_marker(T gmr);
extern int                  gles_map_render_find_marker(T gmr, float x, float y);
extern int                  gles_map_render_update_marker_position(T gmr, int markerid, float x, float y);

/**
 * 当前地图, 的屏幕到地图/地图到屏幕坐标映射
 * 如果当前没有指定地图, 返回(0, 0)
 */
extern struct vec2          gles_map_render_screen_to_map   (T gmr, float x, float y);
extern struct vec2          gles_map_render_map_to_screen   (T gmr, float x, float y);

/**
 * 设置unit层可见性 (主要用于设置unit数据时禁用unit渲染)
 * 添加icon类的unit
 * 添加text类unit
 */
extern void                 gles_map_render_set_unit_visible(T gmr, int visible);
extern void                 gles_map_render_insert_icon(T gmr, int id, int type, double left, double top, double right, double bottom);
extern void                 gles_map_render_insert_text(T gmr, int id, double left, double top, double right, double bottom, const char *text);
extern void                 gles_map_render_set_unit_finish(T gmr);
extern int                  gles_map_render_find_text(T gmr, float x, float y, float range);
extern int                  gles_map_render_find_icon(T gmr, float x, float y, float range);

/**
 * 设置/清除路径规划数据
 */
extern void                 gles_map_render_set_route_data(T gmr, int cnt, float *pts);
extern void                 gles_map_render_clean_route_data(T gmr);

/**
 * 将地图对显示的View进行适应缩放
 * type:
 *      ZOOM_FIT_WIDTH  0   适应宽度缩放
 *      ZOOM_FIT_HEIGHT 1   适应高度缩放
 *      ZOOM_FIT_FIT    2   选择适应宽度和高度之间所需缩放比最小的那种进行缩放
 */
extern void                 gles_map_render_zoom_fit        (T gmr, int type);
/**
 * 给渲染器的当前地图设置状态
 */
extern void                 gles_map_render_set_state       (T gmr, int state);

/**
 * 给渲染器的当前地图设置/清除定位位置
 */
extern void                 gles_map_render_set_location    (T gmr, float x, float y);
extern void                 gles_map_render_clean_location  (T gmr);

/**
 * 当前地图动画: 旋转重置
 */
extern void                 gles_map_render_anim_rotate_reset    (T gmr);
/**
 * 当前地图动画: 比例缩放
 */
extern void                 gles_map_render_anim_zoom_by         (T gmr, float rate);
/**
 * 当前地图动画: 定位Marker平移到指定位置
 */
extern void                 gles_map_render_anim_to              (T gmr, float x, float y);

/**
 * 当前地图动画: 指定位置平移到屏幕中心
 */
extern void                 gles_map_render_anim_center_to       (T gmr, float x, float y);

#undef T
#endif /*GLES_MAP_RENDER_INCLUDE*/
