#include<math.h>
#include <stdlib.h>
#include"assert.h"
#include"mem.h"
#include"seq.h"
#include"gles_map.h"
#include"gles_map_animation.h"
#include"gles_matrix.h"
#include "utils.h"
#include <string.h>

#define T gles_map_animation_t

#define ANIM_TRANSLATE          0
#define ANIM_ROTATE             1
#define ANIM_ZOOM               2
#define ANIM_ROTATE_TRANSLATE   3

typedef struct anim_t *anim_t;

struct T{
    gles_map_t  map;
    int         anim_flag;
    seq_t       anim_seq;
    anim_t      anim_cur;
    GLfloat     savedmat[16];
};
struct anim_rotate{
    float   x;
    float   y;
    float   rotate;
};
struct anim_zoom{
    float   x;
    float   y;
    float   step_rate;
};
struct anim_translate{
    float   tx;
    float   ty;
};
struct anim_rotate_translate{
    float   sx;
    float   sy;
    float   ex;
    float   ey;
    float   azimuthstart;
    float   azimuthend;
    float   scale;
};
struct anim_t{
    int         type;
    int         step;
    int         frame;
    union{
        struct anim_rotate              r;
        struct anim_zoom                z;
        struct anim_translate           t;
        struct anim_rotate_translate    rt;
    }content;
};

static void _anim_start(T gma);
static void _anim_step(T gma);
static void _anim_finish(T gma);


T    
gles_map_animation_new
(gles_map_t map)
{
    T gma;

    NEW0(gma);
    gma->map        = map;
    gma->anim_seq   = seq_new(64);

    return gma;
}


void 
gles_map_animation_free
(T *gma)
{
    assert(gma);
    assert(*gma);
    seq_free(&((*gma)->anim_seq));
    FREE(*gma);
}

void 
gles_map_animation_update
(T gma)
{
    assert(gma);
    if(gma->anim_flag){
        _anim_step(gma);

        if(gma->anim_cur->step >= gma->anim_cur->frame){
            _anim_finish(gma);
        }
    }else{
        if(seq_length(gma->anim_seq)){
            gma->anim_cur   = seq_remove_high(gma->anim_seq);
            _anim_start(gma);
        }
    }
}

void 
gles_map_animation_clean
(T gma)
{
    assert(gma);
    while(seq_length(gma->anim_seq)>0){
        gma->anim_cur   = seq_remove_high(gma->anim_seq);
        _anim_finish(gma);
    }
}

int
gles_map_animation_flag
(T gma)
{
    assert(gma);
    return gma->anim_flag;
}

void 
gles_map_animation_rotate
(T gma, float rotate, float x, float y)
{
    anim_t  a;
    NEW0(a);

    a->type             = ANIM_ROTATE;
    a->step             = 0;
    a->frame            = 24;
    a->content.r.x      = x;
    a->content.r.y      = y;
    a->content.r.rotate = rotate;
    
    seq_add_low(gma->anim_seq, a);
}


void 
gles_map_animation_rotate_reset
(T gma)
{
    float rotate;

    rotate = gles_map_get_rotate_degree(gma->map);
    if(rotate > 180){
        rotate -= 360;
    }
    gles_map_animation_rotate(gma, 
                rotate, 
                gles_map_get_view_width(gma->map)/2.0f,
                gles_map_get_view_height(gma->map)/2.0f);
}

void 
gles_man_animation_zoom_by
(T gma, float rate)
{

    float   scale_cur, scale_target, max, min, true_rate, step;
    anim_t  a;

    do{
        if(utils_math_float_equals(1.0f, rate))
            break;

        NEW0(a);
        max = gles_map_get_scale_max(gma->map);
        min = gles_map_get_scale_min(gma->map);
        scale_cur = gles_map_get_scale_value(gma->map);
        scale_target = rate * scale_cur;
        if(scale_target > max){
            scale_target = max;
        }
        if(scale_target < min){
            scale_target = min;
        }
        true_rate = scale_target / scale_cur;

        if(utils_math_float_equals(1.0f, true_rate))
            break;

        step = powf(true_rate, 1.0f/24.0f);

        a->type             = ANIM_ZOOM;
        a->step             = 0;
        a->frame            = 24;
        a->content.z.x      = gles_map_get_view_width(gma->map)/2.0f;
        a->content.z.y      = gles_map_get_view_height(gma->map)/2.0f;
        a->content.z.step_rate = step;

        seq_add_low(gma->anim_seq, a);
    }while(0);
}

void 
gles_map_animation_translate
(T gma, float tx, float ty)
{
    anim_t  a;
    NEW0(a);

    a->type             = ANIM_TRANSLATE;
    a->step             = 0;
    a->frame            = 24;
    a->content.t.tx      = tx;
    a->content.t.ty      = ty;
    
    seq_add_low(gma->anim_seq, a);
}

void 
gles_map_animation_rotate_translate
(T gma, float rotate, float sx, float sy, float ex, float ey)
{
    anim_t  a;

    float azimuth_start, azimuth_end, azimuth_end_adj, azimuth_delta;

    NEW0(a);
    
    azimuth_start = -gles_map_get_rotate_degree(gma->map);
    azimuth_end   = rotate;

    /**
     * 我的 gles_map_get_rotate_degree
     * 方法有个问题, 会把当地图角度为0.1度时
     * 会返回-359.9度
     * 虽然绝对值没问题, 从-0.1到0.1时就会发生插值方向的跳变
     * 暂时想不到太好的方法解决
     * 打上下面这个补丁可破
     */
    if(azimuth_start < -180){
        azimuth_start += 360;
    }

    /**
     * 保证两个方向都走劣弧, 更改插值方向
     */
    azimuth_delta = azimuth_end - azimuth_start;
    if(azimuth_delta > 180){
        azimuth_end_adj = azimuth_end - 360;
    }else if(azimuth_delta < -180){
        azimuth_end_adj = azimuth_end + 360;
    }else{
        azimuth_end_adj = azimuth_end;
    }


    /**
     * 旋转平移融合动画比一般的要复杂
     * 由于旋转的影响
     * 每一步的平移量需要重新计算
     * 所以不能像平移动画那样事先算好整段位移然后按帧叠加位移
     * 而要用参数方程插值每步实时计算
     */
    a->type             = ANIM_ROTATE_TRANSLATE;
    a->step             = 0;
    a->frame            = 12;
    a->content.rt.sx    = sx;
    a->content.rt.sy    = sy;
    a->content.rt.ex    = ex;
    a->content.rt.ey    = ey;
    a->content.rt.azimuthstart = azimuth_start;
    a->content.rt.azimuthend = azimuth_end_adj;
    a->content.rt.scale = gles_map_get_scale_value(gma->map);

    seq_add_low(gma->anim_seq, a);
}


static 
void 
_anim_step(T gma)
{
    switch(gma->anim_cur->type){
        case ANIM_TRANSLATE:
        {
            GLfloat tmpmat[16];
            float deltax, deltay;

            gma->anim_cur->step += 1;
            deltax = gma->anim_cur->step * (gma->anim_cur->content.t.tx / gma->anim_cur->frame);
            deltay = gma->anim_cur->step * (gma->anim_cur->content.t.ty / gma->anim_cur->frame);
            memcpy((void*)tmpmat, (const void*)gma->savedmat, sizeof(GLfloat)*16);
            gles_matrix_post_translate(tmpmat, deltax, deltay);
            gles_map_matrix(gma->map, tmpmat);
        }
        break;
        case ANIM_ROTATE:
        {
            GLfloat tmpmat[16];
            float rotate;

            gma->anim_cur->step += 1;
            rotate = gma->anim_cur->step * (gma->anim_cur->content.r.rotate / gma->anim_cur->frame);
            memcpy(tmpmat, gma->savedmat, sizeof(GLfloat)*16);
            gles_matrix_post_rotation(tmpmat, rotate, 
                            gma->anim_cur->content.r.x,
                            gma->anim_cur->content.r.y);
            gles_map_matrix(gma->map, tmpmat);
        }
        break;

        case ANIM_ZOOM:
        {
            GLfloat tmpmat[16];

            gma->anim_cur->step += 1;
            memcpy(tmpmat, gma->savedmat, sizeof(GLfloat)*16);
            gles_matrix_post_scale(tmpmat,
                            gma->anim_cur->content.z.step_rate, 
                            gma->anim_cur->content.z.x,
                            gma->anim_cur->content.z.y);
            memcpy(gma->savedmat, tmpmat, sizeof(GLfloat)*16);
            gles_map_matrix(gma->map, tmpmat);
        }
        break;

        case ANIM_ROTATE_TRANSLATE:
        {
            /**
             * 为了实现旋转平移融合动画, 不能用前面的增量动画的手段来计算没一帧的线性变换
             * 而是分两个主要成分每帧进行线性插值
             * 1 位置, 由上一位置和目标位置插值得到
             * 2 角度, 由上一角度和下一角度插值得到
             * 再加上动画过程中不变的缩放值
             * 每帧都由单位矩阵开始重新计算整个地图的线性变换矩阵
             * 这样的做法动画比较流程, 易于理解
             */
            GLfloat tmpmat[16];
            anim_t      a = gma->anim_cur;

            a->step += 1;
            /**
             * 这里全程采用双精度浮点数来插值, 可以大大减少误差 
             * 这里些计算如果用float, 会使放大10倍后, 计算出的中心偏离10像素左右
             */
            double t = ((double)a->step)/((double)a->frame);
            double w= gles_map_get_view_width(gma->map);
            double h= gles_map_get_view_height(gma->map);

            struct vec2 start = {a->content.rt.sx, a->content.rt.sy};
            struct vec2 end   = {a->content.rt.ex, a->content.rt.ey};
            struct vec2 lerpv = vec2_mul(vec2_lerp_vec(start, end, t), a->content.rt.scale);
            double lerpazimuth = vec2_lerp_value(a->content.rt.azimuthstart, a->content.rt.azimuthend, t);
            double offsetx = w - lerpv.x - w / 2;
            double offsety = h - lerpv.y - h / 2;
            
            gles_matrix_identity_4(tmpmat, 0);
            /**
             * 浮点数倒来倒去的做乘法会放大误差, 这里不用矩阵乘法实现的post方法
             * 用直接设置的方法来生成第一阶段的缩放矩阵, 会提高精度到可以接受的地步, 放大10倍几乎不可见误差
             * 否则放大10倍会有5像素的误差
             */
            //gles_matrix_post_scale(tmpmat, a->content.rt.scale, 0, 0);
            tmpmat[0] = a->content.rt.scale;
            tmpmat[5] = a->content.rt.scale;
            /**
             * 实际上这个方法不是用矩阵乘法来实现的, 所以没有扩大浮点数误差
             */
            gles_matrix_post_translate(tmpmat, offsetx, offsety);

            /**
             * 唯一一个用矩阵乘法做的变换, 没办法,这个难以用直接计算得到, 好在采用前面的措施后, 误差已经肉眼不可见
             */
            gles_matrix_post_rotation(tmpmat, lerpazimuth, w/2, h/2);

            gles_map_matrix(gma->map, tmpmat);

            /**
             * 反馈位置
             * 试图这样实时更新投影位置而使得导航线的jump计算更实时
             * 而实际上这样会因为set location更新了一次位置而使得jump在非动画帧按位置计算
             * 而在动画帧实时计算, 所以, 要完全解决这个问题, 还要再细化一下
             *
            struct vec2 mp = gles_map_screen_to_map(gma->map, w/2, h/2);
            gles_map_set_location(gma->map, mp.x, mp.y);*/

        }
        break;

        default:
        {
            gma->anim_cur->step  = gma->anim_cur->frame;
        }
        break;
    }
}

static 
void 
_anim_start(T gma)
{
    memcpy(gma->savedmat, gles_map_get_mapmat(gma->map) , sizeof(GLfloat)*16);
    gma->anim_flag = 1;
}

static 
void 
_anim_finish(T gma)
{
    gma->anim_flag = 0;
    FREE(gma->anim_cur);
}
