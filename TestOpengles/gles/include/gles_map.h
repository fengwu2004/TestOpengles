#ifndef GLES_MAP_INCLUDE
#define GLES_MAP_INCLUDE

#include"vec2.h"
#include"routeresult.h"
#include"gles_2d_c_program.h"
#include"gles_2d_t_program.h"
#include"gles_font.h"
#include"gles_map_render.h"
#import <OpenGLES/ES3/gl.h>
#import <OpenGLES/ES3/glext.h>
#define MAP_STATE_UNKNOWN  -1
#define MAP_STATE_NORMAL    0
#define MAP_STATE_TRACE     1
#define MAP_STATE_FOLLOW    2
#define MAP_STATE_NAVIGATE  3

#define TOUCH_ACTION_DOWN   0
#define TOUCH_ACTION_MOVE   1
#define TOUCH_ACTION_UP     2
#define TOUCH_MULTI_DOWN    3
#define TOUCH_MULTI_UP      4


#define T gles_map_t

typedef struct T *T;

extern void         gles_map_dummy_event(T gm, int event, float x, float y);

/**
 * 新建/释放 gl地图渲染对象
 */
extern T            gles_map_new(gles_map_render_t render, const char *vbfile);
extern void         gles_map_free(T *gm);

/**
 * 设置/获取地图状态
 * MAP_STATE_NORMAL     普通无任何限制
 * MAP_STATE_TRACE      方向指示器如果移动出屏幕, 会触发将它居中的动画
 * MAP_STATE_FOLLOW     方向指示器永远居于屏幕中心, 方向指示器锁定指向正上方, 转动/移动地图来响应方向和位置改变
 * MAP_STATE_NAVIGATE   方向指示器永远居于屏幕中心, 地图方向保持使当前导航段指向正上方, 方向指示器自由
 */
extern int          gles_map_get_state(T gm);
extern void         gles_map_set_state(T gm, int state);

/**
 * 获取导航状态
 */
extern void         gles_map_get_navistatus(T gm, struct nav_status *ns);

/**
 * 从外部设置/获取地图变换矩阵
 * 主要给附属工具类使用
 */
extern void         gles_map_matrix(T gm, GLfloat *mapmat);
extern const GLfloat *gles_map_get_mapmat(T gm);


/**
 * 设置渲染地图的GLSurface的大小
 * 提供给外部环境调用
 */
extern void         gles_map_viewsize(T gm, int vwidth, int vheight);

/**
 * 设置unit层可见性 (主要用于设置unit数据时禁用unit渲染)
 * 添加icon类的unit
 * 添加text类unit
 * 设置添加unit结束
 * 查找icon类unit 返回unit索引, 找不到返回-1
 * 查找text类unit 返回unit索引, 找不到返回-1
 */
extern void         gles_map_set_unit_visible(T gm, int visible);
extern void         gles_map_insert_icon(T gm, int id, int type, double left, double top, double right, double bottom);
extern void         gles_map_insert_text(T gm, int id, double left, double top, double right, double bottom, const char *text);
extern void         gles_map_set_unit_finish(T gm);
extern int          gles_map_find_text(T gm, float x, float y, float range);
extern int          gles_map_find_icon(T gm, float x, float y, float range);

/**
 * 渲染地图, 提供给外部环境调用
 */
extern void         gles_map_render(T gm, 
                    gles_2d_c_program_t gcp, gles_2d_t_program_t gtp, gles_font_t gft,
                    float azimuth);


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
extern int          gles_map_ontouch(T gm, int action, float x0, float y0, float x1, float y1);

/**
 * marker相关接口
 */
extern void         gles_map_markers_visible(T gm, int visible);
extern void         gles_map_remove_marker(T gm, int markerid);
extern void         gles_map_remove_marker_class(T gm, int mclass);
extern void         gles_map_clean_marker(T gm);
extern int          gles_map_find_marker(T gm, float x, float y);
extern int          gles_map_update_marker_position(T gm, int markerid, float x, float y);


/**
 * 设置位置marker到x, y(地图坐标)
 */
extern void         gles_map_set_location(T gm, float x, float y);
/**
 * 播放动画将marker移动到x, y(地图坐标)
 * 如果之前没有设置位置, 或者处于导航/跟随模式 等同与调用
 * gles_map_set_location
 */
extern void         gles_map_animate_to(T gm, float x, float y);
/**
 * 清除位置设置
 */
extern void         gles_map_clean_location(T gm);
/**
 * 设置位置指示marker的方向
 */
extern void         gles_map_set_azimuth(T gm, float azimuth);


/**
 * 获取地图相对于正北的偏移角度
 */
extern float        gles_map_get_map_deflection(T gm);

/**
 * 获取地图的缩放限制
 */
extern float        gles_map_get_scale_max(T gm);
extern float        gles_map_get_scale_min(T gm);


/**
 * 获取地图旋转角度
 */
extern float        gles_map_get_rotate_degree(T gm);

/**
 * 获取地图缩放比
 */
extern float        gles_map_get_scale_value(T gm);

/**
 * 获取GLSurface的高宽
 */
extern float        gles_map_get_view_width(T gm);
extern float        gles_map_get_view_height(T gm);

/**
 * 设置/清除路径规划数据
 */
extern void         gles_map_set_route_data(T gm, int cnt, float *pts);
extern void         gles_map_clean_route_data(T gm);

/**
 * 坐标变换
 * 从屏幕坐标到地图坐标
 * 从地图坐标到屏幕坐标
 */
extern struct vec2  gles_map_screen_to_map(T gm, float x, float y);
extern struct vec2  gles_map_map_to_screen(T gm, float x, float y);

/**
 * 由于地图是按屏幕绘图原点来建立的
 * 原点在左上角
 *
 * 而OpenGL我们的正投影矩阵指定的原点在左下角,
 * 所以外部app调用screen_to_map函数后
 * 需要再校正y坐标
 */
extern float        gles_map_screen_coord_y(T gm, float glcoordy);

/**
 * 按适应 宽度/高度 缩放并居中
 */
extern void         gles_map_zoom_fit_width(T gm);
extern void         gles_map_zoom_fit_height(T gm); 
extern void         gles_map_zoom_fit_fit(T gm);

/**
 * 播放动画重置地图旋转
 */
extern void         gles_map_rotate_reset(T gm);
/**
 * 播放动画按相对比例缩放
 * 例如
 * rate=1.2 就放大1.2倍
 * rate=0.8 会缩小到当前的0.8倍
 * 超过地图缩放限制的部分会截断
 */
extern void         gles_map_zoom_by(T gm, float rate);

/**
 * 地图平移动画
 */
extern void         gles_map_translate(T gm, float tx, float ty);

/**
 * 播放动画将地图坐标x, y的点移动到屏幕中心
 */
extern void         gles_map_center_to(T gm, float x, float y);

#undef T
#endif /*GLES_MAP_INCLUDE*/
