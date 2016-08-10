#include<stdio.h>
#include<string.h>
#include<pthread.h>
#include"assert.h"
#include"except.h"
#include"mem.h"
#include"gles_map_render.h"
#include"gles_map.h"
#include"gles_font.h"
#include"gles_2d_c_program.h"
#include"gles_2d_t_program.h"
#include "gles_utils.h"
#include "gles_matrix.h"

#ifdef ANDROID_IDRN
#include"error_functions.h"
#endif

#define TEXTURE_COUNT 2

#define T gles_map_render_t

struct T{

    pthread_mutex_t     mtx;
    gles_2d_c_program_t gcp;        //颜色着色程序
    gles_2d_t_program_t gtp;        //贴图着色程序
    gles_font_t         gft;        //字型库
    gles_map_t          map;        //当前渲染的地图
    gles_texture_png_t  textures[TEXTURE_COUNT];//默认的贴图
    float               pmat[16];   //投影矩阵
    float               width;      //GLSurface的尺寸
    float               height;
    struct background{
        float               red;
        float               green;
        float               blue;
    }                   color;
    char               *folder;     //默认目录
};

static void _load_font_and_textures(T gmr);

static void _except_handler(const except_t *e,
                        const char *file,
                        const char *func,
                        const char *reason,
                        int line);

T    
gles_map_render_new
(const char *folder)
{
    assert(folder);

    T gmr;
    int len;
    len = strlen(folder);

    gmr = ALLOC(sizeof(*gmr) + len + 1);

    pthread_mutex_init(&(gmr->mtx), NULL);

    gmr->folder = (char*) (gmr+ 1);
    strncpy(gmr->folder, folder, len+1);

    gmr->map = NULL;
    gmr->color.red  = 1.0f;
    gmr->color.green= 1.0f;
    gmr->color.blue = 1.0f;

    except_set_default_handler(_except_handler);

    return gmr;
}

void 
gles_map_render_free
(T *gmr)
{
    assert(gmr);
    assert(*gmr);

    int s;
    pthread_mutex_t     mtx = (*gmr)->mtx;
    s = pthread_mutex_lock(&mtx);
    assert(0 == s);
    gles_2d_c_program_free(&((*gmr)->gcp));
    gles_2d_t_program_free(&((*gmr)->gtp));
    gles_font_free(&((*gmr)->gft));

    if((*gmr)->map)
        gles_map_free(&((*gmr)->map));

    FREE(gmr);
    s = pthread_mutex_unlock(&mtx);
    assert(0 == s);
    pthread_mutex_destroy(&mtx);
}

void 
gles_map_render_init
(T gmr)
{
    assert(gmr);
    gles_print_info();
    gmr->gcp = gles_2d_c_program_new();
    gmr->gtp = gles_2d_t_program_new();

    _load_font_and_textures(gmr);

}

void 
gles_map_render_resize
(T gmr, int width, int height)
{
    assert(gmr);
    int s;
    s = pthread_mutex_lock(&(gmr->mtx));
    assert(0 == s);


    gmr->width = width;
    gmr->height= height;
    gles_matrix_ortho_4(gmr->pmat, 0, width, 0, height, -1, 1);
    glViewport(0, 0, width, height);

    gles_2d_c_program_set_matrices_p(gmr->gcp, gmr->pmat);
    gles_2d_t_program_set_matrices_p(gmr->gtp, gmr->pmat);
    gles_font_pmat(gmr->gft, gmr->pmat);

    if(NULL != gmr->map){
        gles_map_viewsize(gmr->map, width, height);
    }

    s = pthread_mutex_unlock(&(gmr->mtx));
    assert(0 == s);
}

void 
gles_map_render_render
(T gmr, float azimuth)
{
    int s;
    s = pthread_mutex_lock(&(gmr->mtx));
    assert(0 == s);

    glClear(GL_COLOR_BUFFER_BIT);
    glClearColor(gmr->color.red, gmr->color.green, gmr->color.blue, 1.0f);

    if(NULL != gmr->map){
        gles_map_render(gmr->map, gmr->gcp, gmr->gtp, gmr->gft, azimuth);
    }
    s = pthread_mutex_unlock(&(gmr->mtx));
    assert(0 == s);
}

void                 
gles_map_render_set_unit_visible
(T gmr, int visible)
{
    int s;
    s = pthread_mutex_lock(&(gmr->mtx));
    assert(0 == s);

    if(NULL != gmr->map)
        gles_map_set_unit_visible(gmr->map, visible);

    s = pthread_mutex_unlock(&(gmr->mtx));
    assert(0 == s);
}
void                 
gles_map_render_insert_icon
(T gmr, int id, int type, double left, double top, double right, double bottom)
{
    int s;
    s = pthread_mutex_lock(&(gmr->mtx));
    assert(0 == s);

    if(NULL != gmr->map)
        gles_map_insert_icon(gmr->map, id, type, left, top, right, bottom);

    s = pthread_mutex_unlock(&(gmr->mtx));
    assert(0 == s);
}
void                 
gles_map_render_insert_text
(T gmr, int id, double left, double top, double right, double bottom, const char *text)
{
    int s;
    s = pthread_mutex_lock(&(gmr->mtx));
    assert(0 == s);
    if(NULL != gmr->map)
        gles_map_insert_text(gmr->map, id, left, top, right, bottom, text);
    s = pthread_mutex_unlock(&(gmr->mtx));
    assert(0 == s);
}
void                 
gles_map_render_set_unit_finish
(T gmr)
{
    int s;
    s = pthread_mutex_lock(&(gmr->mtx));
    assert(0 == s);
    if(NULL != gmr->map)
        gles_map_set_unit_finish(gmr->map);
    s = pthread_mutex_unlock(&(gmr->mtx));
    assert(0 == s);
}

int                  
gles_map_render_find_text
(T gmr, float x, float y, float range)
{
    int s, retval;
    s = pthread_mutex_lock(&(gmr->mtx));
    assert(0 == s);

    if(NULL != gmr->map){
        retval = gles_map_find_text(gmr->map, x, y, range);
    }else{
        retval = -1;
    }
    s = pthread_mutex_unlock(&(gmr->mtx));
    assert(0 == s);
    return retval;
}
int                  
gles_map_render_find_icon
(T gmr, float x, float y, float range)
{
    int s, retval;
    s = pthread_mutex_lock(&(gmr->mtx));
    assert(0 == s);

    if(NULL != gmr->map){
        retval = gles_map_find_icon(gmr->map, x, y, range);
    }else{
        retval = -1;
    }
    s = pthread_mutex_unlock(&(gmr->mtx));
    assert(0 == s);
    return retval;
}

void 
gles_map_render_setmap
(T gmr, const char *vbfile)
{
    int s;
    s = pthread_mutex_lock(&(gmr->mtx));
    assert(0 == s);

    if(NULL != gmr->map){
        gles_map_free(&(gmr->map));
    }

    if(NULL != vbfile){
        gmr->map = gles_map_new(gmr, vbfile);
        gles_map_viewsize(gmr->map, gmr->width, gmr->height);
    }else{
        gmr->map = NULL;
    }

    s = pthread_mutex_unlock(&(gmr->mtx));
    assert(0 == s);
}

float                
gles_map_render_rotate_degree   
(T gmr)
{
    float retval;
    int s;
    s = pthread_mutex_lock(&(gmr->mtx));
    assert(0 == s);
    if(NULL != gmr->map)
        retval = gles_map_get_rotate_degree(gmr->map);
    else
        retval = 0.0f;
    s = pthread_mutex_unlock(&(gmr->mtx));
    assert(0 == s);
    return retval;
}

gles_texture_png_t  
gles_map_render_get_texture
(T gmr, int index)
{
    assert(gmr);
    assert(index >= 0);
    assert(index < TEXTURE_COUNT);
    return gmr->textures[index];
}

void                 
gles_map_render_get_navistatus  
(T gmr, struct nav_status *ns)
{
    int retval, s;
    assert(gmr);

    s = pthread_mutex_lock(&(gmr->mtx));
    assert(0 == s);

    if(gmr->map){
        gles_map_get_navistatus(gmr->map, ns);
    }

    s = pthread_mutex_unlock(&(gmr->mtx));
    assert(0 == s);
}

int          
gles_map_render_ontouch
(T gmr, int action, float x0, float y0, float x1, float y1)
{
    int retval, s;
    assert(gmr);

    s = pthread_mutex_lock(&(gmr->mtx));
    assert(0 == s);


    retval = 0;
    if(gmr->map){
        retval = gles_map_ontouch(gmr->map, action, x0, y0, x1, y1);
    }

    s = pthread_mutex_unlock(&(gmr->mtx));
    assert(0 == s);
    return retval;
}

void                 
gles_map_render_event           
(T gmr, int event, float x, float y)
{
    assert(gmr);

    int s;
    s = pthread_mutex_lock(&(gmr->mtx));
    assert(0 == s);

    if(gmr->map)
        gles_map_dummy_event(gmr->map, event, x, y);

    s = pthread_mutex_unlock(&(gmr->mtx));
    assert(0 == s);
}

//int                  
//gles_map_render_add_marker_android
//(T gmr, gles_texture_android_t txa, int mclass, float x, float y, float offsetx, float offsety, float scale)
//{
//    assert(gmr);
//
//    int s, retval;
//    s = pthread_mutex_lock(&(gmr->mtx));
//    assert(0 == s);
//
//    if(gmr->map)
//        retval = gles_map_add_marker_android(gmr->map, txa, mclass, x, y, offsetx, offsety, scale);
//
//    s = pthread_mutex_unlock(&(gmr->mtx));
//    assert(0 == s);
//    return retval;
//}
void
gles_map_render_markers_visible
(T gmr, int visible)
{
    assert(gmr);

    int s;
    s = pthread_mutex_lock(&(gmr->mtx));
    assert(0 == s);

    if(gmr->map)
        gles_map_markers_visible(gmr->map, visible);

    s = pthread_mutex_unlock(&(gmr->mtx));
    assert(0 == s);
}
void                 
gles_map_render_remove_marker
(T gmr, int markerid)
{
    assert(gmr);

    int s;
    s = pthread_mutex_lock(&(gmr->mtx));
    assert(0 == s);

    if(gmr->map)
        gles_map_remove_marker(gmr->map, markerid);

    s = pthread_mutex_unlock(&(gmr->mtx));
    assert(0 == s);
}
void                 
gles_map_render_remove_marker_class
(T gmr, int mclass)
{
    assert(gmr);

    int s;
    s = pthread_mutex_lock(&(gmr->mtx));
    assert(0 == s);

    if(gmr->map)
        gles_map_remove_marker_class(gmr->map, mclass);

    s = pthread_mutex_unlock(&(gmr->mtx));
    assert(0 == s);
}
void                 
gles_map_render_clean_marker
(T gmr)
{
    assert(gmr);

    int s;
    s = pthread_mutex_lock(&(gmr->mtx));
    assert(0 == s);

    if(gmr->map)
        gles_map_clean_marker(gmr->map);

    s = pthread_mutex_unlock(&(gmr->mtx));
    assert(0 == s);
}
int                  
gles_map_render_find_marker
(T gmr, float x, float y)
{
    assert(gmr);

    int s, retval;
    s = pthread_mutex_lock(&(gmr->mtx));
    assert(0 == s);

    if(gmr->map)
        retval = gles_map_find_marker(gmr->map, x, y);
    else 
        retval = -1;

    s = pthread_mutex_unlock(&(gmr->mtx));
    assert(0 == s);

    return retval;
}

int                  
gles_map_render_update_marker_position
(T gmr, int markerid, float x, float y)
{
    assert(gmr);

    int s, retval;
    s = pthread_mutex_lock(&(gmr->mtx));
    assert(0 == s);

    if(gmr->map)
        retval = gles_map_update_marker_position(gmr->map, markerid, x, y);
    else 
        retval = -1;

    s = pthread_mutex_unlock(&(gmr->mtx));
    assert(0 == s);

    return retval;
}

void                 
gles_map_render_zoom_fit        
(T gmr, int type)
{
    assert(gmr);
    int s;
    s = pthread_mutex_lock(&(gmr->mtx));
    assert(0 == s);

    if(gmr->map){

        switch(type){
            case ZOOM_FIT_WIDTH:
                gles_map_zoom_fit_width(gmr->map);
            break;
            case ZOOM_FIT_HEIGHT:
                gles_map_zoom_fit_height(gmr->map); 
            break;
            case ZOOM_FIT_FIT:
                gles_map_zoom_fit_fit(gmr->map);
            break;
        }
    }

    s = pthread_mutex_unlock(&(gmr->mtx));
    assert(0 == s);
}

void                 
gles_map_render_set_route_data
(T gmr, int cnt, float *pts)
{
    int i, s;
    assert(gmr);
    s = pthread_mutex_lock(&(gmr->mtx));
    assert(0 == s);

    if(gmr->map){

        /**
         * 由于地图是按屏幕绘图原点来建立的
         * 原点在左上角
         *
         * 而OpenGL我们的正投影矩阵指定的原点在左下角,
         * 所以外部app传入它们使用的地图坐标
         * 需要再校正y坐标得到OpenGL渲染使用的地图坐标
         */
        for(i = 0; i < cnt; i++){
            pts[i*2 + 1] = gles_map_screen_coord_y(gmr->map, pts[i*2 + 1]);
        }
        gles_map_set_route_data(gmr->map, cnt, pts);
    }

    s = pthread_mutex_unlock(&(gmr->mtx));
    assert(0 == s);
}
void                 
gles_map_render_clean_route_data
(T gmr)
{
    assert(gmr);
    int s;
    s = pthread_mutex_lock(&(gmr->mtx));
    assert(0 == s);

    if(gmr->map){
        gles_map_clean_route_data(gmr->map);
    }

    s = pthread_mutex_unlock(&(gmr->mtx));
    assert(0 == s);
}

struct vec2          
gles_map_render_screen_to_map   
(T gmr, float x, float y)
{
    assert(gmr);
    struct vec2 retval = {0, 0};
    int s;
    s = pthread_mutex_lock(&(gmr->mtx));
    assert(0 == s);
    if(gmr->map)
        retval = gles_map_screen_to_map(gmr->map, x, y);

    /**
     * 由于地图是按屏幕绘图原点来建立的
     * 原点在左上角
     *
     * 而OpenGL我们的正投影矩阵指定的原点在左下角,
     * 所以外部app调用screen_to_map函数后
     * 需要再校正y坐标
     */
    retval.y = gles_map_screen_coord_y(gmr->map, retval.y);

    s = pthread_mutex_unlock(&(gmr->mtx));
    assert(0 == s);
    return retval;
}

struct vec2          
gles_map_render_map_to_screen   
(T gmr, float x, float y)
{
    assert(gmr);
    struct vec2 retval = {0, 0};
    int s;
    s = pthread_mutex_lock(&(gmr->mtx));
    assert(0 == s);
    if(gmr->map)
        retval = gles_map_map_to_screen(gmr->map, 
                    x, 
                    gles_map_screen_coord_y(gmr->map, y));
    /**
     * 由于地图是按屏幕绘图原点来建立的
     * 原点在左上角
     *
     * 而OpenGL我们的正投影矩阵指定的原点在左下角,
     * 在GL层面使用的地图坐标和外部app使用的地图坐标,
     * 需要校正y值
     */

    s = pthread_mutex_unlock(&(gmr->mtx));
    assert(0 == s);

    return retval;
}


void                 
gles_map_render_set_state       
(T gmr, int state)
{
    assert(gmr);
    int s;
    s = pthread_mutex_lock(&(gmr->mtx));
    assert(0 == s);
    if(gmr->map)
        gles_map_set_state(gmr->map, state);
    s = pthread_mutex_unlock(&(gmr->mtx));
    assert(0 == s);
}

void                 
gles_map_render_set_location    
(T gmr, float x, float y)
{
    assert(gmr);
    int s;
    s = pthread_mutex_lock(&(gmr->mtx));
    assert(0 == s);
    if(gmr->map){
        /**
         * 由于地图是按屏幕绘图原点来建立的
         * 原点在左上角
         *
         * 而OpenGL我们的正投影矩阵指定的原点在左下角,
         * 所以外部app传入它们使用的地图坐标
         * 需要再校正y坐标得到OpenGL渲染使用的地图坐标
         */
        y = gles_map_screen_coord_y(gmr->map, y);
        gles_map_set_location(gmr->map, x, y);
    }
    s = pthread_mutex_unlock(&(gmr->mtx));
    assert(0 == s);
}

void                 
gles_map_render_clean_location  
(T gmr)
{
    assert(gmr);
    int s;
    s = pthread_mutex_lock(&(gmr->mtx));
    assert(0 == s);
    if(gmr->map){
        gles_map_clean_location(gmr->map);
    }
    s = pthread_mutex_unlock(&(gmr->mtx));
    assert(0 == s);
}

void                 
gles_map_render_anim_rotate_reset    
(T gmr)
{
    assert(gmr);
    int s;
    s = pthread_mutex_lock(&(gmr->mtx));
    assert(0 == s);

    if(gmr->map)
        gles_map_rotate_reset(gmr->map);

    s = pthread_mutex_unlock(&(gmr->mtx));
    assert(0 == s);
}

void                 
gles_map_render_anim_zoom_by         
(T gmr, float rate)
{
    assert(gmr);
    int s;
    s = pthread_mutex_lock(&(gmr->mtx));
    assert(0 == s);

    if(gmr->map)
        gles_map_zoom_by(gmr->map, rate);

    s = pthread_mutex_unlock(&(gmr->mtx));
    assert(0 == s);
}

void                 
gles_map_render_anim_to              
(T gmr, float x, float y)
{
    assert(gmr);
    int s;
    s = pthread_mutex_lock(&(gmr->mtx));
    assert(0 == s);

    if(gmr->map){
        /**
         * 由于地图是按屏幕绘图原点来建立的
         * 原点在左上角
         *
         * 而OpenGL我们的正投影矩阵指定的原点在左下角,
         * 所以外部app传入它们使用的地图坐标
         * 需要再校正y坐标得到OpenGL渲染使用的地图坐标
         */
        y = gles_map_screen_coord_y(gmr->map, y);
        gles_map_animate_to(gmr->map, x, y);
    }

    s = pthread_mutex_unlock(&(gmr->mtx));
    assert(0 == s);
}

void                 
gles_map_render_anim_center_to       
(T gmr, float x, float y)
{
    assert(gmr);
    int s;
    s = pthread_mutex_lock(&(gmr->mtx));
    assert(0 == s);

    if(gmr->map){
        /**
         * 由于地图是按屏幕绘图原点来建立的
         * 原点在左上角
         *
         * 而OpenGL我们的正投影矩阵指定的原点在左下角,
         * 所以外部app传入它们使用的地图坐标
         * 需要再校正y坐标得到OpenGL渲染使用的地图坐标
         */
        float adjy = gles_map_screen_coord_y(gmr->map, y);
        gles_map_center_to(gmr->map, x, adjy);

    }

    s = pthread_mutex_unlock(&(gmr->mtx));
    assert(0 == s);
}

static 
void 
_load_font_and_textures(T gmr)
{
    int end, i, len, s;
    char *path;

    s = pthread_mutex_lock(&(gmr->mtx));
    assert(0 == s);

    len = strlen(gmr->folder);
    path = ALLOC(len + 64);
    sprintf(path, "%s/font.ttf", gmr->folder);
    gmr->gft = gles_font_new(path, 60);
    gles_font_load(gmr->gft);

    for(i = 0; i < TEXTURE_COUNT; i++){
        sprintf(path, "%s/%d.png", gmr->folder, i);
        gmr->textures[i] = gles_texture_png_new(path);
        gles_texture_png_load(gmr->textures[i]);
    }

    FREE(path);

    s = pthread_mutex_unlock(&(gmr->mtx));
    assert(0 == s);
}

static
void 
_except_handler(const except_t *e,
                        const char *file,
                        const char *func,
                        const char *reason,
                        int line)
{
    
#ifdef ANDROID_IDRN
    log_e("\n*****************Exception*****************\n"
            "Abort for a uncaught exception type:%s\n"
                    "raised in %s at %s:%d\n"
                    "reason:%s"
           "\n*****************Exception*****************\n"
                    ,
                    e->type, 
                    func, file, line,
                    reason);
#else
    printf("\n*****************Exception*****************\n"
           "Abort for a uncaught exception type:%s\n"
           "raised in %s at %s:%d\n"
           "reason:%s"
           "\n*****************Exception*****************\n"
           ,
           e->type,
           func, file, line,
           reason);
#endif
    abort();
}

#undef TEXTURE_COUNT
