
#include<math.h>
#include"assert.h"
#include"mem.h"
#include"gles_map.h"
#include"gles_scene_touchmgr.h"
#include "gles_matrix.h"
#include <string.h>

#ifdef ANDROID_IDRN
#include"error_functions.h"
#endif

#define T gles_scene_touchmgr_t

struct scene_touch_strategy{
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
    gles_scene_render_t             gsr;
    int                             zoomend;
    struct scene_touch_strategy    *strategy;
    float                           startx;
    float                           starty;
    float                           startspacing;
    GLfloat                         worldmatbuf[16];
    GLfloat                         tmpmatbuf[16];
    GLfloat                         savedmat[16];
};


static int _none_touch_down(T tm, float x, float y);
static int _none_touch_move(T tm, float x0, float y0, float x1, float y1);
static int _none_multi_down(T tm, float x0, float y0, float x1, float y1);

static int _azimuth_touch_move(T tm, float x0, float y0, float x1, float y1);
static int _azimuth_touch_up(T tm, float x, float y);
static int _azimuth_multi_down(T tm, float x0, float y0, float x1, float y1);

static int _pitch_touch_move(T tm, float x0, float y0, float x1, float y1);
static int _pitch_touch_up(T tm, float x, float y);
static int _pitch_multi_down(T tm, float x0, float y0, float x1, float y1);

static int _zoom_touch_move(T tm, float x0, float y0, float x1, float y1);
static int _zoom_multi_up(T tm, float x0, float y0, float x1, float y1);
static int _zoom_touch_up(T tm, float x, float y);
static void _save_matrix(T tm);
static void _matrix_rotate3d(float *rm, float a, float x, float y, float z);
static void _matrix_translate_4(GLfloat *m, float x, float y, float z);
static float _spacing(float x0, float y0, float x1, float y1);
static float _translate_distance(T tm);


static struct scene_touch_strategy    _none_strategy = {
    .name           = "none",
    .on_touch_down  = _none_touch_down,
    .on_touch_up    = NULL,
    .on_touch_move  = _none_touch_move,
    .on_multi_down  = _none_multi_down,
    .on_multi_up    = NULL,
};
static struct scene_touch_strategy    _azimuth_strategy = {
    .name           = "azimuth",
    .on_touch_down  = NULL,
    .on_touch_up    = _azimuth_touch_up,
    .on_touch_move  = _azimuth_touch_move,
    .on_multi_down  = _azimuth_multi_down,
    .on_multi_up    = NULL,
};
static struct scene_touch_strategy    _pitch_strategy = {
    .name           = "pitch",
    .on_touch_down  = NULL,
    .on_touch_up    = _pitch_touch_up,
    .on_touch_move  = _pitch_touch_move,
    .on_multi_down  = _pitch_multi_down,
    .on_multi_up    = NULL,
};
static struct scene_touch_strategy    _zoom_strategy = {
    .name           = "zoom",
    .on_touch_down  = NULL,
    .on_touch_up    = _zoom_touch_up,
    .on_touch_move  = _zoom_touch_move,
    .on_multi_down  = NULL,
    .on_multi_up    = _zoom_multi_up,
};

T    
gles_scene_touchmgr_new
(gles_scene_render_t gsr)
{
    T tm;
    tm = ALLOC(sizeof(*tm));
    tm->gsr = gsr;
    tm->strategy = &_none_strategy;

    return tm;
}

void 
gles_scene_touchmgr_free
(T *tm)
{

    assert(tm);
    assert(*tm);

    FREE(*tm);
}

int  
gles_scene_touchmgr_ontouch
(T tm, int action, float x0, float y0, float x1, float y1)
{
    int retval = 0;
    
    assert(tm);


    if(tm->strategy){
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
int 
_none_touch_down(T tm, float x, float y)
{
    tm->startx = x;
    tm->starty = y;
#ifdef ANDROID_IDRN
    log_i("------none down start:(%f, %f)", x, y);
#endif
    return 1;
}
static 
int 
_none_touch_move(T tm, float x0, float y0, float x1, float y1)
{
    float dx, dy;
    dx = x0 - tm->startx;
    dy = y0 - tm->starty;


    if((dx*dx + dy*dy) > 500){
        if(abs((int)dx) > abs((int)dy)){
            tm->strategy = &_azimuth_strategy;
        }else{
            tm->strategy = &_pitch_strategy;
        }
        _save_matrix(tm);
    }
    return 1;
}
static 
int 
_none_multi_down(T tm, float x0, float y0, float x1, float y1)
{
    tm->strategy = &_zoom_strategy;
    tm->zoomend = 0;
    tm->startspacing = _spacing(x0, y0, x1, y1);
    _save_matrix(tm);
    return 1;
}


static 
int 
_azimuth_touch_move(T tm, float x0, float y0, float x1, float y1)
{
    float dx;

    dx = x0 - tm->startx;

    memcpy((void*)tm->worldmatbuf, (const void*)tm->savedmat, sizeof(GLfloat)*16);
    _matrix_rotate3d(tm->tmpmatbuf, dx/20.0f, 0, 1, 0);
    gles_matrix_mult_matrix_4(tm->worldmatbuf, tm->tmpmatbuf);
    gles_scene_render_set_world_matrix(tm->gsr, tm->worldmatbuf);
    return 1;
}
static 
int 
_azimuth_touch_up(T tm, float x, float y)
{
    tm->strategy = &_none_strategy;
    return 1;
}
static 
int 
_azimuth_multi_down(T tm, float x0, float y0, float x1, float y1)
{
    tm->strategy = &_zoom_strategy;
    tm->zoomend = 0;
    tm->startspacing = _spacing(x0, y0, x1, y1);
    _save_matrix(tm);
    return 1;
}



static 
int 
_pitch_touch_move(T tm, float x0, float y0, float x1, float y1)
{
    float dy;

    dy = y0 - tm->starty;

    float inmat[16];
    float vec[4];
    vec[0] = 1.0f;
    vec[1] = 0.0f;
    vec[2] = 0.0f;
    vec[3] = 0.0f;
    gles_matrix_invert_4(tm->savedmat, inmat);
    gles_matrix_mult_vec_4(inmat, vec, 0);

    memcpy(tm->worldmatbuf, tm->savedmat, sizeof(GLfloat)*16);
    _matrix_rotate3d(tm->tmpmatbuf, dy/20.0f, vec[0], vec[1], vec[2]);
    gles_matrix_mult_matrix_4(tm->worldmatbuf, tm->tmpmatbuf);
    gles_scene_render_set_world_matrix(tm->gsr, tm->worldmatbuf);
    return 1;
}
static 
int 
_pitch_touch_up(T tm, float x, float y)
{
    tm->strategy = &_none_strategy;
    return 1;
}
static 
int 
_pitch_multi_down(T tm, float x0, float y0, float x1, float y1)
{
    tm->strategy = &_zoom_strategy;
    tm->zoomend = 0;
    tm->startspacing = _spacing(x0, y0, x1, y1);
    _save_matrix(tm);
    return 1;
}


static 
int 
_zoom_touch_move(T tm, float x0, float y0, float x1, float y1)
{


    float new_spacing = _spacing(x0, y0, x1, y1);

    if(10 > new_spacing)
        return 1;

    float rate = new_spacing/tm->startspacing;
    float step;
    float dist = _translate_distance(tm);
    dist /= 10.0f;
    if(rate > 1.0){
        step = -dist*rate;
    }else if(rate > 0){
        step = dist / rate;
    }

    float inmat[16];
    float vec[4];
    vec[0] = 0.0f;
    vec[1] = 0.0f;
    vec[2] = step;
    vec[3] = 0.0f;
    gles_matrix_invert_4(tm->savedmat, inmat);
    gles_matrix_mult_vec_4(inmat, vec, 0);

    memcpy(tm->worldmatbuf, tm->savedmat, sizeof(GLfloat)*16);
    _matrix_translate_4(tm->tmpmatbuf, vec[0], vec[1], vec[2]);
    gles_matrix_mult_matrix_4(tm->worldmatbuf, tm->tmpmatbuf);
    gles_scene_render_set_world_matrix(tm->gsr, tm->worldmatbuf);
    return 1;
}
static 
int 
_zoom_multi_up(T tm, float x0, float y0, float x1, float y1)
{
    /*
    float new_spacing = _spacing(x0, y0, x1, y1);
    if(10 > new_spacing)
        return 1;

    float rate = new_spacing/tm->startspacing;
    float step;
    float dist = _translate_distance(tm);
    dist /= 10.0f;
    if(rate > 1.0){
        step = -dist*rate;
    }else if(rate > 0){
        step = dist / rate;
    }

    float inmat[16];
    float vec[4];
    vec[0] = 0.0f;
    vec[1] = 0.0f;
    vec[2] = step;
    vec[3] = 0.0f;
    gles_matrix_invert_4(tm->savedmat, inmat);
    gles_matrix_mult_vec_4(inmat, vec, 0);

    memcpy(tm->worldmatbuf, tm->savedmat, sizeof(GLfloat)*16);
    _matrix_translate_4(tm->tmpmatbuf, vec[0], vec[1], vec[2]);
    gles_matrix_mult_matrix_4(tm->worldmatbuf, tm->tmpmatbuf);
    gles_scene_render_set_world_matrix(tm->gsr, tm->worldmatbuf);

    //tm->zoomend = 1;*/
    return 1;
}

static 
int 
_zoom_touch_up(T tm, float x, float y)
{
    tm->strategy = &_none_strategy;
    return 1;
}

static 
void     
_save_matrix(T tm)
{
    memcpy(tm->savedmat, 
            gles_scene_render_get_world_matrix(tm->gsr),
            sizeof(GLfloat)*16);
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
float 
_translate_distance(T tm)
{
    float x, y, z;
    x = tm->savedmat[12];
    y = tm->savedmat[13];
    z = tm->savedmat[14];

    return sqrt(x*x + y*y + z*z);
}

static
void
_matrix_rotate3d
(float *rm, float a, float x, float y, float z)
{
    rm[3] = 0;
    rm[7] = 0;
    rm[11]= 0;
    rm[12]= 0;
    rm[13]= 0;
    rm[14]= 0;
    rm[15]= 1;
    a *= (float) (M_PI / 180.0f);
    float s = sinf(a);
    float c = cosf(a);
    if(1.0f == x && 0.0f == y && 0.0f == z) {
        rm[5] = c;   rm[10]= c;
        rm[6] = s;   rm[9] = -s;
        rm[1] = 0;   rm[2] = 0;
        rm[4] = 0;   rm[8] = 0;
        rm[0] = 1;
    }else if (0.0f == x && 1.0f == y && 0.0f == z) {
        rm[0] = c;   rm[10]= c;
        rm[8] = s;   rm[2] = -s;
        rm[1] = 0;   rm[4] = 0;
        rm[6] = 0;   rm[9] = 0;
        rm[5] = 1;
    }else if (0.0f == x && 0.0f == y && 1.0f == z) {
        rm[0] = c;   rm[5] = c;
        rm[1] = s;   rm[4] = -s;
        rm[2] = 0;   rm[6] = 0;
        rm[8] = 0;   rm[9] = 0;
        rm[10]= 1;
    }else{
        float len = sqrtf(x*x + y*y + z*z);
        if (1.0f != len) {
            float recipLen = 1.0f / len;
            x *= recipLen;
            y *= recipLen;
            z *= recipLen;
        }
        float nc = 1.0f - c;
        float xy = x * y;
        float yz = y * z;
        float zx = z * x;
        float xs = x * s;
        float ys = y * s;
        float zs = z * s;
        rm[ 0] = x*x*nc +  c;
        rm[ 4] =  xy*nc - zs;
        rm[ 8] =  zx*nc + ys;
        rm[ 1] =  xy*nc + zs;
        rm[ 5] = y*y*nc +  c;
        rm[ 9] =  yz*nc - xs;
        rm[ 2] =  zx*nc - ys;
        rm[ 6] =  yz*nc + xs;
        rm[10] = z*z*nc +  c;
    }
}

static void 
_matrix_translate_4
(GLfloat *m, float x, float y, float z)
{
    m[ 0] = 1.0f;
    m[ 1] = 0.0f;
    m[ 2] = 0.0f;
    m[ 3] = 0.0f;

    m[ 4] = 0.0f;
    m[ 5] = 1.0f;
    m[ 6] = 0.0f;
    m[ 7] = 0.0f;

    m[ 8] = 0.0f;
    m[ 9] = 0.0f;
    m[10] = 1.0f;
    m[11] = 0.0f;

    m[12] = x;
    m[13] = y;
    m[14] = z;
    m[15] = 1.0f;
}
