#include<stdlib.h>
#include<string.h>
#include"assert.h"
#include"mem.h"
#include"arena.h"
#include"quadtree.h"
#include"qcdt.h"
#include"textblock.h"
#include"gles_unit_layer.h"
#include "gles_matrix.h"



#define T gles_unit_layer_t

struct T{
    gles_map_t      map;
    arena_t         arena;
    qcdt_t          qcdt;
    quadtree_t      qt_icon;
    quadtree_t      qt_text;
    GLuint          tid;
    float           width;
    float           height;
    int             visible;
    int             viewsized;
    int             vwidth;
    int             vheight;
    unsigned int    txtcolor;
};

struct unit_cl{
    T     gul;
    gles_font_t  font;
    float width;
    float height;
    const GLfloat *mmat;
};

struct icon_unit{
    int     id;
    int     type;
    float   x;
    float   y;
};

struct text_unit{
    int         id;
    textblock_t tb;
    char        *text;
};

static float            _pow(float x, int times);
static float            _map(float value, float istart, float istop, float ostart, float ostop);
static float            _calc_view_port_depth(T gul, float viewportwidth, float viewportheight);
static void             _calc_tex_coords(int type);
static void             _render(void *obj, envelop_t e, void *cl);
static void             _render_icons(T gul, gles_2d_t_program_t gtp, const float *mmat);
static struct textbound _text_bounds(const char *line, float x, float y, void *cl);
static void             _clean_cr(T gul);
static int              _check_cr(T gul, float left, float top, float right, float bottom);

static GLfloat _texCoords[] = {0.0f, 1.0f,
                                1.0f, 1.0f,
                                0.0f, 0.0f,
                                1.0f, 0.0f
                                };

static GLfloat _quad[]     ={
                            -20.0f, -21.0f,
                             20.0f, -21.0f,
                            -20.0f, 21.0f,
                             20.0f, 21.0f
                            };


T            
gles_unit_layer_new
(gles_map_t map, GLuint tid, int mapwidth, int mapheight)
{
    T gul;

    gul = ALLOC(sizeof(*gul));
    gul->map        = map;
    gul->tid        = tid;
    gul->width      = mapwidth;
    gul->height     = mapheight;
    gul->viewsized  = 0;
    gul->visible    = 0;
    gul->arena      = arena_new();
    gul->qt_text    = quadtree_new(gul->arena, 0, 0, mapwidth, mapheight);
    gul->qt_icon    = quadtree_new(gul->arena, 0, 0, mapwidth, mapheight);
    gul->qcdt       = qcdt_new(0, 0, mapwidth, mapheight);
    gul->txtcolor   = 0xFF303030;
    return gul;
}

void         
gles_unit_layer_free
(T *gul)
{
    assert(gul);
    assert(*gul);

    
    qcdt_free(&((*gul)->qcdt));
    arena_dispose(&((*gul)->arena));
    FREE(*gul);
}

void         
gles_unit_set_visible
(T gul, int visible)
{
    assert(gul);
    gul->visible = visible;
}

int          
gles_unit_quad_tree_depth
(T gul)
{
    assert(gul);
    return quadtree_depth(gul->qt_text);
}

void         
gles_unit_view_size
(T gul, int vwidth, int vheight)
{
    assert(gul);
    assert(vwidth > 0);
    assert(vheight > 0);

    

    gul->vwidth = vwidth;
    gul->vheight= vheight;

    gul->viewsized = 1;
}


void         
gles_unit_insert_icon
(T gul, int id, int type, double left, double top, double right, double bottom)
{
    struct icon_unit *iu = ARENA_ALLOC(gul->arena, sizeof(*iu));
    iu->id = id;
    iu->type = type;
    iu->x = (float)((left + right)/2);
    iu->y = gul->height - (float)((top + bottom)/2);
    quadtree_insert(gul->qt_icon, id, iu, left, top, right, bottom);
}

void         
gles_unit_insert_text
(T gul, int id, double left, double top, double right, double bottom, const char *text)
{
    struct text_unit *tu = ARENA_ALLOC(gul->arena, sizeof(*tu));
    int len = strlen(text);
    tu->id = id;
    tu->tb = NULL;
    tu->text = ARENA_CALLOC(gul->arena, 1, len+1);
    strncpy(tu->text, text, len);
    quadtree_insert(gul->qt_text, id, tu, left, top, right, bottom);
}

int          
gles_unit_find_text
(T gul, float x, float y, float range)
{
    int retval;
    struct text_unit *tu;

    assert(gul);

    seq_t   searchlist;

    searchlist = quadtree_search(gul->qt_text, x, y, x, y, range, 1, NULL);

    if(seq_length(searchlist) > 0){
        tu = seq_remove_high(searchlist);
        retval = tu->id;
    }else{
        retval = -1;
    }

    seq_free(&searchlist);

    return retval;
}
int          
gles_unit_find_icon
(T gul, float x, float y, float range)
{
    int retval;
    struct icon_unit *iu;

    assert(gul);

    seq_t   searchlist;

    searchlist = quadtree_search(gul->qt_icon, x, y, x, y, range, 1, NULL);

    if(seq_length(searchlist) > 0){
        iu = seq_remove_high(searchlist);
        retval = iu->id;
    }else{
        retval = -1;
    }

    seq_free(&searchlist);

    return retval;
}

void         
gles_unit_layer_render
(T gul, 
 gles_2d_c_program_t gcp, 
 gles_2d_t_program_t gtp, 
 gles_font_t  font, const GLfloat *mmat, const GLfloat *inmmat)
{
    int i, maxdepth;
    float depth;
    float left, top, right, bottom;
    float extent[16];
    float xs[4];
    float ys[4];

    if(!gul->visible){
        return ;
    }
    if(!gul->viewsized){
        return ;
    }


    /**
     * 先把当前屏幕的四角, 用地图变换矩阵的逆矩阵变换到地图坐标系
     */
    extent[0] = 0;
    extent[1] = gul->vheight;
    extent[2] = 0;
    extent[3] = 1;

    extent[4] = gul->vwidth;
    extent[5] = gul->vheight;
    extent[6] = 0;
    extent[7] = 1;

    extent[8] = gul->vwidth;
    extent[9] = 0;
    extent[10]= 0;
    extent[11]= 1;

    extent[12] = 0;
    extent[13] = 0;
    extent[14] = 0;
    extent[15] = 1;
    gles_matrix_mult_vec_4(inmmat, extent, 0);
    gles_matrix_mult_vec_4(inmmat, extent, 4);
    gles_matrix_mult_vec_4(inmmat, extent, 8);
    gles_matrix_mult_vec_4(inmmat, extent, 12);

    xs[0] = extent[0];
    xs[1] = extent[4];
    xs[2] = extent[8];
    xs[3] = extent[12];

    ys[0] = extent[1];
    ys[1] = extent[5];
    ys[2] = extent[9];
    ys[3] = extent[13];

    left = right = xs[0];
    top  = bottom = ys[0];
    for(i = 0; i < 4; i++){
        if(left > xs[i]){
            left = xs[i];
        }
        if(right < xs[i]){
            right = xs[i];
        }
        if(top > ys[i]){
            top = ys[i];
        }
        if(bottom < ys[i]){
            bottom = ys[i];
        }
    }
    /**
     * 为了简化计算, 实际上是用的屏幕四角在地图坐标系中的范围
     * 沿地图坐标轴方向
     */

    struct unit_cl cl;
    cl.width = gul->vwidth;
    cl.height = gul->vheight;
    cl.gul = gul;
    cl.mmat = mmat;
    cl.font = font;

    /**
     * 清空碰撞检查表
     */
    _clean_cr(gul);

    /**
     * 先独立渲染图标
     */
    _render_icons(gul, gtp, mmat);

    /**
     * 准备渲染文字, 先给字体对象重置变换矩阵(因为前面可能有其他渲染变更了字体的变换矩阵)
     */
    gles_font_reset_mat(font);

    /**
     * 以重新计算的地图显示范围, 调用四叉树的渲染方法, 传递_render回调函数和cl闭包数据给它
     */
    quadtree_render(gul->qt_text,
                    left, gul->height - bottom,
                    right, gul->height - top,
                    _render,
                    &cl);
    /*
    depth = _calc_view_port_depth(gul, right - left, bottom - top);
    maxdepth = quadtree_depth(gul->qt_text);
    if(depth >= (maxdepth - 2)){
        quadtree_render(gul->qt_text,
                        left, gul->height - bottom,
                        right, gul->height - top,
                        _render,
                        &cl);
    }else{
        quadtree_render_depth(gul->qt_text, (int)(depth+2.0f),
                        left, gul->height - bottom,
                        right, gul->height - top,
                        _render,
                        &cl);
    }*/
}

static 
void             
_render_icons(T gul, gles_2d_t_program_t gtp, const float *mmat)
{
    float pos[4];
    float hw, hh, x, y;
    int len, i;

    gles_2d_t_program_bind_texture(gtp, gul->tid);

    
    len = quadtree_get_count(gul->qt_icon);
    for(i = 0; i < len; i++){
        struct icon_unit *iu = quadtree_get_by_index(gul->qt_icon, i);
        pos[0] = iu->x;
        pos[1] = iu->y;
        pos[2] = 0;
        pos[3] = 1;
        gles_matrix_mult_vec_4(
                mmat,
                pos, 0);
        x = pos[0];
        y = pos[1];
        hw = 32;
        hh = 32;
        _quad[0] = x-hw;
        _quad[1] = y-hh;
        _quad[2] = x+hw;
        _quad[3] = y-hh;
        _quad[4] = x-hw;
        _quad[5] = y+hh;
        _quad[6] = x+hw;
        _quad[7] = y+hh;

        if(!_check_cr(gul, x-hw, y-hh, x+hw, y+hh)){
            continue;
        }
        _calc_tex_coords(iu->type);
        gles_2d_t_program_draw_arrays(gtp, GL_TRIANGLE_STRIP, _quad, _texCoords, 4);
    }
}

static 
void             
_calc_tex_coords(int type)
{
    int row, col;
    row = (type-1) / 8;
    col = (type-1) % 8;

    _texCoords[0] = (1.0f/8.0f) * (col);
    _texCoords[1] = (1.0f/8.0f) * (row+1);

    _texCoords[2] = (1.0f/8.0f) * (col+1);
    _texCoords[3] = (1.0f/8.0f) * (row+1);

    _texCoords[4] = (1.0f/8.0f) * (col);
    _texCoords[5] = (1.0f/8.0f) * (row);

    _texCoords[6] = (1.0f/8.0f) * (col+1);
    _texCoords[7] = (1.0f/8.0f) * (row);
    /*
static GLfloat _texCoords[] = {0.0f, 1.0f,
                                1.0f, 1.0f,
                                0.0f, 0.0f,
                                1.0f, 0.0f
                                };
    材质坐标的计算, 看起来有点怪
    其实坐标按绘图的原点在左上, Y轴向下来取

    点序列按和_quad上下颠倒的顺序

       2       3
        ┌─────┐
        │     │
        │     │
        └─────┘
       0       1
    *
    _texCoords[0] = 0.0f;
    _texCoords[1] = 1.0f/8.0f;

    _texCoords[2] = 1.0f/8.0f;
    _texCoords[3] = 1.0f/8.0f;

    _texCoords[4] = 0.0f;
    _texCoords[5] = 0.0f;

    _texCoords[6] = 1.0f/8.0f;
    _texCoords[7] = 0.0f;*/
}

static 
void 
_render(void *obj, envelop_t e, void *cl)
{
    struct text_unit *tu;
    struct unit_cl *uc;
    float pos[4];
    int linecnt, i;

    tu = obj;
    do{
        uc = (struct unit_cl*)cl;


        pos[0] = (float)envelop_center_x(e);
        pos[1] = uc->gul->height - (float)envelop_center_y(e);
        pos[2] = 0;
        pos[3] = 1;

        gles_matrix_mult_vec_4(
                uc->mmat, 
                pos, 0);


         //查看该要素有无所属的text_block, 如无
         //给它生成一个
        if(NULL == tu->tb){
            tu->tb = textblock_new(uc->gul->arena, tu->text, _text_bounds, cl);
        }


         //碰撞查表
        struct textbound bb = textblock_bound(tu->tb);



        pos[1]-=((bb.bottom-bb.top)/2);
        if(!_check_cr(uc->gul, pos[0]+bb.left, pos[1]+bb.top, pos[0]+bb.right, pos[1]+bb.bottom)){
            break;
        }

        //渲染要素的text_block
        linecnt = textblock_line_count(tu->tb);
        for(i = 0; i < linecnt; i ++){
            textline_t line = textblock_get_line(tu->tb, i);
            struct textbound lb = textline_bound(line);
            float lh = lb.bottom - lb.top;
            gles_font_drawtext(uc->font,
                                textline_get_text(line),
                                uc->gul->txtcolor,
                                pos[0], 
                                pos[1]+i*(lb.bottom-lb.top));
        }

    }while(0);
}

static 
struct textbound 
_text_bounds(const char *line, float x, float y, void *cl)
{
    struct unit_cl *uc;
    struct bounds_t textbounds;
    struct textbound retval;
    uc = (struct unit_cl*)cl;

    gles_font_text_bounds(uc->font, line, &textbounds, x, y);

    retval.left     = textbounds.left;
    retval.top      = textbounds.top;
    retval.right    = textbounds.right;
    retval.bottom   = textbounds.bottom;

    return retval;
}

static 
void             
_clean_cr(T gul)
{
    qcdt_reset(gul->qcdt);
}

static 
int              
_check_cr(T gul, float left, float top, float right, float bottom)
{
    return qcdt_cd_check(gul->qcdt, left, top, right, bottom);
}

float 
_calc_view_port_depth
(T gul, float viewportwidth, float viewportheight)
{
    int i;
    float ret = 0;
    float viewportarea = viewportwidth * viewportheight;
    float maparea = gul->width * gul->height;
    int  maxdepth = quadtree_depth(gul->qt_text);

    if(viewportarea >= maparea){
        ret = 0;
    }else if(viewportarea <= maparea * _pow(0.25f, maxdepth)){
        ret = maxdepth;
    }else {
        for(i = 1; i <= maxdepth; i++) {
            float prearea = maparea * _pow(0.25f, i-1);
            float curarea = maparea * _pow(0.25f, i);
            if(viewportarea > curarea && viewportarea <= prearea){
                ret = _map(viewportarea, prearea, curarea, i+1, i+2);
                break;
            }
        }
    }
    return ret;
}

static
float            
_pow(float x, int times)
{
    if(times <= 0){
        return 1.0f;
    }else{
        int i;
        for(i = 0; i < times; i++){
            x *= x;
        }
        return x;
    }
}

static
float
_map(float value, float istart, float istop, float ostart, float ostop)
{
    return ostart + (ostop - ostart) * ((value - istart) / (istop - istart));
}
