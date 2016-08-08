#include"assert.h"
#include"mem.h"
#include"seq.h"
#include"sparsearray.h"
#include"gles_marker_layer.h"

#ifdef ANDROID_IDRN
#include"error_functions.h"
#endif

#define T gles_marker_layer_t

struct T{
    gles_map_t      map;
    int             width;
    int             height;
    int             visible;
    int             cursor;
    sparsearray_t   markers;
    seq_t           trash;
};

struct marker{
    int             mclass;
    int             type;
    int             id;
    float           x;
    float           y;
    float           offsetx;
    float           offsety;
    float           scale;
    union{
        gles_texture_android_t txa;
    }texture;
};

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

static void     _render_marker(const struct marker *m, gles_2d_t_program_t gtp, const GLfloat *mmat);

static void     _free_marker_texture(struct marker *m);

T            
gles_marker_layer_new
(gles_map_t map, int mwidth, int mheight)
{
    T gml;
    assert(map);

    gml = ALLOC(sizeof(*gml));
    gml->width  = mwidth;
    gml->height = mheight;
    gml->visible= 0;
    gml->cursor = 0;
    gml->markers = sparsearray_new(32);
    gml->trash  = seq_new(512);
    
    return gml;
}

void         
gles_marker_layer_free
(T *gml)
{
    assert(gml);
    assert(*gml);

    int len, i;
    struct marker   *m;

    /**
     * 先释放在绘制列表中的marker
     */
    len = sparsearray_length((*gml)->markers);
    for(i = 0; i < len; i++){
        m = sparsearray_get_at((*gml)->markers, i);
        _free_marker_texture(m);
        FREE(m);
    }
    sparsearray_free(&((*gml)->markers));


    /**
     * 再释放在回收表中的marker
     */
    len = seq_length((*gml)->trash);
    for(i = 0; i < len; i++){
        m = seq_get((*gml)->trash, i);
        _free_marker_texture(m);
        FREE(m);
    }
    seq_free(&((*gml)->trash));

    FREE(*gml);
}


void         
gles_marker_layer_render
(T gml, gles_2d_t_program_t gtp, const GLfloat *mmat)
{
    assert(gml);
    struct marker *m;
    int len, i;
    if(gml->visible){
        len = sparsearray_length(gml->markers);
        for(i = 0; i < len; i++){
            m = sparsearray_get_at(gml->markers, i);
            _render_marker(m, gtp, mmat);
        }
    }

    /**
     * 每帧绘制完毕检查并
     * 回收表中的marker
     * 因为释放GL贴图对象需要在GL线程中
     */
    len = seq_length(gml->trash);
    for(i = 0; i < len; i++){
        m = seq_remove_high(gml->trash);
        _free_marker_texture(m);
        FREE(m);
    }
}

void         
gles_marker_layer_set_visible
(T gml, int visible)
{
    assert(gml);
    gml->visible = visible;
}

int 
gles_marker_layer_find
(T gml, float x, float y)
{
    assert(gml);
    struct marker *m;
    float rsq, dx, dy;
    int len, i;
    y = gml->height - y;
    if(gml->visible){
        len = sparsearray_length(gml->markers);

        for(i = 0; i < len; i++){
            m = sparsearray_get_at(gml->markers, i);
            dx = x - (m->x - m->offsetx*m->scale);
            dy = y - (m->y + m->offsety*m->scale);
            rsq = m->scale * (gles_texture_android_width(m->texture.txa) + gles_texture_android_height(m->texture.txa))/4.0f;
            rsq = rsq*rsq;
            if((dx*dx + dy*dy) <= rsq){
                return m->id;
            }
        }
    }
    return -1;
}

int          
gles_marker_layer_insert_android
(T gml, gles_texture_android_t txa, int mclass, float x, float y, float offsetx, float offsety, float scale)
{
    assert(gml);

    int ret = gml->cursor;
    struct marker *m = ALLOC(sizeof(*m));
    m->mclass   = mclass;
    m->id       = ret;
    m->type     = 1;
    m->x        = x;
    m->y        = gml->height -  y;
    m->offsetx  = offsetx;
    m->offsety  = offsety;
    m->scale    = scale;
    m->texture.txa = txa;
    sparsearray_put(gml->markers, gml->cursor++ , m);
    return ret;
}

void         
gles_marker_layer_remove_class
(T gml, int mclass)
{
    assert(gml);
    int len, i;
    int *rmlist;
    struct marker *m;
    len = sparsearray_length(gml->markers);

    /**
     * 这里看起来略繁琐, 是因为不能在遍历容器时删除其中对象
     * 所以我们先遍历, 记录该类的对象, 然后再按key来remove
     */
    rmlist = ALLOC(sizeof(int)*len);
    for(i = 0; i < len; i++){
        m = sparsearray_get_at(gml->markers, i);
        if(mclass == m->mclass){
            rmlist[i] = m->id;
        }else{
            rmlist[i] = -1;
        }
    }
    for(i = 0; i < len; i++){
        if(rmlist[i] >= 0){
            m = sparsearray_remove(gml->markers, rmlist[i]);
            seq_add_low(gml->trash, m);
        }
    }
    FREE(rmlist);
}

int          
gles_marker_update_position
(T gml, int markerid, float x, float y)
{
    assert(gml);
    struct marker *m = sparsearray_get(gml->markers, markerid);
    if(NULL != m){
        m->x        = x;
        m->y        = gml->height -  y;
        return 1;
    }else{
        return 0;
    }
}

void         
gles_marker_remove
(T gml, int markerid)
{
    assert(gml);
    /**
     * 由于需要有效释放GL贴图对象, 删除某marker时并不直接释放
     * 而是将它从绘制表中移除, 插入到回收队列
     * 每帧绘制后会检查并释放回收队列中的marker
     */
    struct marker *m = sparsearray_remove(gml->markers, markerid);
    seq_add_low(gml->trash, m);
}

void         
gles_marker_clean
(T gml)
{
    int len, i;
    struct marker *m;
    len = sparsearray_length(gml->markers);
    for(i = 0; i < len; i++){
        m = sparsearray_get_at(gml->markers, i);
        seq_add_low(gml->trash, m);
    }
    /**
     * 由于需要有效释放GL贴图对象, 清除marker时并不直接释放
     * 而是将它们从绘制表中移除, 插入到回收队列
     * 每帧绘制后会检查并释放回收队列中的marker
     */
}

static 
void     
_render_marker(const struct marker *m, gles_2d_t_program_t gtp, const GLfloat *mmat)
{
    unsigned int tid;
    float pos[4];
    float hw, hh, x, y;


    if(1 == m->type){
        tid = gles_texture_android_tid(m->texture.txa);
        /**
         * 没加载就加载一下
         */
        if(0 == tid){
            gles_texture_android_load(m->texture.txa);
            tid = gles_texture_android_tid(m->texture.txa);
        }

        pos[0] = m->x;
        pos[1] = m->y;
        pos[2] = 0;
        pos[3] = 1;
        gles_matrix_mult_vec_4(
                mmat,
                pos, 0);
        x = pos[0] - m->offsetx*m->scale;
        y = pos[1] + m->offsety*m->scale;
        hw = m->scale * gles_texture_android_width(m->texture.txa)/2;
        hh = m->scale * gles_texture_android_height(m->texture.txa)/2;

        _quad[0] = x-hw;
        _quad[1] = y-hh;
        _quad[2] = x+hw;
        _quad[3] = y-hh;
        _quad[4] = x-hw;
        _quad[5] = y+hh;
        _quad[6] = x+hw;
        _quad[7] = y+hh;
        gles_2d_t_program_bind_texture(gtp, tid);
        gles_2d_t_program_draw_arrays(gtp, GL_TRIANGLE_STRIP, _quad, _texCoords, 4);
    }
}

static 
void     
_free_marker_texture(struct marker *m)
{
    if(1 == m->type){
        gles_texture_android_free(&(m->texture.txa));
    }
}
