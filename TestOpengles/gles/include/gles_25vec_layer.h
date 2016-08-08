#ifndef GLES_25VEC_LAYER_INCLUDE
#define GLES_25VEC_LAYER_INCLUDE

#include"gles_25d_c_program.h"
#import <OpenGLES/ES3/gl.h>
#import <OpenGLES/ES3/glext.h>
#define T gles_25vec_layer_t

typedef struct T *T;

extern T            gles_25vec_layer_new(const char *vbfile);

extern void         gles_25vec_layer_free(T *gvl);

extern void         gles_25vec_layer_render(T gvl, gles_25d_c_program_t gcp);

extern int          gles_25vec_layer_width(T gvl);
extern int          gles_25vec_layer_height(T gvl);
extern int          gles_25vec_layer_azimuth(T gvl);

#undef T
#endif /*GLES_25VEC_LAYER_INCLUDE*/
