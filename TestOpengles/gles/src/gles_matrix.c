#include<string.h>
#include<math.h>
#include"assert.h"
#include"gles_matrix.h"

#ifdef ANDROID_IDRN
#include"error_functions.h"
#endif


void 
gles_matrix_identity_3
(GLfloat *m, int offset)
{
    int i;
    for (i=0 ; i< 9 ; i++) {
        m[offset + i] = 0;
    }
    for(i = 0; i < 9; i += 4) {
        m[offset + i] = 1.0f;
    }
}

void 
gles_matrix_dump_3
(const char *lable, float *mat)
{
#ifdef ANDROID_IDRN
    log_v("3X3 matrix %s:\n"
          "\t┌%.1f\t%.1f\t%.1f┐\n"
          "\t│%.1f\t%.1f\t%.1f│\n"
          "\t└%.1f\t%.1f\t%.1f┘\n",
            lable,
            mat[0], mat[3], mat[6],
            mat[1], mat[4], mat[7],
            mat[2], mat[5], mat[8]);
#endif 
}

void 
gles_matrix_dump_4
(const char *lable, float *mat)
{
#ifdef ANDROID_IDRN
    log_v("4X4 matrix %s:\n"
          "\t┌%.1f\t%.1f\t%.1f\t%.1f┐\n"
          "\t│%.1f\t%.1f\t%.1f\t%.1f│\n"
          "\t│%.1f\t%.1f\t%.1f\t%.1f│\n"
          "\t└%.1f\t%.1f\t%.1f\t%.1f┘\n",
            lable,
            mat[0], mat[4], mat[8], mat[12],
            mat[1], mat[5], mat[9], mat[13],
            mat[2], mat[6], mat[10], mat[14],
            mat[3], mat[7], mat[11], mat[15]);
#endif
}

void
gles_matrix_identity_4
(GLfloat *m, int offset)
{
    int i;
    for (i=0 ; i< 16 ; i++) {
        m[offset + i] = 0;
    }
    for(i = 0; i < 16; i += 5) {
        m[offset + i] = 1.0f;
    }
}

void 
gles_matrix_postranslate_4
(GLfloat *m, double x, double y, double z)
{
    m[12]= (float)x;
    m[13]= (float)y;
    m[14]= (float)z;
    m[15]= 1;
}


void 
gles_matrix_rotate2d_4
(GLfloat *m, double azimuth)
{
    float c, s;
    float radian;

    
    radian = (float)(azimuth * 2 * M_PI)/360.0;
    c = cos(radian);
    s = sin(radian);

    m[0] = c;
    m[1] = s;
    m[2] = 0;
    m[3] = 0;

    m[4] = -s;
    m[5] = c;
    m[6] = 0;
    m[7] = 0;

    m[8] = 0;
    m[9] = 0;
    m[10]= 1;
    m[11]= 0;

    m[12]= 0;
    m[13]= 0;
    m[14]= 0;
    m[15]= 1;
}

void
gles_matrix_ortho_4(float *m,
                     int left, int right, int bottom, int top,
                     int near, int far)
{
    assert(left != right);
    assert(bottom != top);
    assert(near != far);

    GLfloat r_width  = 1.0f / (right - left);
    GLfloat r_height = 1.0f / (top - bottom);
    GLfloat r_depth  = 1.0f / (far - near);
    GLfloat x =  2.0f * (r_width);
    GLfloat y =  2.0f * (r_height);
    GLfloat z = -2.0f * (r_depth);
    GLfloat tx = -(right + left) * r_width;
    GLfloat ty = -(top + bottom) * r_height;
    GLfloat tz = -(far + near) * r_depth;
    m[ 0] = x;
    m[ 5] = y;
    m[10] = z;
    m[12] = tx;
    m[13] = ty;
    m[14] = tz;
    m[15] = 1.0f;
    m[ 1] = 0.0f;
    m[ 2] = 0.0f;
    m[ 3] = 0.0f;
    m[ 4] = 0.0f;
    m[ 6] = 0.0f;
    m[ 7] = 0.0f;
    m[ 8] = 0.0f;
    m[ 9] = 0.0f;
    m[ 11] = 0.0f;
}

void gles_matrix_view
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
    gles_matrix_translate_4(tm, 0, -eyex, -eyey, -eyez);
    gles_matrix_mult_matrix_4(tm, m);
    memcpy(m, tm, sizeof(float)*16);
}
                        

void
gles_matrix_frustum
(float *m, int left, int right, int bottom, int top, int near, int far)
{
    assert(left != right);
    assert(top != bottom);
    assert(near != far);
    assert(near > 0);
    assert(far > 0);

    float r_width  = 1.0f / (right - left);
    float r_height = 1.0f / (top - bottom);
    float r_depth  = 1.0f / (near - far);
    float x = 2.0f * (near * r_width);
    float y = 2.0f * (near * r_height);
    float A = (right + left) * r_width;
    float B = (top + bottom) * r_height;
    float C = (far + near) * r_depth;
    float D = 2.0f * (far * near * r_depth);

    m[0] = x;
    m[5] = y;
    m[8] = A;
    m[9] = B;
    m[10] = C;
    m[14] = D;
    m[11] = -1.0f;
    m[1] = 0.0f;
    m[2] = 0.0f;
    m[3] = 0.0f;
    m[4] = 0.0f;
    m[6] = 0.0f;
    m[7] = 0.0f;
    m[12] = 0.0f;
    m[13] = 0.0f;
    m[15] = 0.0f;
}

void 
gles_matrix_translate_4
(GLfloat *m, int offset, double x, double y, double z)
{
    m[offset + 0] = 1.0f;
    m[offset + 1] = 0.0f;
    m[offset + 2] = 0.0f;
    m[offset + 3] = 0.0f;

    m[offset + 4] = 0.0f;
    m[offset + 5] = 1.0f;
    m[offset + 6] = 0.0f;
    m[offset + 7] = 0.0f;

    m[offset + 8] = 0.0f;
    m[offset + 9] = 0.0f;
    m[offset +10] = 1.0f;
    m[offset +11] = 0.0f;

    m[offset +12] = (float)x;
    m[offset +13] = (float)y;
    m[offset +14] = 0.0f;
    m[offset +15] = 1.0f;
}

void 
gles_matrix_mult_vec_4
(const GLfloat *m, float *v, int offset)
{
    float a, b, c, d;
    a = v[offset+0];
    b = v[offset+1];
    c = v[offset+2];
    d = v[offset+3];

    v[offset+0] = a*m[0] + b*m[4] + c*m[8] + d*m[12];
    v[offset+1] = a*m[1] + b*m[5] + c*m[9] + d*m[13];
    v[offset+2] = a*m[2] + b*m[6] + c*m[10]+ d*m[14];
    v[offset+3] = a*m[3] + b*m[7] + c*m[11]+ d*m[15];
}

/**
 * 暴力即是美
 */
void
gles_matrix_invert_4
(const GLfloat *src, GLfloat *dst)
{
    GLfloat
        a00 = src[0], a01 = src[1], a02 = src[2],  a03 = src[3],
        a10 = src[4], a11 = src[5], a12 = src[6],  a13 = src[7],
        a20 = src[8], a21 = src[9], a22 = src[10], a23 = src[11],
        a30 = src[12],a31 = src[13],a32 = src[14], a33 = src[15],

        b00 = a00 * a11 - a01 * a10,
        b01 = a00 * a12 - a02 * a10,
        b02 = a00 * a13 - a03 * a10,
        b03 = a01 * a12 - a02 * a11,
        b04 = a01 * a13 - a03 * a11,
        b05 = a02 * a13 - a03 * a12,
        b06 = a20 * a31 - a21 * a30,
        b07 = a20 * a32 - a22 * a30,
        b08 = a20 * a33 - a23 * a30,
        b09 = a21 * a32 - a22 * a31,
        b10 = a21 * a33 - a23 * a31,
        b11 = a22 * a33 - a23 * a32,

        det = b00 * b11 - b01 * b10 + b02 * b09 + b03 * b08 - b04 * b07 + b05 * b06;

    assert(0 != det);

    dst[0] = (a11 * b11 - a12 * b10 + a13 * b09) / det;
    dst[1] = (a02 * b10 - a01 * b11 - a03 * b09) / det;
    dst[2] = (a31 * b05 - a32 * b04 + a33 * b03) / det;
    dst[3] = (a22 * b04 - a21 * b05 - a23 * b03) / det;

    dst[4] = (a12 * b08 - a10 * b11 - a13 * b07) / det;
    dst[5] = (a00 * b11 - a02 * b08 + a03 * b07) / det;
    dst[6] = (a32 * b02 - a30 * b05 - a33 * b01) / det;
    dst[7] = (a20 * b05 - a22 * b02 + a23 * b01) / det;

    dst[8] = (a10 * b10 - a11 * b08 + a13 * b06) / det;
    dst[9] = (a01 * b08 - a00 * b10 - a03 * b06) / det;
    dst[10]= (a30 * b04 - a31 * b02 + a33 * b00) / det;
    dst[11]= (a21 * b02 - a20 * b04 - a23 * b00) / det;

    dst[12]= (a11 * b07 - a10 * b09 - a12 * b06) / det;
    dst[13]= (a00 * b09 - a01 * b07 + a02 * b06) / det;
    dst[14]= (a31 * b01 - a30 * b03 - a32 * b00) / det;
    dst[15]= (a20 * b03 - a21 * b01 + a22 * b00) / det;
}

void 
gles_matrix_mult_matrix_4
(GLfloat *left, const GLfloat *right)
{
    float a, b, c, d;
    int col;
    GLfloat m[16];

    memcpy(m, left, sizeof(GLfloat)*16);

    col = 0;
    for(col = 0; col < 4; col++){
        a = right[col*4+0];
        b = right[col*4+1];
        c = right[col*4+2];
        d = right[col*4+3];

        left[col*4+0] = a*m[0] + b*m[4] + c*m[8] + d*m[12];
        left[col*4+1] = a*m[1] + b*m[5] + c*m[9] + d*m[13];
        left[col*4+2] = a*m[2] + b*m[6] + c*m[10]+ d*m[14];
        left[col*4+3] = a*m[3] + b*m[7] + c*m[11]+ d*m[15];
    }
}

void 
gles_matrix_post_translate
(GLfloat *m, double tx, double ty)
{
    float   mt[16];
    gles_matrix_translate_4(mt, 0, tx, ty, 0);
    gles_matrix_mult_matrix_4(mt, m);
    memcpy(m, mt, sizeof(GLfloat)*16);
}


void     
gles_matrix_post_rotation
(GLfloat *m, double rotate, double x, double y)
{

    float   mt1[16];
    float   mr[16];
    float   mt2[16];


    gles_matrix_translate_4(mt1, 0, -x, -y, 0);
    gles_matrix_translate_4(mt2, 0, x, y, 0);
    gles_matrix_rotate2d_4(mr, rotate);
    
    gles_matrix_mult_matrix_4(mt1, m);
    gles_matrix_mult_matrix_4(mr, mt1);
    gles_matrix_mult_matrix_4(mt2, mr);

    memcpy(m, mt2, sizeof(GLfloat)*16);
}


void 
gles_matrix_post_scale
(GLfloat *m, double scale, double x, double y)
{
    float   mt1[16];
    float   ms[16];
    float   mt2[16];

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

    
    gles_matrix_mult_matrix_4(mt1, m);
    gles_matrix_mult_matrix_4(ms, mt1);
    gles_matrix_mult_matrix_4(mt2, ms);

    memcpy(m, mt2, sizeof(GLfloat)*16);
}
