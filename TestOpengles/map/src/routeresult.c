#include<math.h>
#include<stdlib.h>
#include<float.h>
#include"assert.h"
#include"arena.h"
#include"mem.h"
#include"seq.h"
#include"vec2.h"
#include"routeresult.h"

#define T routeresult_t

struct T {
    float   azimuth;
    arena_t arena;
    seq_t   seglist;
};


static void         _load_seg(T rr, const float *pts, int cnt);
static route_seg_t  _make_seg(T rr, float sx, float sy, float ex, float ey);
static float        _make_azimuth(float sx, float sy, float ex, float ey);
static float        _make_serial_distance(T rr, int index, float tail);
static int          _make_cur_suggestion(float dev_azimuth, float seg_azimuth);
static int          _make_next_suggestion(T rr, int segIndex);
static double       _radian_2_degree(double radian);


T                    
routeresult_new
(float mapazimuth)
{
    T rr;
    arena_t arena;

    arena       = arena_new();
    rr          = ALLOC(sizeof(*rr));
    rr->arena   = arena;
    rr->azimuth = mapazimuth;
    rr->seglist = seq_new(64);

    return rr;
}

void                 
routeresult_free
(T *rr)
{
    assert(rr);
    assert(*rr);
    seq_free(&((*rr)->seglist));
    arena_free((*rr)->arena);
    FREE(*rr);
}

/**
 * 设置路径数据
 */
void                 
routeresult_set_data
(T rr, const float *pts, int cnt)
{
    assert(rr);

    if(seq_length(rr->seglist) > 0){
        routeresult_clean_data(rr);
    }
    _load_seg(rr, pts, cnt);
}

/**
 * 清除路径数据
 */
void                 
routeresult_clean_data(T rr)
{
    assert(rr);
    
    while(seq_length(rr->seglist) > 0){
        seq_remove_low(rr->seglist);
    }
    arena_free(rr->arena);
}

int                  
routeresult_seg_count
(T rr)
{
    assert(rr);
    return  seq_length(rr->seglist);
}


const route_seg_t    
routeresult_get_seg
(T rr, int index)
{
    assert(rr);
    return seq_get(rr->seglist, index);
}


struct nav_status   
routeresult_get_status
(T rr, float x, float y, float azimuth)
{
    struct nav_status nv;
    int len, i, count, j, hasprev;
    double minidistsq, distsq, dx, dy;
    struct vec2 point;
    struct vec2 start;
    struct vec2 end;
    struct vec2 normp;
    route_seg_t   rs;

    point = vec2_new(x, y);
    minidistsq = DBL_MAX;

    len = seq_length(rr->seglist);

    if(len > 0){
        /**
         * 遍历seg, 做投射
         */
        for(i = 0; i < len; i++){

            rs = seq_get(rr->seglist, i);

            start.x = rs->startx;
            start.y = rs->starty;
            end.x   = rs->endx;
            end.y   = rs->endy;

            normp = vec2_normal_projection(point, start, end);
            distsq = vec2_dist_sq(point, normp);

            /**
             * 保存距离最近的投射点
             * 这里因为同样的距离我们更趋向与建议用户往前走
             * 所以用<=.
             * 使得都投影到端点时, 趋向于后面的seg
             */
            if(distsq <= minidistsq){
                nv.projx = normp.x;
                nv.projy = normp.y;
                nv.segindex = i;
                minidistsq = distsq;
            }
        }
        nv.validate     = 1;
        nv.errordist    = sqrt(minidistsq);

        rs = seq_get(rr->seglist, nv.segindex);
        dx = nv.projx - rs->endx;
        dy = nv.projy - rs->endy;

        nv.azimuth      = rs->azimuth;
        nv.pathdist     = sqrt(dx*dx + dy*dy);
        nv.serialdist   = _make_serial_distance(rr, nv.segindex, nv.pathdist);
        nv.sug          = _make_cur_suggestion(azimuth, rs->azimuth);
        nv.nextsug      = _make_next_suggestion(rr, nv.segindex);
    }else{
        nv.validate = 0;
    }


    return nv;
}

static 
void _load_seg(T rr, const float *pts, int cnt)
{
    assert(cnt >= 2);

    int i, len, first;
    float cx, cy, sx, sy;

    sx = sy = 0;
    first = 1;
    for(i = 0; i < cnt; i++){
        cx = pts[2*i+0];
        cy = pts[2*i+1];
        /**
         * cur设为段末, 计算段方位角, 添到段表
         */
        if(first) {
            first = 0;
        }else{
            route_seg_t rs = _make_seg(rr, sx, sy, cx, cy);
            seq_add_high(rr->seglist, rs);
        }

        /**
         * 继续, 起新段
         */
        sx  = cx;
        sy  = cy;
    }
}

static 
route_seg_t  
_make_seg(T rr, float sx, float sy, float ex, float ey)
{
    route_seg_t rs;

    rs = ARENA_CALLOC(rr->arena, 1, sizeof(*rs));
    rs->startx = sx;
    rs->starty = sy;
    rs->endx = ex;
    rs->endy = ey;
    rs->azimuth = _make_azimuth(sx, sy, ex, ey);

    return rs;
}

static 
float        
_make_azimuth(float sx, float sy, float ex, float ey)
{
    struct vec2 true_north  = {0, 1};
    struct vec2 true_org    = {0, 0};
    struct vec2 end_vec     = {ex, ey};
    struct vec2 start_vec   = {sx, sy};

    true_north = vec2_normalize(vec2_sub(true_north, true_org));

    struct vec2 seg_vec = vec2_normalize(vec2_sub(end_vec, start_vec));
    double cos_theta = vec2_dot_prod(seg_vec, true_north);
    double radian_theta = acos(cos_theta);
    if(seg_vec.x < 0){
        radian_theta = (2 * M_PI - radian_theta);
    }
    double degree_theta = _radian_2_degree(radian_theta);
    return (float)degree_theta;
}


static 
float        
_make_serial_distance(T rr, int index, float tail)
{
    float distance = tail;
    int len;
    float delta, dx, dy;
    route_seg_t   curs, nexts;

    curs = seq_get(rr->seglist, index);

    len = seq_length(rr->seglist);
    while(index < (len-1)){
        nexts = seq_get(rr->seglist, ++index);
        delta = nexts->azimuth - curs->azimuth;
        if(delta < 0.0){
            delta += 360.0;
        }
        if(delta <= 10.0 || delta >= 350.0) {
            dx = nexts->endx - nexts->startx;
            dy = nexts->endy - nexts->starty;
            distance += sqrt(dx*dx + dy*dy);
        }else {
            break;
        }
    }
    return distance;
}



static 
int          
_make_cur_suggestion(float dev_azimuth, float seg_azimuth)
{
    float delta_seg_dev;
    int orientation;

    delta_seg_dev = seg_azimuth - dev_azimuth;

    /**
     * 滤除负角度
     */
    if(delta_seg_dev < 0){
        delta_seg_dev = 360 + delta_seg_dev;
    }
    if((delta_seg_dev > 337.5f  && delta_seg_dev <= 0) || 
       (delta_seg_dev > 0      && delta_seg_dev <= 22.5f)){
        orientation = ROUTE_SUG_FRONT;
    }else if(delta_seg_dev > 22.5f && delta_seg_dev <= 67.5f){
        orientation = ROUTE_SUG_FRONT;
    }else if(delta_seg_dev > 67.5f && delta_seg_dev <= 112.5f){
        orientation = ROUTE_SUG_RIGHT;
    }else if(delta_seg_dev > 112.5f && delta_seg_dev <= 157.5f){
        orientation = ROUTE_SUG_BACKWARD;
    }else if(delta_seg_dev > 157.5f && delta_seg_dev <= 202.5f){
        orientation = ROUTE_SUG_BACKWARD;
    }else if(delta_seg_dev > 202.5f && delta_seg_dev <= 247.5f){
        orientation = ROUTE_SUG_BACKWARD;
    }else if(delta_seg_dev > 247.5f && delta_seg_dev <= 292.5f){
        orientation = ROUTE_SUG_LEFT;
    }else{
        orientation = ROUTE_SUG_FRONT;
    }
    return orientation;
}


static 
int          
_make_next_suggestion(T rr, int segindex)
{
    int len, ns;
    float delta;
    route_seg_t   curs, nexts;


    len = seq_length(rr->seglist);
    if(segindex >= (len-1)){
        ns = ROUTE_NEXT_SUG_ARRIVE;
    }else{
        curs = seq_get(rr->seglist, segindex);
        nexts= seq_get(rr->seglist, segindex+1);
        delta = nexts->azimuth - curs->azimuth;
        if(delta < 0.0){
            delta += 360.0;
        }

        if(delta <= 10.0 || delta >= 350.0) {
            ns = ROUTE_NEXT_SUG_FRONT;
        }else if(delta < 180.0){
            ns = ROUTE_NEXT_SUG_RIGHT;
        }else{
            ns = ROUTE_NEXT_SUG_LEFT;
        }
    }
    return ns;
}

static 
double
_radian_2_degree(double radian)
{
    return 360.0 * radian / (2 * M_PI);
}
