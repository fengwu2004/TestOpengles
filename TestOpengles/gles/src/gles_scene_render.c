#include<pthread.h>
#include<math.h>
#include"assert.h"
#include"except.h"
#include"mem.h"
#include"gles_scene_render.h"
#include"gles_3d_c_program.h"
#include"gles_25d_c_program.h"
#include"gles_25vec_layer.h"
#include"gles_scene_touchmgr.h"
#include "gles_matrix.h"
#include <string.h>

//#include"error_functions.h"

#define EVENT_MOVE_FONT         0
#define EVENT_MOVE_LEFT         1
#define EVENT_MOVE_RIGHT        2
#define EVENT_MOVE_BACK         3
#define EVENT_MOVE_UP           4
#define EVENT_MOVE_DOWN         5

#define EVENT_AZIMUTH_LEFT      6
#define EVENT_AZIMUTH_RIGHT     7
#define EVENT_PITCH_UP          8
#define EVENT_PITCH_DOWN        9
#define EVENT_ROLL_LEFT         10
#define EVENT_ROLL_RIGHT        11



#define T gles_scene_render_t

struct T{

    gles_scene_touchmgr_t   tmgr;
    pthread_mutex_t         mtx;
    gles_3d_c_program_t     g3cp;        //颜色着色程序
    gles_25d_c_program_t    g25cp;       //颜色着色程序
    gles_25vec_layer_t      gvl;
    float                   pmat[16];   //投影矩阵
    float                   vmat[16];   //视图矩阵
    float                   wmat[16];   //世界矩阵
    float                   width;      //GLSurface的尺寸
    float                   height;
    struct background{
        float               red;
        float               green;
        float               blue;
    }                   color;
};

static void _except_handler(const except_t *e,
                        const char *file,
                        const char *func,
                        const char *reason,
                        int line);
static GLfloat _cube_0[] = {
                            -10.0f,     10.0f,      -10.0f,
                            10.0f,      10.0f,      -10.0f,
                            10.0f,      10.0f,      10.0f,
                            -10.0f,     10.0f,      10.0f,

                            -10.0f,     -10.0f,     -10.0f,
                            10.0f,      -10.0f,     -10.0f,
                            10.0f,      -10.0f,     10.0f,
                            -10.0f,     -10.0f,     10.0f};


static GLfloat _color[] = {
                            1.0f,       0.0f,       0.0f,
                            1.0f,       0.5f,       0.0f,
                            1.0f,       1.0f,       0.0f,
                            0.0f,       1.0f,       0.0f,

                            0.0f,       0.5f,       1.0f,
                            0.0f,       0.0f,       1.0f,
                            0.0f,       1.0f,       1.0f,
                            1.0f,       1.0f,       1.0f};

static GLushort _index_top_0[] = {0, 1, 2, 3};
static GLushort _index_top_1[] = {4, 5, 6, 7};
static GLushort _index_edges[] = {0, 4,
                                  1, 5,
                                  2, 6,
                                  3, 7};

static float    _objmat_0[16];
static float    _objmat_1[16];

static void 
_gles_matrix_translate_4
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

static
void
_gles_matrix_perspective
(float *m, float fovy, float aspect, float znear, float zfar)
{
    float f = 1.0f / (float) tan(fovy * (M_PI / 360.0));
    float range_reciprocal = 1.0f / (znear - zfar);

    m[0] = f / aspect;
    m[1] = 0.0f;
    m[2] = 0.0f;
    m[3] = 0.0f;

    m[4] = 0.0f;
    m[5] = f;
    m[6] = 0.0f;
    m[7] = 0.0f;

    m[8] = 0.0f;
    m[9] = 0.0f;
    m[10] = (zfar + znear) * range_reciprocal;
    m[11] = -1.0f;

    m[12] = 0.0f;
    m[13] = 0.0f;
    m[14] = 2.0f * zfar * znear * range_reciprocal;
    m[15] = 0.0f;
}

static
void _gles_matrix_view
(float *m, 
double eyex, double eyey, double eyez,
double centerx, double centery, double centerz,
double upx, double upy, double upz)
{
    float fx = centerx - eyex;
    float fy = centery - eyey;
    float fz = centerz - eyez;

    // Normalize f
    float rlf = 1.0f / sqrt(fx*fx + fy*fy + fz*fz);
    fx *= rlf;
    fy *= rlf;
    fz *= rlf;

    // compute s = f x up (x means "cross product")
    float sx = fy * upz - fz * upy;
    float sy = fz * upx - fx * upz;
    float sz = fx * upy - fy * upx;

    // and normalize s
    float rls = 1.0f / sqrt(sx*sx + sy*sy + sz*sz);
    sx *= rls;
    sy *= rls;
    sz *= rls;

    // compute u = s x f
    float ux = sy * fz - sz * fy;
    float uy = sz * fx - sx * fz;
    float uz = sx * fy - sy * fx;

    m[0] = sx;
    m[1] = ux;
    m[2] = -fx;
    m[3] = 0.0f;

    m[4] = sy;
    m[5] = uy;
    m[6] = -fy;
    m[7] = 0.0f;

    m[8] = sz;
    m[9] = uz;
    m[10] = -fz;
    m[11] = 0.0f;

    m[12] = 0.0f;
    m[13] = 0.0f;
    m[14] = 0.0f;
    m[15] = 1.0f;

    float tm[16];
    _gles_matrix_translate_4(tm, -eyex, -eyey, -eyez);
    gles_matrix_mult_matrix_4(tm, m);
    memcpy(m, tm, sizeof(float)*16);
}

static
void
_gles_matrix_rotate3d
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

T    
gles_scene_render_new
()
{

    T gsr;

    gsr = ALLOC(sizeof(*gsr));

    pthread_mutex_init(&(gsr->mtx), NULL);

    gsr->tmgr = gles_scene_touchmgr_new(gsr);


    gsr->color.red  = 0.8f;
    gsr->color.green= 0.8f;
    gsr->color.blue = 0.8f;

    except_set_default_handler(_except_handler);

    return gsr;
}

void 
gles_scene_render_free
(T *gsr)
{
    assert(gsr);
    assert(*gsr);

    int s;
    pthread_mutex_t     mtx = (*gsr)->mtx;
    s = pthread_mutex_lock(&mtx);
    assert(0 == s);
    gles_3d_c_program_free(&((*gsr)->g3cp));
    gles_25d_c_program_free(&((*gsr)->g25cp));

    gles_25vec_layer_free(&((*gsr)->gvl));
    
    gles_scene_touchmgr_free(&((*gsr)->tmgr));

    FREE(gsr);
    s = pthread_mutex_unlock(&mtx);
    assert(0 == s);
    pthread_mutex_destroy(&mtx);
}

void                 
gles_scene_render_init            
(T gsr)
{
    assert(gsr);
    gsr->g3cp   = gles_3d_c_program_new();
    gsr->g25cp  = gles_25d_c_program_new();
    gsr->gvl    = gles_25vec_layer_new("/storage/emulated/0/Android/data/com.indoorun.idrndemo/files/map/svg6/LE.vbf");

    int height, width;
    width = gles_25vec_layer_width(gsr->gvl);
    height = gles_25vec_layer_height(gsr->gvl);

    gles_25d_c_program_set_height(gsr->g25cp, height);
    _gles_matrix_translate_4(gsr->wmat, 
            width/2, 
            -7000, 
            2.5*height
            );

    float tm[16];
            _gles_matrix_rotate3d(tm, -45, 1, 0, 0);
            gles_matrix_mult_matrix_4(tm, gsr->wmat);
            memcpy(gsr->wmat, tm, sizeof(float)*16);
}

void 
gles_scene_render_resize
(T gsr, int width, int height)
{
    assert(gsr);
    int s;
    s = pthread_mutex_lock(&(gsr->mtx));
    assert(0 == s);


    gsr->width = width;
    gsr->height= height;
    _gles_matrix_perspective(gsr->pmat, 45, (float)width/(float)height, 100, 100000);
    _gles_matrix_view(gsr->vmat, 0, 0, 0, //摄像机坐标原点和投影坐标原点重合
                                0, 0, 100, //看向z轴正方向
                                0, 1, 0);//摄像机正上方指定为投影坐标的y轴正方向
    _gles_matrix_translate_4(_objmat_0, 0, 1000, 1000);
    _gles_matrix_translate_4(_objmat_1, 0, 2000, 2000);


    //gles_matrix_identity_4(gsr->wmat, 0);

    glViewport(0, 0, width, height);

    gles_3d_c_program_set_matrix_projection(gsr->g3cp, gsr->pmat);
    gles_3d_c_program_set_matrix_view(gsr->g3cp, gsr->vmat);

    gles_25d_c_program_set_matrix_projection(gsr->g25cp, gsr->pmat);
    gles_25d_c_program_set_matrix_view(gsr->g25cp, gsr->vmat);


    s = pthread_mutex_unlock(&(gsr->mtx));
    assert(0 == s);
}

void 
gles_scene_render_render
(T gsr)
{
    int s;
    s = pthread_mutex_lock(&(gsr->mtx));
    assert(0 == s);

    glClear(GL_COLOR_BUFFER_BIT);
    glClearColor(gsr->color.red, gsr->color.green, gsr->color.blue, 1.0f);
    glEnable(GL_CULL_FACE);

    gles_3d_c_program_set_matrix_world(gsr->g3cp, gsr->wmat);
    gles_25d_c_program_set_matrix_world(gsr->g25cp, gsr->wmat);

    gles_25vec_layer_render(gsr->gvl, gsr->g25cp);

    gles_25d_c_program_set_matrix_object(gsr->g25cp, _objmat_0);
    gles_25vec_layer_render(gsr->gvl, gsr->g25cp);
    gles_25d_c_program_set_matrix_object(gsr->g25cp, _objmat_1);
    gles_25vec_layer_render(gsr->gvl, gsr->g25cp);

    /*
    gles_3d_c_program_set_matrix_object(gsr->g3cp, _objmat_0);
    gles_3d_c_program_draw_elements(gsr->g3cp, GL_LINE_LOOP, _cube_0, _color, _index_top_0, 4);
    gles_3d_c_program_draw_elements(gsr->g3cp, GL_LINE_LOOP, _cube_0, _color, _index_top_1, 4);
    gles_3d_c_program_draw_elements(gsr->g3cp, GL_LINES, _cube_0, _color, _index_edges, 8);

    gles_3d_c_program_set_matrix_object(gsr->g3cp, _objmat_1);
    gles_3d_c_program_draw_elements(gsr->g3cp, GL_LINE_LOOP, _cube_0, _color, _index_top_0, 4);
    gles_3d_c_program_draw_elements(gsr->g3cp, GL_LINE_LOOP, _cube_0, _color, _index_top_1, 4);
    gles_3d_c_program_draw_elements(gsr->g3cp, GL_LINES, _cube_0, _color, _index_edges, 8);*/


    glDisable(GL_CULL_FACE);

    s = pthread_mutex_unlock(&(gsr->mtx));
    assert(0 == s);
}

int          
gles_scene_render_ontouch
(T gsr, int action, float x0, float y0, float x1, float y1)
{
    int retval, s;
    assert(gsr);

    s = pthread_mutex_lock(&(gsr->mtx));
    assert(0 == s);


    retval = gles_scene_touchmgr_ontouch(gsr->tmgr, action, x0, y0, x1, y1);

    s = pthread_mutex_unlock(&(gsr->mtx));
    assert(0 == s);
    return retval;
}

void                 
gles_scene_render_set_world_matrix       
(T gsr, GLfloat *wmat)
{
    memcpy(gsr->wmat, wmat, sizeof(GLfloat)*16);
}
const GLfloat *
gles_scene_render_get_world_matrix       
(T gsr)
{
    return gsr->wmat;
}

void                 
gles_scene_render_event           
(T gsr, int event, float x, float y)
{
    int s;
    float tm[16];
    float step = 32;
    assert(gsr);

    s = pthread_mutex_lock(&(gsr->mtx));
    assert(0 == s);

    switch(event){
        /**
         * 因为是投影和视图坐标系不动, 转动世界坐标系, 所以
         * 都是反着的
         */
        case EVENT_MOVE_FONT:
            _gles_matrix_translate_4(tm, 0, 0, -step);
            gles_matrix_mult_matrix_4(tm, gsr->wmat);
            memcpy(gsr->wmat, tm, sizeof(float)*16);
        break;
        case EVENT_MOVE_BACK:
            _gles_matrix_translate_4(tm, 0, 0, step);
            gles_matrix_mult_matrix_4(tm, gsr->wmat);
            memcpy(gsr->wmat, tm, sizeof(float)*16);
        break;
        case EVENT_MOVE_LEFT:
            _gles_matrix_translate_4(tm, -step, 0, 0);
            gles_matrix_mult_matrix_4(tm, gsr->wmat);
            memcpy(gsr->wmat, tm, sizeof(float)*16);
        break;
        case EVENT_MOVE_RIGHT:
            _gles_matrix_translate_4(tm, step, 0, 0);
            gles_matrix_mult_matrix_4(tm, gsr->wmat);
            memcpy(gsr->wmat, tm, sizeof(float)*16);
        break;
        case EVENT_MOVE_UP:
            _gles_matrix_translate_4(tm, 0, -step, 0);
            gles_matrix_mult_matrix_4(tm, gsr->wmat);
            memcpy(gsr->wmat, tm, sizeof(float)*16);
        break;
        case EVENT_MOVE_DOWN:
            _gles_matrix_translate_4(tm, 0, step, 0);
            gles_matrix_mult_matrix_4(tm, gsr->wmat);
            memcpy(gsr->wmat, tm, sizeof(float)*16);
        break;

        case EVENT_AZIMUTH_LEFT:
            _gles_matrix_rotate3d(tm, -2, 0, 1, 0);
            gles_matrix_mult_matrix_4(tm, gsr->wmat);
            memcpy(gsr->wmat, tm, sizeof(float)*16);
        break;
        case EVENT_AZIMUTH_RIGHT:
            _gles_matrix_rotate3d(tm, 2, 0, 1, 0);
            gles_matrix_mult_matrix_4(tm, gsr->wmat);
            memcpy(gsr->wmat, tm, sizeof(float)*16);
        break;
        break;

        case EVENT_PITCH_UP:
            _gles_matrix_rotate3d(tm, -2, 1, 0, 0);
            gles_matrix_mult_matrix_4(tm, gsr->wmat);
            memcpy(gsr->wmat, tm, sizeof(float)*16);
        break;
        case EVENT_PITCH_DOWN:
            _gles_matrix_rotate3d(tm, 2, 1, 0, 0);
            gles_matrix_mult_matrix_4(tm, gsr->wmat);
            memcpy(gsr->wmat, tm, sizeof(float)*16);
        break;

        case EVENT_ROLL_LEFT:
            _gles_matrix_rotate3d(tm, -2, 0, 0, 1);
            gles_matrix_mult_matrix_4(tm, gsr->wmat);
            memcpy(gsr->wmat, tm, sizeof(float)*16);
        break;
        case EVENT_ROLL_RIGHT:
            _gles_matrix_rotate3d(tm, 2, 0, 0, 1);
            gles_matrix_mult_matrix_4(tm, gsr->wmat);
            memcpy(gsr->wmat, tm, sizeof(float)*16);
        break;
    }

    s = pthread_mutex_unlock(&(gsr->mtx));
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
#endif
    abort();
}

#undef TEXTURE_COUNT
