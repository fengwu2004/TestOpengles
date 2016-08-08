#ifndef GLES_LOC_LAYER_INCLUDE
#define GLES_LOC_LAYER_INCLUDE

#import <OpenGLES/ES3/gl.h>
#import <OpenGLES/ES3/glext.h>
#include"routeresult.h"
#include"gles_2d_c_program.h"
#include"gles_2d_t_program.h"
#include"gles_texture_png.h"
#include"gles_map.h"
#include"vec2.h"

#define T gles_loc_layer_t

typedef struct T *T;


extern T            gles_loc_layer_new(gles_map_t map, gles_texture_png_t marker, int width, int height);

extern void         gles_loc_layer_free(T *gll);

/**
 * 测试时用于添加路点
 */
extern void         gles_loc_layer_dummy_add_route_point(T gll, float x, float y);

/**
 * 清除和设置路点数据集
 */
extern void         gles_loc_layer_clean_route_data(T gll);
extern void         gles_loc_layer_set_route_data(T gll, int cnt, float *pts);

/**
 * 定位层独立动画, 将光标移动到某地图坐标
 */
extern void         gles_loc_layer_animate_to(T gll, float x, float y);

/**
 * 直接设置位置指示marker
 */
extern void         gles_loc_layer_set_pos(T gll, float x, float y);

/**
 * 判断是否有设置marker
 */
extern int          gles_loc_layer_isseted_pos(T gll);

/**
 * 获取位置坐标
 */
extern struct vec2  gles_loc_layer_get_pos(T gll);

/**
 * 获取上次设置的位置
 */
extern struct vec2  gles_loc_layer_get_last_pos(T gll);

/**
 * 同步上次设置的位置和当前位置
 */
extern void         gles_loc_layer_pos_sync(T gll);

/**
 * 获取导航状态
 */
extern void         gles_loc_layer_get_navistatus(T gll, struct nav_status *ns);

/**
 * 导航计算结果是否有效
 */
extern int          gles_loc_layer_isnavi_validate(T gll);

/**
 * 获取当前导航投影位置
 */
extern struct vec2  gles_loc_layer_get_navi_proj_pos(T gll);

/**
 * 获取上次导航投影位置
 */
extern struct vec2  gles_loc_layer_get_last_navi_proj_pos(T gll);

/**
 * 同步上次导航投影位置和当前导航投影位置
 */
extern void         gles_loc_layer_navi_proj_sync(T gll);

/**
 * 获取当前导航段方位角
 */
extern float        gles_loc_layer_navi_azimuth(T gll);


/**
 * 清除导航位置
 */
extern void         gles_loc_layer_clean_pos(T gll);


/**
 * 设置导航位置指示器的方位角度显示, Follow状态忽略此设置
 */
extern void         gles_loc_layer_set_azimuth(T gll, float azimuth);

/**
 * 渲染导航路径
 */
extern void         gles_loc_layer_render_route(T gll, 
                        gles_2d_c_program_t gcp, 
                        gles_2d_t_program_t gtp, 
                        gles_font_t  font, 
                        int         state,
                        const GLfloat *mmat);

/**
 * 渲染定位标记
 */
extern void         gles_loc_layer_render_location(T gll, 
                        gles_2d_c_program_t gcp, 
                        gles_2d_t_program_t gtp, 
                        gles_font_t  font, 
                        int         state,
                        const GLfloat *mmat);


#undef T
#endif /*GLES_LOCATION_LAYER_INCLUDE*/
