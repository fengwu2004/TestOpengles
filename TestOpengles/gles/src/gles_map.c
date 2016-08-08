#include<sys/times.h>
#include<time.h>
#include<stdlib.h>
#include <string.h>
#include<stdio.h>
#include<math.h>
#include"assert.h"
#include"mem.h"
#include"vec2.h"
#include"utils.h"
#include"gles_matrix.h"
#include"gles_map.h"
#include"gles_vec_layer.h"
#include"gles_unit_layer.h"
#include"gles_marker_layer.h"
#include"gles_loc_layer.h"
#include"gles_texture_png.h"
#include"gles_2d_c_program.h"
#include"gles_touchmgr.h"
#include"gles_map_animation.h"


#ifdef ANDROID_IDRN
#include"error_functions.h"
#endif

#define TEXTURE_COUNT 2

#define T gles_map_t


struct T{
    int                 mwidth;
    int                 mheight;
    int                 vwidth;
    int                 vheight;
    int                 state;
    int                 statechange;
    int                 unitsetted;
    float               azimuth;
    float               deflection;
    float               scalemax;
    float               scalemin;
    gles_map_render_t   render;
    gles_map_animation_t anim;
    gles_touchmgr_t     tmgr;
    gles_vec_layer_t    gvl;
    gles_unit_layer_t   gul;
    gles_marker_layer_t gml;
    gles_loc_layer_t    gll;
    GLfloat             mapmat[16];
    GLfloat             rendermat[16];
};


static void _map_state_update(T gmap);

static void _follow_check(T gmap);
static void _navigate_check(T gmap);
static void _trace_check(T gmap);


T            
gles_map_new(gles_map_render_t render, const char *vbfile)
{
    T gmap;
    int len, e;

    gmap = ALLOC(sizeof(*gmap));
    gmap->render = render;

    gmap->scalemax = 10.0f;
    gmap->scalemin = 0.1f;
    gmap->azimuth = 0;

    gmap->state = MAP_STATE_NORMAL;
    gmap->statechange = 0;
    gmap->unitsetted = 0;

    gmap->tmgr = gles_touchmgr_new(gmap);
    gmap->anim = gles_map_animation_new(gmap);



    gmap->gvl = gles_vec_layer_new(gmap, vbfile);
    gmap->deflection = gles_vec_layer_azimuth(gmap->gvl);
    gmap->mwidth = gles_vec_layer_width(gmap->gvl);
    gmap->mheight = gles_vec_layer_height(gmap->gvl);


    gmap->gul = gles_unit_layer_new(gmap, 
                        gles_texture_png_tid(
                            gles_map_render_get_texture(gmap->render, 1)
                            ),
                        gmap->mwidth, gmap->mheight);

    gmap->gml = gles_marker_layer_new(gmap, gmap->mwidth, gmap->mheight);


    gmap->gll = gles_loc_layer_new(gmap, 
                gles_map_render_get_texture(gmap->render, 0),
                gmap->mwidth, gmap->mheight);


    gles_matrix_identity_4(gmap->mapmat, 0);

    return gmap;
}

void         
gles_map_free
(T *gm)
{
    assert(gm);
    assert(*gm);

    gles_vec_layer_free(&((*gm)->gvl));
    gles_unit_layer_free(&((*gm)->gul));
    gles_marker_layer_free(&((*gm)->gml));
    gles_loc_layer_free(&((*gm)->gll));
    gles_touchmgr_free(&((*gm)->tmgr));
    gles_map_animation_free(&((*gm)->anim));

    FREE(*gm);
}

void         
gles_map_get_navistatus
(T gm, struct nav_status *ns)
{
    assert(gm);
    gles_loc_layer_get_navistatus(gm->gll, ns);
}

int 
gles_map_get_state
(T gm)
{
    assert(gm);
    return gm->state;
}

void
gles_map_set_state
(T gm, int state)
{
    assert(gm);

    if(gm->state != state){
        gm->statechange = 1;
    }
    gm->state = state; 
}

float        
gles_map_get_scale_max
(T gm)
{
    assert(gm);
    return gm->scalemax;
}

float        
gles_map_get_scale_min
(T gm)
{
    assert(gm);
    return gm->scalemin;
}

float
gles_map_get_view_width(T gm)
{
    assert(gm);
    return gm->vwidth;
}

float
gles_map_get_view_height(T gm)
{
    assert(gm);
    return gm->vheight;
}

int          
gles_map_ontouch
(T gm, int action, float x0, float y0, float x1, float y1)
{
    /**
     * 如果在跟随模式和导航模式, 发生触摸事件
     * 自动回到normal模式
     **
    if(MAP_STATE_FOLLOW == gm->state || MAP_STATE_NAVIGATE == gm->state){
        gles_map_set_state(gm, MAP_STATE_NORMAL);
    }*/
    return gles_touchmgr_ontouch(gm->tmgr, action, x0, y0, x1, y1);
}

float        
gles_map_get_map_deflection
(T gm)
{
    assert(gm);
    return gm->deflection;
}

float
gles_map_get_rotate_degree
(T gm)
{
    float pos[8] = {0.0f, 1.0f, 0.0f, 1.0f,
                    0.0f, 0.0f, 0.0f, 1.0f};
    gles_matrix_mult_vec_4(gm->mapmat, pos, 0);
    gles_matrix_mult_vec_4(gm->mapmat, pos, 4);
    struct vec2 true_north  = {0, 1};
    struct vec2 true_org    = {0, 0};
    struct vec2 maped_north = {pos[0], pos[1]};
    struct vec2 maped_org   = {pos[4], pos[5]};

    true_north = vec2_normalize(vec2_sub(true_north, true_org));
    maped_north = vec2_normalize(vec2_sub(maped_north, maped_org));

    double cos_theta = vec2_dot_prod(maped_north, true_north);
    double radian_theta = acos(cos_theta);
    if(maped_north.x < 0){
        radian_theta = (2 * M_PI - radian_theta);
    }
    double degree_theta = utils_math_radian_2_deg(radian_theta);
    return (float)degree_theta;
}

float        
gles_map_get_scale_value
(T gm)
{
    float pos[8] = {0.0f, 1.0f, 0.0f, 1.0f,
                    0.0f, 0.0f, 0.0f, 1.0f};
    gles_matrix_mult_vec_4(gm->mapmat, pos, 0);
    gles_matrix_mult_vec_4(gm->mapmat, pos, 4);
    struct vec2 true_north  = {0, 1};
    struct vec2 true_org    = {0, 0};
    struct vec2 maped_north = {pos[0], pos[1]};
    struct vec2 maped_org   = {pos[4], pos[5]};

    true_north = vec2_sub(true_north, true_org);
    maped_north = vec2_sub(maped_north, maped_org);
    
    double mag_true = vec2_norm(true_north);
    double mag_maped= vec2_norm(maped_north);
    return (float)(mag_maped/mag_true);
}

void         
gles_map_matrix
(T gm, GLfloat *mapmat)
{
    memcpy(gm->mapmat, mapmat, sizeof(GLfloat)*16);
}

void         
gles_map_dummy_event
(T gm, int event, float x, float y)
{
    assert(gm);
    GLfloat inmmat[16];
    float pos[4];
    float mapedx, mapedy;

    pos[0] = x;
    pos[1] = gm->vheight - y;
    pos[2] = 0.0f;
    pos[3] = 1.0f;
    gles_matrix_invert_4(
            gm->mapmat,
            inmmat);
    gles_matrix_mult_vec_4(inmmat, pos, 0);
    mapedx = pos[0];
    mapedy = pos[1];
    switch(event){
        case 0:
            gles_map_animate_to(gm, mapedx, mapedy);
        break;

        case 1:
            gles_loc_layer_dummy_add_route_point(gm->gll, mapedx, mapedy);
        break;

        case 2:
            gles_loc_layer_clean_route_data(gm->gll);
        break;

        case 1024:
            gles_map_rotate_reset(gm);
        break;
        case 1025:
            gles_map_zoom_by(gm, 1.2f);
        break;
        case 1026:
            gles_map_zoom_by(gm, 0.8f);
        break;
        case 1027:
            gles_map_zoom_fit_width(gm);
        break;
        case 1028:
            gles_map_zoom_fit_height(gm);
        break;
        case 1029:
            gles_map_zoom_fit_fit(gm);
        break;
    }
}

//int          
//gles_map_add_marker_android
//(T gm, gles_texture_android_t txa, int mclass, float x, float y, float offsetx, float offsety, float scale)
//{
//    assert(gm);
//
//    return gles_marker_layer_insert_android(gm->gml, txa, mclass, x, y, offsetx, offsety, scale);
//}

void         
gles_map_markers_visible
(T gm, int visible)
{
    assert(gm);
    gles_marker_layer_set_visible(gm->gml, visible);
}

void         
gles_map_remove_marker
(T gm, int markerid)
{
    assert(gm);
    gles_marker_remove(gm->gml, markerid);
}

void         
gles_map_remove_marker_class
(T gm, int mclass)
{

    assert(gm);
    gles_marker_layer_remove_class(gm->gml, mclass);
}

void         
gles_map_clean_marker
(T gm)
{
    assert(gm);
    gles_marker_clean(gm->gml);
}

int          
gles_map_find_marker
(T gm, float x, float y)
{
    assert(gm);
    return gles_marker_layer_find(gm->gml, x, y);
}
int          
gles_map_update_marker_position
(T gm, int markerid, float x, float y)
{
    assert(gm);
    return gles_marker_update_position(gm->gml, markerid, x, y);
}

void         
gles_map_set_route_data
(T gm, int cnt, float *pts)
{
    assert(gm);
    gles_loc_layer_set_route_data(gm->gll, cnt, pts);
}

void         
gles_map_clean_route_data
(T gm)
{
    assert(gm);
    gles_loc_layer_clean_route_data(gm->gll);
}

void         
gles_map_set_location
(T gm, float x, float y)
{
    assert(gm);


    gles_loc_layer_set_pos(gm->gll, x, y);

    if(MAP_STATE_FOLLOW == gm->state){
        _follow_check(gm);
    }else if(MAP_STATE_NAVIGATE == gm->state){
        _navigate_check(gm);
    }else if(MAP_STATE_TRACE == gm->state){
        _trace_check(gm);
    }
}

void         
gles_map_animate_to
(T gm, float x, float y)
{
    if(MAP_STATE_NORMAL == gm->state||
            MAP_STATE_TRACE == gm->state){
        gles_loc_layer_animate_to(gm->gll, x, y);
    }else{
        gles_map_set_location(gm, x, y);
    }
}

void         
gles_map_clean_location(T gm)
{
    assert(gm);
    gles_loc_layer_clean_pos(gm->gll);
}


void         
gles_map_set_azimuth
(T gm, float azimuth)
{
    assert(gm);
    gm->azimuth = azimuth;
    /**
     * Normal, Trace和Navigate模式下
     * 方向指示器是自由的
     */
    gles_loc_layer_set_azimuth(gm->gll, azimuth);

    /**
     * 如果没位置, 跟随模式直接忽略
     */
    if(gles_loc_layer_isseted_pos(gm->gll)){

        if(MAP_STATE_FOLLOW == gm->state){
            _follow_check(gm);
        }
    }
}


void         
gles_map_viewsize
(T gm, int vwidth, int vheight)
{
    gm->vwidth = vwidth;
    gm->vheight= vheight;

    gles_unit_view_size(gm->gul, vwidth, vheight);
    gles_map_zoom_fit_width(gm);
}

void         
gles_map_set_unit_visible
(T gm, int visible)
{
    gles_unit_set_visible(gm->gul, visible);
}

void         
gles_map_insert_icon
(T gm, int id, int type, double left, double top, double right, double bottom)
{
    if(!gm->unitsetted)
        gles_unit_insert_icon(gm->gul, id, type, left, top, right, bottom);
}
void         
gles_map_insert_text
(T gm, int id, double left, double top, double right, double bottom, const char *text)
{
    if(!gm->unitsetted)
        gles_unit_insert_text(gm->gul, id, left, top, right, bottom, text);
}
void         
gles_map_set_unit_finish
(T gm)
{
    gm->unitsetted = 1;

#ifdef ANDROID_IDRN
    log_i("unit layer qt depth:%d", gles_unit_quad_tree_depth(gm->gul));
#endif
}
int          
gles_map_find_text
(T gm, float x, float y, float range)
{
    if(gm->unitsetted){
        return gles_unit_find_text(gm->gul, x, y, range);
    }else{
        return -1;
    }
}
int          
gles_map_find_icon
(T gm, float x, float y, float range)
{
    if(gm->unitsetted){
        return gles_unit_find_icon(gm->gul, x, y, range);
    }else{
        return -1;
    }
}

void         
gles_map_render
(T gm, gles_2d_c_program_t gcp, gles_2d_t_program_t gtp, 
 gles_font_t gft, float azimuth)
{


    GLfloat inmmat[16];
    int state = gm->state;

    if(!utils_math_float_equals(azimuth, gm->azimuth)){
        gles_map_set_azimuth(gm, azimuth);
    }

    _map_state_update(gm);

    /**
     * 因为手势可能会在一次渲染过程中修改地图变换矩阵
     * 造成unit漂移, 在渲染前拷贝地图变换矩阵, 一次渲染中都采用这个副本来做变换
     * 就可以解决这个问题
     */
    memcpy(gm->rendermat, gm->mapmat, sizeof(float)*16);
    gles_matrix_invert_4(gm->rendermat, inmmat);


    gles_2d_c_program_set_matrices_m(gcp, gm->rendermat);
    gles_vec_layer_render(gm->gvl, gcp);
    gles_2d_c_program_set_matrices_m(gcp, gm->rendermat);
    gles_unit_layer_render(gm->gul, gcp, gtp, gft, gm->rendermat, inmmat);



    gles_loc_layer_render_route(gm->gll, gcp, gtp, gft, state, gm->rendermat);

    gles_marker_layer_render(gm->gml, gtp, gm->rendermat);

    gles_loc_layer_render_location(gm->gll, gcp, gtp, gft, state, gm->rendermat);
}

const GLfloat *
gles_map_get_mapmat
(T gm)
{
    assert(gm);
    return gm->mapmat;
}

struct vec2
gles_map_screen_to_map
(T gm, float x, float y)
{
    assert(gm);
    GLfloat inmmat[16];
    float pos[4];

    pos[0] = x;
    pos[1] = gm->vheight - y;
    pos[2] = 0.0f;
    pos[3] = 1.0f;
    gles_matrix_invert_4(
            gm->mapmat,
            inmmat);
    gles_matrix_mult_vec_4(inmmat, pos, 0);
    struct vec2 retval = {pos[0], pos[1]};
    return retval;
}

struct vec2  
gles_map_map_to_screen
(T gm, float x, float y)
{
    assert(gm);
    float pos[4];

    pos[0] = x;
    pos[1] = y;
    pos[2] = 0.0f;
    pos[3] = 1.0f;
    gles_matrix_mult_vec_4(gm->mapmat, pos, 0);
    struct vec2 retval = {pos[0], gm->vheight - pos[1]};
    return retval;
}

float        
gles_map_screen_coord_y
(T gm, float glcoordy)
{
    return gm->mheight - glcoordy;
}

void         
gles_map_rotate_reset
(T gm)
{
    assert(gm);
    gles_map_animation_rotate_reset(gm->anim);
}

void         
gles_map_zoom_fit_width
(T gm)
{
    assert(gm);

    float zoom_rate;
    zoom_rate = (float)gm->vwidth / gm->mwidth;
    gles_matrix_identity_4(gm->mapmat, 0);
    gm->mapmat[0] = zoom_rate;
    gm->mapmat[5] = zoom_rate;

    //准备放在屏幕中心的目标点的地图坐标(即是地图中心)
    struct vec2 maped_target = {gm->mwidth/2.0f, gm->mheight/2.0f};


    //现在屏幕中心的地图坐标
    struct vec2 maped_center = 
        gles_map_screen_to_map(gm, 
                                gm->vwidth/2.0f,
                                gm->vheight/2.0f);
    //矢量差
    struct vec2 delta = vec2_sub(maped_center, maped_target);

    //按当前地图缩放比把矢量差做个变换
    delta = vec2_mul(delta, gles_map_get_scale_value(gm));

    //给地图变换矩阵附加这个平移变换
    gles_matrix_post_translate(gm->mapmat, delta.x, delta.y);
}

void         
gles_map_zoom_fit_height
(T gm)
{
    assert(gm);

    float zoom_rate;
    zoom_rate = (float)gm->vheight / gm->mheight;
    gles_matrix_identity_4(gm->mapmat, 0);
    gm->mapmat[0] = zoom_rate;
    gm->mapmat[5] = zoom_rate;

    //准备放在屏幕中心的目标点的地图坐标(即是地图中心)
    struct vec2 maped_target = {gm->mwidth/2.0f, gm->mheight/2.0f};


    //现在屏幕中心的地图坐标
    struct vec2 maped_center = 
        gles_map_screen_to_map(gm, 
                                gm->vwidth/2.0f,
                                gm->vheight/2.0f);
    //矢量差
    struct vec2 delta = vec2_sub(maped_center, maped_target);

    //按当前地图缩放比把矢量差做个变换
    delta = vec2_mul(delta, gles_map_get_scale_value(gm));

    //给地图变换矩阵附加这个平移变换
    gles_matrix_post_translate(gm->mapmat, delta.x, delta.y);
}

void         
gles_map_zoom_fit_fit
(T gm)
{
    assert(gm);
    float fw_rate, fh_rate;
    fw_rate = (float)gm->vwidth / gm->mwidth;
    fh_rate = (float)gm->vheight / gm->mheight;
    if(fw_rate > fh_rate){
        gles_map_zoom_fit_height(gm);
    }else {
        gles_map_zoom_fit_width(gm);
    }
}

void         
gles_map_zoom_by
(T gm, float rate)
{
    assert(gm);
    gles_man_animation_zoom_by(gm->anim, rate);
}

void         
gles_map_translate
(T gm, float tx, float ty)
{
    assert(gm);
    gles_map_animation_translate(gm->anim, tx, -ty);
}

void         
gles_map_center_to
(T gm, float x, float y)
{
    /**
     * 意图放在屏幕中心的目标点的地图坐标
     */
    struct vec2 maped_target = {x, y};

    /**
     * 现在屏幕中心的地图坐标
     */
    struct vec2 maped_center = 
        gles_map_screen_to_map(gm, 
                                gm->vwidth/2.0f,
                                gm->vheight/2.0f);

    /**
     * 矢量差
     */
    struct vec2 delta = vec2_sub(maped_center, maped_target);


    /**
     * 按当前地图变换矩阵把矢量差做个变换
     */
    delta = vec2_mul(delta, gles_map_get_scale_value(gm));
    delta = vec2_rotate(delta, -gles_map_get_rotate_degree(gm));

    /**
     * 准备一个平移动画
     */
    gles_map_animation_translate(gm->anim, delta.x, delta.y);
}


static 
void 
_map_state_update(T gmap)
{

    if(gmap->statechange){
        gles_map_animation_clean(gmap->anim);
        gmap->statechange = 0;

        if(MAP_STATE_NAVIGATE == gmap->state && gles_loc_layer_isseted_pos(gmap->gll)){
            /**
             * 如果切换到导航模式, 且位置已经设置过, 就用当前设置的位置, 再设置一次
             * 可以触发计算导航段, 和导航模式的动画
             */
            struct vec2 cur = gles_loc_layer_get_pos(gmap->gll);
            gles_map_set_location(gmap, cur.x, cur.y);
        }
    }else{

        /**
         * 当处于Trace状态, 需要检查
         * 定位位置是否超出屏幕
         */
        if(MAP_STATE_TRACE == gmap->state){

        }

        gles_map_animation_update(gmap->anim);
    }
}

static 
void 
_follow_check(T gmap)
{
    /**
     * 当没有动画在播放时
     * 准备位移旋转融合动画
     */
    if(!gles_map_animation_flag(gmap->anim)){
        struct vec2 start = gles_loc_layer_get_last_pos(gmap->gll);
        struct vec2 end   = gles_loc_layer_get_pos(gmap->gll);
        gles_loc_layer_pos_sync(gmap->gll);
        gles_map_animation_rotate_translate(gmap->anim, 
                /**
                 * 折以地图相对于正北的方向角
                 */
                gmap->azimuth - gmap->deflection,
                start.x, start.y, 
                end.x, end.y);
    }
}

void 
_navigate_check
(T gmap)
{
    /**
     * 当没有动画在播放时, 且导航计算有效
     * 准备位移旋转融合动画
     */
    int flag_anim, flag_navi;
    flag_anim = gles_map_animation_flag(gmap->anim);
    flag_navi = gles_loc_layer_isnavi_validate(gmap->gll);
    if(!flag_anim && flag_navi){
    /*
    if(!gles_map_animation_flag(gmap->anim) && 
            gles_loc_layer_isnavi_validate(gmap->gll)){*/
        struct vec2 start = gles_loc_layer_get_last_navi_proj_pos(gmap->gll);
        struct vec2 end   = gles_loc_layer_get_navi_proj_pos(gmap->gll);
        gles_loc_layer_navi_proj_sync(gmap->gll);
        gles_map_animation_rotate_translate(gmap->anim, 
                gles_loc_layer_navi_azimuth(gmap->gll),
                start.x, start.y, 
                end.x, end.y);
    }
}

static 
void 
_trace_check(T gmap)
{
    /**
     * 做一个出界检查
     * 如果出界就调用center_to把当前位置移动到屏幕中间
     */

    if(!gles_map_animation_flag(gmap->anim)){
        struct vec2  pos_map    = gles_loc_layer_get_pos(gmap->gll);
        struct vec2  pos_screen = gles_map_map_to_screen(gmap, pos_map.x, pos_map.y);

        if(pos_screen.x <= 0 || pos_screen.x >= gmap->vwidth ||
                pos_screen.y <=0 || pos_screen.y >= gmap->vheight){

            gles_map_center_to(gmap, pos_map.x, pos_map.y);
        }
    }
}
#undef TEXTURE_COUNT
