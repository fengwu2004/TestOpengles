#include"stdio.h"
#include"math.h"
#include"assert.h"
#include"mem.h"
#include"seq.h"
#include"mover.h"
#include"gles_font.h"
#include"gles_loc_layer.h"
#include "gles_matrix.h"
#include <string.h>


#define T gles_loc_layer_t

struct T{
    gles_map_t      map;
    GLuint          tid;
    GLuint          width;
    GLuint          height;
    int             possetted;
    int             animateflag;
    float           posx;
    float           posy;
    float           lastposx;
    float           lastposy;
    float           lastprojx;
    float           lastprojy;
    struct vec2     target;
    float           azimuth;
    float           routescale;
    mover_t         mover;
    routeresult_t   rr;
    struct nav_status   nv;
    GLfloat         objmat[16];

    seq_t           tmp_route_pts;
    float           *tmp_route_pts_array;

    /**
     * 渲染用路径顶点/颜色/索引缓存
     */
    GLfloat         *route_verbuf;
    GLfloat         *route_colbuf;
    GLushort        *route_indbuf;
    int             route_shape_cnt;
};

/**
 * 生成路径顶点缓存所用的标识路径上插值点的位置及方向的结构
 */
struct shape_pos{
    float x;
    float y;
    float azimuth;
};


/**
 * 用以路径shape仿射变换的矩阵
 */
static GLfloat _shp_mat[16];

/**
 *路径shape模版
 */
static GLfloat _shape[] = {
                            0.0f,   3.0f,
                           -3.0f,   0.0f,
                           -3.0f,  -3.0f,
                            0.0f,   0.0f,
                            3.0f,  -3.0f,
                            3.0f,   0.0f};
/**
 * shape索引模板
 */
static GLushort  _shp_indics[] = {
                            0, 1, 3,
                            1, 2, 3,
                            0, 3, 5,
                            5, 3, 4};
                            
                            
/**
 * 定位标记纹理坐标
 */
static GLfloat _loc_tex_coords[] = {0.0f, 1.0f,
                                1.0f, 1.0f,
                                0.0f, 0.0f,
                                1.0f, 0.0f
                                };

/**
 * 定位标记纹理顶点缓存
 */
static GLfloat _loc_quad[]     ={
                            -20.0f, -21.0f,
                             20.0f, -21.0f,
                            -20.0f, 21.0f,
                             20.0f, 21.0f
                            };




/**
 * 遍历导航段, 每段内插值构造shape位置
 * 遍历shape位置列表
 * 每shapew位置含有坐标+方向
 * 将shape的顶点集模板平移+旋转到插值产生的位置
 *
 * 将每个变换后的shape顶点集拼成shape顶点缓存
 */
static void _make_shape_route_vbuf(T gll, float scale);


/**
 * P(t) = (1-t)*P1 + t*P2;
 * 段内插值, 构造shape位置
 */
static struct shape_pos *_make_shape_pos(float t, const route_seg_t rs, float azimuth);

static void _render_loc_route(T gll, gles_2d_c_program_t gcp, gles_font_t font, const GLfloat *mmat, int state);
static void _render_loc_marker_normal(T gll, gles_2d_t_program_t gtp, const GLfloat *mmat);
static void _render_loc_marker_follow(T gll, gles_2d_t_program_t gtp);
static void _render_loc_marker_navigate(T gll, gles_2d_t_program_t gtp);
static int  _calc_jump_shape(T gll);

void         
gles_loc_layer_dummy_add_route_point
(T gll, float x, float y)
{

    struct vec2 *p = ALLOC(sizeof(*p));
    p->x = x;
    p->y = y;
    seq_add_high(gll->tmp_route_pts, p);

    if(seq_length(gll->tmp_route_pts) > 1){
        _make_shape_route_vbuf(gll, gll->routescale);
    }
}

static 
struct shape_pos *
_make_shape_pos(float t, const route_seg_t rs, float azimuth)
{
    struct shape_pos *retval;
    float t_1;
    t_1 = 1.0f - t;
    retval = ALLOC(sizeof(*retval));
    retval->x = t_1 * rs->startx + t * rs->endx;
    retval->y = t_1 * rs->starty + t * rs->endy;
    retval->azimuth = azimuth;

    return retval;
}

static void _make_shape_route_vbuf(T gll, float scale)
{
    struct vec2 *p;
    seq_t   shape_list;
    int i, j, cnt, segcnt, shapecnt;
    float t, x, y, dx, dy, seglen, azimuth;
    float pos[4];
    cnt = seq_length(gll->tmp_route_pts);

    shape_list = seq_new(512);

    /**
     * 先给路径点集合填充数据,
     */
    if(gll->tmp_route_pts_array){
        RESIZE(gll->tmp_route_pts_array, sizeof(float)*2*cnt);
    }else{
        gll->tmp_route_pts_array = ALLOC(sizeof(float)*2*cnt);
    }
    for(i = 0; i < cnt; i++){

        p = seq_get(gll->tmp_route_pts, i);
        gll->tmp_route_pts_array[i*2+0] = p->x;
        gll->tmp_route_pts_array[i*2+1] = p->y;
    }
    routeresult_set_data(gll->rr, gll->tmp_route_pts_array, cnt);

    /**
     * 计算每段路径长度, 除以shape的长, 计算路径段内shape数
     * 用参数方程P(t)计算段内每个shape的坐标, 段内shape的方位角按段角度计算
     * 在段内P(1.0)的点, 是本段的终点也可能是下段的起点,
     * 如果本段不是最终段, 则P(1.0)的shape的方位角用本段和下段的平均值
     * 同理, 如果本段不是最初段, 则P(0.0)的shape不存入shapelist
     */
    segcnt = routeresult_seg_count(gll->rr);
    for(i = 0; i < segcnt; i++){
        const route_seg_t rs = routeresult_get_seg(gll->rr, i);
        dx = rs->endx - rs->startx;
        dy = rs->endy - rs->starty;
        seglen = sqrt(dx*dx + dy*dy);
        shapecnt = (int)ceil(seglen/(6.0*scale));
        j = (0 == i) ? 0 : 1;
        for(; j < shapecnt; j++){
            t = (float)j/(float)shapecnt;
            seq_add_high(shape_list, _make_shape_pos(t, rs, rs->azimuth));
        }
        if(i == (segcnt - 1)){
            azimuth = rs->azimuth;
        }else{
            const route_seg_t nextrs = routeresult_get_seg(gll->rr, i+1);
            float delta = nextrs->azimuth - rs->azimuth;
            azimuth = (rs->azimuth + nextrs->azimuth) / 2.0f;

            /**
             * 计算平均角度时, 需要保证方向, 要从劣弧那面平均, 否则会看起来有点怪
             */
            if(delta > 180 || delta < -180){
                azimuth+=180;
            }
        }
        seq_add_high(shape_list, _make_shape_pos(1.0f, rs, azimuth));
    }

    cnt = seq_length(shape_list);
    if(gll->route_verbuf){
        RESIZE(gll->route_verbuf, sizeof(GLfloat)*12*cnt);
    }else{
        gll->route_verbuf = ALLOC(sizeof(GLfloat)*12*cnt);
    }
    if(gll->route_colbuf){
        RESIZE(gll->route_colbuf, sizeof(GLfloat)*18*cnt);
    }else{
        gll->route_colbuf = ALLOC(sizeof(GLfloat)*18*cnt);
    }
    if(gll->route_indbuf){
        RESIZE(gll->route_indbuf, sizeof(GLushort)*12*cnt);
    }else{
        gll->route_indbuf = ALLOC(sizeof(GLushort)*12*cnt);
    }

    for(i = 0; i < cnt; i++){
        struct shape_pos *p = seq_get(shape_list, i);

        gles_matrix_rotate2d_4(_shp_mat, (float)-(p->azimuth));
        gles_matrix_postranslate_4(_shp_mat, p->x, p->y, 0);

        for(j = 0; j < 6; j++){
            pos[0] = _shape[j*2+0] * scale;
            pos[1] = _shape[j*2+1] * scale;
            pos[2] = 0;
            pos[3] = 1;

            gles_matrix_mult_vec_4(_shp_mat, pos, 0);

            gll->route_verbuf[i*12+j*2+0] = pos[0];
            gll->route_verbuf[i*12+j*2+1] = pos[1];

            //_route_colbuf[i*18+j*3+0] = (float)i/(float)cnt;
            gll->route_colbuf[i*18+j*3+0] = 0.0f;
            gll->route_colbuf[i*18+j*3+1] = 0.73f;
            gll->route_colbuf[i*18+j*3+2] = 1.0f;

        }

        for(j = 0; j < 4; j++){
            gll->route_indbuf[12*i+3*j+0] = _shp_indics[3*j+0] + i*6;
            gll->route_indbuf[12*i+3*j+1] = _shp_indics[3*j+1] + i*6;
            gll->route_indbuf[12*i+3*j+2] = _shp_indics[3*j+2] + i*6;
        }
        FREE(p);
    }

    seq_free(&shape_list);
    gll->route_shape_cnt = cnt;
}

void         
gles_loc_layer_set_route_data
(T gll, int cnt, float *pts)
{
    struct vec2 *p;
    int i;
    assert(gll);
    assert(cnt > 1);

    gles_loc_layer_clean_route_data(gll);

    for(i = 0; i < cnt; i++){
        p = ALLOC(sizeof(*p));
        p->x = pts[i*2 + 0];
        p->y = pts[i*2 + 1];
        seq_add_high(gll->tmp_route_pts, p);
    }

    _make_shape_route_vbuf(gll, gll->routescale);
}

void         
gles_loc_layer_clean_route_data
(T gll)
{
    if(NULL == gll->tmp_route_pts)
        return ;

    gll->route_shape_cnt = 0;
    routeresult_clean_data(gll->rr);
    while(seq_length(gll->tmp_route_pts) > 0){
        void *p  = seq_remove_high(gll->tmp_route_pts);
        FREE(p);
    }
}


T            
gles_loc_layer_new
(gles_map_t map, gles_texture_png_t marker, int width, int height)
{
    T gll;
    float hw, hh;

    assert(marker);

    gll = ALLOC(sizeof(*gll));
    gll->map        = map;
    gll->possetted  = 0;
    gll->animateflag= 0;
    
    gll->tid = gles_texture_png_tid(marker);
    gll->width = gles_texture_png_width(marker);
    gll->height= gles_texture_png_height(marker);

    
    gll->rr = routeresult_new(0);
    gll->mover = mover_new_def(0, 0);


    gles_matrix_identity_4(gll->objmat, 0);

    gles_matrix_identity_4(_shp_mat, 0);

    gll->azimuth = 0;
    gll->nv.validate = 0;
    gll->routescale = 2.0f;

    hw = gll->width/2.0f;
    hh = gll->height/2.0f;
    _loc_quad[0] = -hw;
    _loc_quad[1] = -hh;
    _loc_quad[2] = +hw;
    _loc_quad[3] = -hh;
    _loc_quad[4] = -hw;
    _loc_quad[5] = +hh;
    _loc_quad[6] = +hw;
    _loc_quad[7] = +hh;

    gll->tmp_route_pts_array = NULL;

    gll->route_verbuf = NULL;
    gll->route_colbuf = NULL;
    gll->route_indbuf = NULL;
    gll->route_shape_cnt = 0;

    /**
     * 测试用临时缓存路径节点
     */
    gll->tmp_route_pts = seq_new(64);
    return gll;
}

void         
gles_loc_layer_free
(T *gll)
{
    assert(gll);
    assert(*gll);

    while(seq_length((*gll)->tmp_route_pts) > 0){
        void *p  = seq_remove_high((*gll)->tmp_route_pts);
        FREE(p);
    }
    seq_free(&((*gll)->tmp_route_pts));

    FREE((*gll)->tmp_route_pts_array);
    FREE((*gll)->route_verbuf);
    FREE((*gll)->route_colbuf);
    FREE((*gll)->route_indbuf);

    FREE(*gll);
}

void         
gles_loc_layer_animate_to
(T gll, float x, float y)
{
    assert(gll);
    if(gll->possetted){
        mover_set_location(gll->mover, gll->posx, gll->posy);
        gll->animateflag = 1;
        gll->target.x = x;
        gll->target.y = y;
    }else{

        /**
         * 用于开着Trace切楼层后第一次用animate to定位
         * 如果个这里不通知一下要等动画跑完才会触发trace的居中
         */
        gles_map_set_location(gll->map, x, y);
        gles_loc_layer_set_pos(gll, x, y);
    }
}

int
gles_loc_layer_isseted_pos
(T gll)
{
    assert(gll);
    return gll->possetted;
}

void         
gles_loc_layer_set_pos
(T gll, float x, float y)
{
    assert(gll);
    if(1 == gll->possetted){
        gll->lastposx = gll->posx;
        gll->lastposy = gll->posy;

        /**
         * 如果已经设置过位置, 且上次导航段计算的结果有效
         * 暂存上次的投影位置
         */
        if(gll->nv.validate){
            gll->lastprojx = gll->nv.projx;
            gll->lastprojy = gll->nv.projy;
        }
    }else{
        gll->lastposx = x;
        gll->lastposy = y;
    }
    gll->posx = x;
    gll->posy = y;

    gll->nv = routeresult_get_status(gll->rr, x, y, gll->azimuth);
    if(gll->nv.validate){
        /**
         * 如果本次导航段计算结果有效, 且还未设置过位置
         * 则同步上次投影位置和本次投影位置
         */
        if(!gll->possetted){
            gll->lastprojx = gll->nv.projx;
            gll->lastprojy = gll->nv.projy;
        }
    }

    gll->possetted = 1;
}

struct vec2  
gles_loc_layer_get_pos
(T gll)
{
    assert(gll);
    struct vec2 retval = {gll->posx, gll->posy};
    return retval;
}

struct vec2  
gles_loc_layer_get_last_pos
(T gll)
{
    assert(gll);
    struct vec2 retval = {gll->lastposx, gll->lastposy};
    return retval;
}

void
gles_loc_layer_pos_sync
(T gll)
{
    assert(gll);
    gll->lastposx = gll->posx;
    gll->lastposy = gll->posy;
}

void 
gles_loc_layer_get_navistatus
(T gll, struct nav_status *ns)
{
    assert(gll);
    memcpy(ns, &(gll->nv), sizeof(struct nav_status));
}

int          
gles_loc_layer_isnavi_validate
(T gll)
{
    assert(gll);
    return gll->nv.validate;
}
struct vec2  
gles_loc_layer_get_navi_proj_pos
(T gll)
{
    assert(gll);
    struct vec2 retval = {gll->nv.projx, gll->nv.projy};
    return retval;
}
struct vec2  
gles_loc_layer_get_last_navi_proj_pos
(T gll)
{
    assert(gll);
    struct vec2 retval = {gll->lastprojx, gll->lastprojy};
    return retval;
}
void         
gles_loc_layer_navi_proj_sync
(T gll)
{
    assert(gll);
    gll->lastprojx = gll->nv.projx;
    gll->lastprojy = gll->nv.projy;
}

float        
gles_loc_layer_navi_azimuth
(T gll)
{
    assert(gll);
    return gll->nv.azimuth;
}

void         
gles_loc_layer_clean_pos
(T gll)
{
    assert(gll);
    gll->possetted = 0;
}


void         
gles_loc_layer_set_azimuth
(T gll, float azimuth)
{
    assert(gll);


    gll->azimuth = azimuth;
}

void         
gles_loc_layer_render_route
(T gll, 
 gles_2d_c_program_t gcp, 
 gles_2d_t_program_t gtp, 
 gles_font_t font,
 int state,
 const GLfloat *mmat)
{

    assert(gll);
    _render_loc_route(gll, gcp, font, mmat, state);

}
void         
gles_loc_layer_render_location
(T gll, 
 gles_2d_c_program_t gcp, 
 gles_2d_t_program_t gtp, 
 gles_font_t font,
 int state,
 const GLfloat *mmat)
{

    assert(gll);

    if(!gll->possetted)
        return;

    if(MAP_STATE_NORMAL == state ||
            MAP_STATE_TRACE == state)
        _render_loc_marker_normal(gll, gtp, mmat);
    else if(MAP_STATE_FOLLOW == state)
        _render_loc_marker_follow(gll, gtp);
    else if(MAP_STATE_NAVIGATE == state)
        _render_loc_marker_navigate(gll, gtp);
}

static int  _calc_jump_shape(T gll)
{
    int segcnt, i, j, jump, shapecnt;
    float seglen, dx, dy;

    jump = 0;
    for(i = 0; i < gll->nv.segindex; i++){
        const route_seg_t rs = routeresult_get_seg(gll->rr, i);
        dx = rs->endx - rs->startx;
        dy = rs->endy - rs->starty;
        seglen = sqrt(dx*dx + dy*dy);
        shapecnt = (int)ceil(seglen/(6.0*gll->routescale));
        j = (0 == i) ? 0 : 1;
        for(; j < shapecnt; j++){
            jump ++;
        }
        jump ++;
    }
    segcnt = routeresult_seg_count(gll->rr);
    if(i < segcnt){
        const route_seg_t rs = routeresult_get_seg(gll->rr, i);
        dx = gll->nv.projx - rs->startx;
        dy = gll->nv.projy - rs->starty;

        seglen = sqrt(dx*dx + dy*dy);
        shapecnt = (int)ceil(seglen/(6.0*gll->routescale));
        j = (0 == i) ? 0 : 1;
        for(; j < shapecnt; j++){
            jump ++;
        }
    }
    return jump;
}

static
void
_render_loc_route(T gll, gles_2d_c_program_t gcp, gles_font_t font, const GLfloat *mmat, int state)
{
    if(gll->route_shape_cnt<=0)
        return;

    int jump_shape;
    /**
     * 导航模式才需要跳过走过的导航线
     */
    if(gll->nv.validate && MAP_STATE_NAVIGATE == state){
        jump_shape = _calc_jump_shape(gll);
    }else{
        jump_shape = 0;
    }

    gles_2d_c_program_draw_elements(gcp, GL_TRIANGLES, 
                                    gll->route_verbuf, 
                                    gll->route_colbuf, 
                                    gll->route_indbuf + 12*jump_shape,
                                    (gll->route_shape_cnt-jump_shape)*12);

}

static 
void             
_render_loc_marker_normal(T gll, gles_2d_t_program_t gtp, const GLfloat *mmat)
{
    float pos[4];

    if(gll->animateflag){

        if(mover_arrive(gll->mover, gll->target, 0.1f)){
            gll->animateflag = 0;
        }else{
            mover_update(gll->mover);
            struct vec2 pos = mover_get_location(gll->mover);
            //gles_loc_layer_set_pos(gll, pos.x, pos.y);
            //改用map的set location 多了一点间接性, 但可以帮助trace模式实时检查到出界
            //又避免了每帧检查的开销
            gles_map_set_location(gll->map, pos.x, pos.y);

            /**
             * 将箭头转向导航段方向只是前面的dummy
            if(gll->nv.validate)
                gles_matrix_rotate2d_4(gll->objmat, 0, -(gll->nv.azimuth+gles_map_get_rotate_degree(gll->map)));*/
        }
    }

    gles_2d_t_program_bind_texture(gtp, gll->tid);

    /**
     * 策略改为只有导航模式才强制把marker画到线上
     *
    if(gll->nv.validate && gll->nv.errordist < 300){
        pos[0] = gll->nv.projx;
        pos[1] = gll->nv.projy;
    }else{
        pos[0] = gll->posx;
        pos[1] = gll->posy;
    }*/
    pos[0] = gll->posx;
    pos[1] = gll->posy;
    pos[2] = 0;
    pos[3] = 1;
    gles_matrix_mult_vec_4(mmat, pos, 0);


    gles_matrix_rotate2d_4(gll->objmat,
            /**
             * 按跟随模式的算法,
             * 转动的总角度(指示器marker的角度+地图转动角度) - 地图与正北方向的偏角
             *
             * 这里是转指示器, 注意和跟随模式转地图的方向相反
             * 所以相当于
             * 地图与正北方向的偏角 - 转动的总角度(指示器marker的角度+地图转动角度)
             */
            (float)(gles_map_get_map_deflection(gll->map) - 
            (gll->azimuth + gles_map_get_rotate_degree(gll->map)))
            );
    gles_matrix_postranslate_4(gll->objmat, pos[0], pos[1], pos[2]);


    gles_2d_t_program_set_matrices_o(gtp, gll->objmat);
    gles_2d_t_program_draw_arrays(gtp, GL_TRIANGLE_STRIP, _loc_quad, _loc_tex_coords, 4);
    gles_2d_t_program_reset_matrices_o(gtp);
}

static 
void             
_render_loc_marker_follow(T gll, gles_2d_t_program_t gtp)
{
    /**
     * 有没完的动画, 清了
     */
    if(gll->animateflag)
        gll->animateflag = 0;



    /**
     * marker不再旋转
     * follow模式转地图
     */
    gles_matrix_translate_4(gll->objmat, 0,
                                gles_map_get_view_width(gll->map)/2.0f,
                                gles_map_get_view_height(gll->map)/2.0f,
                                0);
    gles_2d_t_program_set_matrices_o(gtp, gll->objmat);
    gles_2d_t_program_bind_texture(gtp, gll->tid);
    gles_2d_t_program_draw_arrays(gtp, GL_TRIANGLE_STRIP, _loc_quad, _loc_tex_coords, 4);
    gles_2d_t_program_reset_matrices_o(gtp);
}

static 
void             
_render_loc_marker_navigate(T gll, gles_2d_t_program_t gtp)
{
    /**
     * 有没完的动画, 清了
     */
    if(gll->animateflag)
        gll->animateflag = 0;


    /**
     * marker旋转同normal模式
     * navigate模式按所在导航线方向转地图
     */
    gles_matrix_translate_4(gll->objmat, 0,
                                gles_map_get_view_width(gll->map)/2.0f,
                                gles_map_get_view_height(gll->map)/2.0f,
                                0);
    gles_matrix_post_rotation(gll->objmat, 
            /**
             * 按跟随模式的算法,
             * 转动的总角度(指示器marker的角度+地图转动角度) - 地图与正北方向的偏角
             *
             * 这里是转指示器, 注意和跟随模式转地图的方向相反
             * 所以相当于
             * 地图与正北方向的偏角 - 转动的总角度(指示器marker的角度+地图转动角度)
             */
            gles_map_get_map_deflection(gll->map) - (gll->azimuth + gles_map_get_rotate_degree(gll->map)),

            gles_map_get_view_width(gll->map)/2.0f,
            gles_map_get_view_height(gll->map)/2.0f);


    gles_2d_t_program_set_matrices_o(gtp, gll->objmat);
    gles_2d_t_program_bind_texture(gtp, gll->tid);
    gles_2d_t_program_draw_arrays(gtp, GL_TRIANGLE_STRIP, _loc_quad, _loc_tex_coords, 4);
    gles_2d_t_program_reset_matrices_o(gtp);
}
