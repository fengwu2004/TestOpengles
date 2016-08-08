#include<math.h>
#include"assert.h"
#include"mem.h"
#include"vec2.h"
#include"gles_touchmgr.h"
#include "gles_matrix.h"
#include "utils.h"
#define T gles_touchmgr_t
#include <string.h>

/**
 * 触摸策略
 */
struct touch_strategy{
    const char *name;
    int (*on_touch_down)(T tm, float x, float y);
    int (*on_touch_up)  (T tm, float x, float y);
    int (*on_touch_move)(T tm, float x0, float y0, float x1, float y1);
    int (*on_multi_down)(T tm, float x0, float y0, float x1, float y1);
    int (*on_multi_up)(T tm, float x0, float y0, float x1, float y1);
};


/**
 * 触摸管理器
 */
struct T{
    gles_map_t              map;
    int                     enable;
    struct touch_strategy  *strategy;
    float                   startx;
    float                   starty;
    float                   translatex;
    float                   translatey;
    float                   prevx;
    float                   prevy;
    float                   midx_map;
    float                   midy_map;
    float                   midx_touch;
    float                   midy_touch;
    float                   startscale;
    float                   startspacing;
    float                   dist_alpha;
    float                   dist_beta;
    float                   dist_gamma;
    float                   first_degree;
    GLfloat                 mapmatbuf[16];
    GLfloat                 tmpmatbuf[16];
    GLfloat                 savedmat[16];
};


static void     _save_matrix(T tm);
static float    _spacing(float x0, float y0, float x1, float y1);
static void     _mid_point(T tm, float x0, float y0, float x1, float y1);
static int      _rotate_detect(T tm);
static float    _rotate_degree(float x0, float y0, float x1, float y1);
static void     _post_translate(T tm);
static void     _post_scale(T tm, float scale);
static void     _post_rotation(T tm, float rotate);
static void     _post_scale_rotation(T tm, float scale, float rotate);

static int _none_touch_down(T tm, float x, float y);
static int _none_multi_down(T tm, float x0, float y0, float x1, float y1);

static int _scroll_touch_move(T tm, float x0, float y0, float x1, float y1);
static int _scroll_touch_up(T tm, float x, float y);
static int _scroll_multi_down(T tm, float x0, float y0, float x1, float y1);

static int _multi_touch_move(T tm, float x0, float y0, float x1, float y1);
static int _multi_multi_up(T tm, float x0, float y0, float x1, float y1);

static int _zoom_touch_move(T tm, float x0, float y0, float x1, float y1);
static int _zoom_multi_up(T tm, float x0, float y0, float x1, float y1);

static int _rotate_touch_move(T tm, float x0, float y0, float x1, float y1);
static int _rotate_multi_up(T tm, float x0, float y0, float x1, float y1);

static struct touch_strategy    _none_strategy = {
    .name           = "none",
    .on_touch_down  = _none_touch_down,
    .on_touch_up    = NULL,
    .on_touch_move  = NULL,
    .on_multi_down  = _none_multi_down,
    .on_multi_up    = NULL,
};
static struct touch_strategy    _scroll_strategy = {
    .name           = "scroll",
    .on_touch_down  = NULL,
    .on_touch_up    = _scroll_touch_up,
    .on_touch_move  = _scroll_touch_move,
    .on_multi_down  = _scroll_multi_down,
    .on_multi_up    = NULL,
};
static struct touch_strategy    _multi_strategy = {
    .name           = "multi",
    .on_touch_down  = NULL,
    .on_touch_up    = NULL,
    .on_touch_move  = _multi_touch_move,
    .on_multi_down  = NULL,
    .on_multi_up    = _multi_multi_up,
};
static struct touch_strategy    _zoom_strategy = {
    .name           = "zoom",
    .on_touch_down  = NULL,
    .on_touch_up    = NULL,
    .on_touch_move  = _zoom_touch_move,
    .on_multi_down  = NULL,
    .on_multi_up    = _zoom_multi_up,
};
static struct touch_strategy    _rotate_strategy = {
    .name           = "rotate",
    .on_touch_down  = NULL,
    .on_touch_up    = NULL,
    .on_touch_move  = _rotate_touch_move,
    .on_multi_down  = NULL,
    .on_multi_up    = _rotate_multi_up,
};

T    
gles_touchmgr_new
(gles_map_t map)
{
    T tm;

    tm = ALLOC(sizeof(*tm));
    tm->enable = 1;
    tm->map = map;
    tm->strategy = &_none_strategy;

    return tm;
}

void 
gles_touchmgr_free
(T *tm)
{
    assert(tm);
    assert(*tm);

    FREE(*tm);
}

void
gles_touchmgr_enable
(T tm, int enable)
{
    assert(tm);
    if(enable != tm->enable){
        tm->strategy = &_none_strategy;
    }
    tm->enable = enable;
}

int  
gles_touchmgr_ontouch
(T tm, int action, float x0, float y0, float x1, float y1)
{
    int retval = 0;
    
    assert(tm);

    if(tm->enable && tm->strategy){
        switch(action){
            case TOUCH_ACTION_DOWN:
                if(tm->strategy->on_touch_down)
                    retval = tm->strategy->on_touch_down(tm, x0, y0);
                break;
            case TOUCH_ACTION_MOVE:
                if(tm->strategy->on_touch_move)
                    retval = tm->strategy->on_touch_move(tm, x0, y0, x1, y1);
                break;
            case TOUCH_ACTION_UP:
                if(tm->strategy->on_touch_up)
                    retval = tm->strategy->on_touch_up(tm, x0, y0);
                break;
            case TOUCH_MULTI_DOWN:
                if(tm->strategy->on_multi_down)
                    retval = tm->strategy->on_multi_down(tm, x0, y0, x1, y1);
                break;
            case TOUCH_MULTI_UP:
                if(tm->strategy->on_multi_up)
                    retval = tm->strategy->on_multi_up(tm, x0, y0, x1, y1);
                break;
        }
    }
    return retval;
}


static 
void     
_save_matrix(T tm)
{
    memcpy(tm->savedmat, gles_map_get_mapmat(tm->map), sizeof(GLfloat)*16);
}

static 
float 
_spacing(float x0, float y0, float x1, float y1)
{
    float dx, dy;
    dx = x1 - x0;
    dy = y1 - y0;
    return sqrt(dx*dx + dy*dy);
}

static 
void     
_mid_point(T tm, float x0, float y0, float x1, float y1)
{

    tm->midx_touch = (x0 + x1)/2.0f;
    tm->midy_touch = (y0 + y1)/2.0f;
    tm->midx_map = tm->midx_touch;
    tm->midy_map = tm->midy_touch - gles_map_get_view_height(tm->map);
}

static 
int      
_rotate_detect(T tm)
{
    float cos, angle;
    int retval = 0;

    do{
        cos = (tm->dist_alpha*tm->dist_alpha + tm->dist_beta*tm->dist_beta - tm->dist_gamma*tm->dist_gamma)/
                (3 * tm->dist_alpha * tm->dist_beta);
        if(isnan(cos)){
            break;
        }

        angle = acosf(cos) * (180.0f / M_PI);

        if(angle < 150 && angle > 30){
            retval = 1;
        }
    }while(0);

    return retval;
}

static 
float    
_rotate_degree(float x0, float y0, float x1, float y1)
{
    float radian, degree;
    radian = atan2f(y1 - y0, x0 - x1);
    degree = 360.0f * radian / (2 * M_PI);
    return degree;
}

static 
void     
_post_translate(T tm)
{
    memcpy(tm->mapmatbuf, tm->savedmat, sizeof(GLfloat)*16);

    gles_matrix_translate_4(tm->tmpmatbuf, 0,
                     tm->translatex, -tm->translatey, 0);
    gles_matrix_mult_matrix_4(tm->tmpmatbuf, tm->mapmatbuf);
    gles_map_matrix(tm->map, tm->tmpmatbuf);
}

static 
void     
_post_scale_rotation(T tm, float scale, float rotate)
{

    float x, y;
    float   mt1[16];
    float   ms[16];
    float   mr[16];
    float   mt2[16];

    x = tm->midx_map;
    y = -tm->midy_map;

    memcpy(tm->mapmatbuf, tm->savedmat, sizeof(GLfloat)*16);

    gles_matrix_translate_4(mt1, 0, -x, -y, 0);
    gles_matrix_translate_4(mt2, 0, x, y, 0);
    gles_matrix_rotate2d_4(mr, rotate);
    ms[0] = scale;
    ms[1] = 0.0f;
    ms[2] = 0.0f;
    ms[3] = 0.0f;

    ms[4] = 0.0f;
    ms[5] = scale;
    ms[6] = 0.0f;
    ms[7] = 0.0f;

    ms[8] = 0.0f;
    ms[9] = 0.0f;
    ms[10]= 1.0f;
    ms[11]= 0.0f;

    ms[12]= 0.0f;
    ms[13]= 0.0f;
    ms[14]= 0.0f;
    ms[15]= 1.0f;

    
    gles_matrix_mult_matrix_4(mt1, tm->mapmatbuf);
    gles_matrix_mult_matrix_4(ms, mt1);
    gles_matrix_mult_matrix_4(mr, ms);
    gles_matrix_mult_matrix_4(mt2, mr);

    gles_map_matrix(tm->map, mt2);
}


static 
void     
_post_scale(T tm, float scale)
{

    float x, y;
    float   mt1[16];
    float   ms[16];
    float   mt2[16];

    x = tm->midx_map;
    y = -tm->midy_map;

    memcpy(tm->mapmatbuf, tm->savedmat, sizeof(GLfloat)*16);

    gles_matrix_translate_4(mt1, 0, -x, -y, 0);
    gles_matrix_translate_4(mt2, 0, x, y, 0);
    ms[0] = scale;
    ms[1] = 0.0f;
    ms[2] = 0.0f;
    ms[3] = 0.0f;

    ms[4] = 0.0f;
    ms[5] = scale;
    ms[6] = 0.0f;
    ms[7] = 0.0f;

    ms[8] = 0.0f;
    ms[9] = 0.0f;
    ms[10]= 1.0f;
    ms[11]= 0.0f;

    ms[12]= 0.0f;
    ms[13]= 0.0f;
    ms[14]= 0.0f;
    ms[15]= 1.0f;

    
    gles_matrix_mult_matrix_4(mt1, tm->mapmatbuf);
    gles_matrix_mult_matrix_4(ms, mt1);
    gles_matrix_mult_matrix_4(mt2, ms);

    gles_map_matrix(tm->map, mt2);
}

static 
void     
_post_rotation(T tm, float rotate)
{

    float x, y;
    float   mt1[16];
    float   mr[16];
    float   mt2[16];

    x = tm->midx_map;
    y = -tm->midy_map;

    memcpy(tm->mapmatbuf, tm->savedmat, sizeof(GLfloat)*16);

    gles_matrix_translate_4(mt1, 0, -x, -y, 0);
    gles_matrix_translate_4(mt2, 0, x, y, 0);
    gles_matrix_rotate2d_4(mr, rotate);
    
    gles_matrix_mult_matrix_4(mt1, tm->mapmatbuf);
    gles_matrix_mult_matrix_4(mr, mt1);
    gles_matrix_mult_matrix_4(mt2, mr);

    gles_map_matrix(tm->map, mt2);
}

static 
int 
_none_touch_down(T tm, float x, float y)
{
    _save_matrix(tm);
    tm->startx      = x;
    tm->starty      = y;
    tm->prevx       = x;
    tm->prevy       = y;
    tm->translatex  = 0;
    tm->translatey  = 0;
    tm->strategy = &_scroll_strategy;
    return 1;
}
static 
int 
_none_multi_down(T tm, float x0, float y0, float x1, float y1)
{
    tm->dist_alpha = tm->startspacing = _spacing(x0, y0, x1, y1);
    if(10.0f < tm->startspacing){

        _save_matrix(tm);
        tm->startx = x0;
        tm->starty = y0;
        tm->prevx  = x0;
        tm->prevy  = y0;
        tm->startscale = gles_map_get_scale_value(tm->map);
        _mid_point(tm, x0, y0, x1, y1);
        tm->strategy = &_multi_strategy;
        return 1;
    }
    return 0;
}

static 
int 
_scroll_touch_move(T tm, float x0, float y0, float x1, float y1)
{

    tm->translatex += (x0 - tm->prevx);
    tm->translatey += (y0 - tm->prevy);
    tm->prevx = x0;
    tm->prevy = y0;

    /**
     * xoozi 给矩阵附加平移变化
    _matrix.set(_savedMatrix);
    _matrix.postTranslate(_tx, -_ty);*/
    _post_translate(tm);
    return 1;
}
static 
int 
_scroll_touch_up(T tm, float x, float y)
{
    tm->translatex += (x - tm->prevx);
    tm->translatey += (y - tm->prevy);
    tm->prevx = x;
    tm->prevy = y;

    _post_translate(tm);
    tm->strategy = &_none_strategy;
    return 1;
}
static 
int 
_scroll_multi_down(T tm, float x0, float y0, float x1, float y1)
{
    tm->dist_alpha = tm->startspacing = _spacing(x0, y0, x1, y1);
    if(10.0f < tm->startspacing){
        _save_matrix(tm);
        tm->startx = x0;
        tm->starty = y0;
        tm->prevx  = x0;
        tm->prevy  = y0;
        tm->startscale = gles_map_get_scale_value(tm->map);
        _mid_point(tm, x0, y0, x1, y1);
        tm->strategy = &_multi_strategy;
        return 1;
    }
    return 0;
}

static 
int 
_multi_touch_move(T tm, float x0, float y0, float x1, float y1)
{
    tm->first_degree = _rotate_degree(x0, y0, x1, y1);
    tm->strategy = &_rotate_strategy;
    /*
     * 这样看起来效果好点, 直接舍弃缩放状态
     * 否则在触摸反应慢的机器上, 判断缩放到旋转有个小延迟
    tm->dist_beta = _spacing(tm->startx, tm->starty, x0, y0);
    tm->dist_gamma = _spacing(tm->midx_touch, tm->midy_touch, x0, y0);
    if(_rotate_detect(tm)){
        tm->first_degree = _rotate_degree(x0, y0, x1, y1);
        tm->strategy = &_rotate_strategy;
    }else{
        tm->strategy = &_zoom_strategy;
    }*/
    return 1;
}
static 
int 
_multi_multi_up(T tm, float x0, float y0, float x1, float y1)
{
    tm->strategy = &_none_strategy;
    return 1;
}

static 
int 
_zoom_touch_move(T tm, float x0, float y0, float x1, float y1)
{
    float new_spacing = _spacing(x0, y0, x1, y1);
    int retval = 0;
    do{
        /**
         * 手势跨度小, 忽略
         */
        if(new_spacing < 10)
            break;

        /**
         * 由于缩放时, 其实每步重置了矩阵, 然后post算出来的比例,
         * 所以 这个计算比例的zoom不能用当前取的, 要用开始缩放策略时记录的
         */
        float zoom = tm->startscale;
        float zoom_rate = new_spacing / tm->startspacing;
        float zoom_value = zoom * zoom_rate;

        /**
         * 将当前缩放比乘以手势缩放比例后
         * 做区间限制
         */
        float max = gles_map_get_scale_max(tm->map);
        float min = gles_map_get_scale_min(tm->map);
        if (zoom_value > max) {
            zoom_value = max;
        } else if (zoom_value < min) {
            zoom_value = min;
        }

        /**
         * 如果区间限制后的缩放比和当前缩放比相同
         * 也忽略
         */
        if(utils_math_float_equals(zoom_value, zoom))
            break;

        /**
         * 确实需要缩放, 计算缩放比
         */
        zoom_rate = zoom_value / zoom;

        /**xoozi
        _matrix.set(_savedMatrix);
        _matrix.postScale(zoom_rate, zoom_rate, _mid.x, -_mid.y);
        */
        _post_scale(tm, zoom_rate);

        retval = 1;
    } while (0);

    /** 动起来发现旋转了, 可以转换为旋转*/
    tm->dist_beta = _spacing(tm->startx, tm->starty, x0, y0);
    tm->dist_gamma = _spacing(tm->midx_touch, tm->midy_touch, x0, y0);
    if(_rotate_detect(tm)){
        tm->first_degree = _rotate_degree(x0, y0, x1, y1);
        tm->strategy = &_rotate_strategy;
        retval = 1;
    }
    return retval;
}

static 
int 
_zoom_multi_up(T tm, float x0, float y0, float x1, float y1)
{
    float new_spacing = _spacing(x0, y0, x1, y1);
    int retval = 0;
    do{
        /**
         * 手势跨度小, 忽略
         */
        if(new_spacing < 10)
            break;

        /**
         * 由于缩放时, 其实每步重置了矩阵, 然后post算出来的比例,
         * 所以 这个计算比例的zoom不能用当前取的, 要用开始缩放策略时记录的
         */
        float zoom = tm->startscale;
        float zoom_rate = new_spacing / tm->startspacing;
        float zoom_value = zoom * zoom_rate;

        /**
         * 将当前缩放比乘以手势缩放比例后
         * 做区间限制
         */
        float max = gles_map_get_scale_max(tm->map);
        float min = gles_map_get_scale_min(tm->map);
        if (zoom_value > max) {
            zoom_value = max;
        } else if (zoom_value < min) {
            zoom_value = min;
        }

        /**
         * 如果区间限制后的缩放比和当前缩放比相同
         * 也忽略
         */
        if(utils_math_float_equals(zoom_value, zoom))
            break;

        /**
         * 确实需要缩放, 计算缩放比
         */
        zoom_rate = zoom_value / zoom;

        /**xoozi
        _matrix.set(_savedMatrix);
        _matrix.postScale(zoom_rate, zoom_rate, _mid.x, -_mid.y);
        */
        _post_scale(tm, zoom_rate);

        retval = 1;
    }while(0);

    tm->strategy = &_none_strategy;
    return retval;
}

static 
int 
_rotate_touch_move(T tm, float x0, float y0, float x1, float y1)
{
    float new_spacing = _spacing(x0, y0, x1, y1);
    if(new_spacing > 10){
        float delta_degree = _rotate_degree(x0, y0, x1, y1) - tm->first_degree;
        float scale = new_spacing / tm->startspacing;
        /**
         * 由于缩放时, 其实每步重置了矩阵, 然后post算出来的比例,
         * 所以 这个计算比例的zoom不能用当前取的, 要用开始缩放策略时记录的
         */
        float zoom = tm->startscale;
        float zoom_rate = 1;
        int   need_zoom = 1;

        /**
         * 将当前缩放比乘以手势缩放比例后
         * 做区间限制
         */
        float zoom_value = zoom * scale;
        float max = gles_map_get_scale_max(tm->map);
        float min = gles_map_get_scale_min(tm->map);
        if (zoom_value > max) {
            zoom_value = max;
        } else if (zoom_value < min) {
            zoom_value = min;
        }
        /**
         * 如果区间限制后的缩放比和当前缩放比相同
         * 也忽略
         */
        if (utils_math_float_equals(zoom_value, zoom)) {
            need_zoom = 0;
        } else {
            /**
             * 确实需要缩放, 计算缩放比
             */
            zoom_rate = zoom_value / zoom;
        }


        /**
         * xoozi
        _matrix.set(_savedMatrix);
        if(need_zoom)
            _matrix.postScale(zoom_rate, zoom_rate, _mid.x, -_mid.y);
        _matrix.postRotate(delta_degree, _mid.x, -_mid.y);
        */

        if(need_zoom){
            _post_scale_rotation(tm, zoom_rate, delta_degree);
        }else{
            _post_rotation(tm, delta_degree);
        }
        return 1;
    }
    return 0;
}
static 
int 
_rotate_multi_up(T tm, float x0, float y0, float x1, float y1)
{

    int retval = 0;
    float new_spacing = _spacing(x0, y0, x1, y1);
    if(new_spacing > 10){
        float delta_degree = _rotate_degree(x0, y0, x1, y1) - tm->first_degree;
        float scale = new_spacing / tm->startspacing;
        /**
         * 由于缩放时, 其实每步重置了矩阵, 然后post算出来的比例,
         * 所以 这个计算比例的zoom不能用当前取的, 要用开始缩放策略时记录的
         */
        float zoom = tm->startscale;
        float zoom_rate = 1;
        int   need_zoom = 1;

        /**
         * 将当前缩放比乘以手势缩放比例后
         * 做区间限制
         */
        float zoom_value = zoom * scale;
        float max = gles_map_get_scale_max(tm->map);
        float min = gles_map_get_scale_min(tm->map);
        if (zoom_value > max) {
            zoom_value = max;
        } else if (zoom_value < min) {
            zoom_value = min;
        }
        /**
         * 如果区间限制后的缩放比和当前缩放比相同
         * 也忽略
         */
        if (utils_math_float_equals(zoom_value, zoom)) {
            need_zoom = 0;
        } else {
            /**
             * 确实需要缩放, 计算缩放比
             */
            zoom_rate = zoom_value / zoom;
        }


        /**
         * xoozi
        _matrix.set(_savedMatrix);
        if(need_zoom)
            _matrix.postScale(zoom_rate, zoom_rate, _mid.x, -_mid.y);
        _matrix.postRotate(delta_degree, _mid.x, -_mid.y);
        */
        if(need_zoom){
            _post_scale_rotation(tm, zoom_rate, delta_degree);
        }else{
            _post_rotation(tm, delta_degree);
        }

        retval = 1;
    }
    tm->strategy = &_none_strategy;
    return retval;
}
