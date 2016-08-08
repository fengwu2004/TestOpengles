#ifndef GLES_MATRIX_INCLUDE
#define GLES_MATRIX_INCLUDE

#import <OpenGLES/ES3/gl.h>
#import <OpenGLES/ES3/glext.h>

extern void gles_matrix_identity_3(GLfloat *m, int offset);

extern void gles_matrix_dump_3(const char *lable, float *mat);

extern void gles_matrix_dump_4(const char *lable, float *mat);



/**
 * 构造一个4阶单位矩阵
 */
extern void gles_matrix_identity_4(GLfloat *m, int offset);

/**
 * 此处有毒, 一旦第一个参数是float指针
 * 后面的float参数全部就像没压栈一样,是乱的
 * 用int, 用double都正常
 * 怀疑是交叉编译器的问题
 */
extern void gles_matrix_postranslate_4(GLfloat *m, double x, double y, double z);


/**
 * 此处有毒, 一旦第一个参数是float指针
 * 后面的float参数全部就像没压栈一样,是乱的
 * 用int, 用double都正常
 * 怀疑是交叉编译器的问题
 *
 *
 * 但是
   gles_matrix_ortho_4(GLfloat *m, int offset,
   为什么就没问题, 现在没时间弄清楚了

    终于    
   gles_matrix_ortho_4(GLfloat *m, int offset,
   也挂了, 改了下参数表好了, 这个问题很烦人
   找时间集中解决
 */
extern void gles_matrix_rotate2d_4(GLfloat *m, double azimuth);

/**
 * 构建正投影矩阵
 */
extern void gles_matrix_ortho_4(float *m,
                     int left, int right, int bottom, int top,
                     int near, int far);

/**
 * 构建视图矩阵
 */
extern void gles_matrix_view(float *m, 
                        double eyex, double eyey, double eyez,
                        double centerx, double centery, double centerz,
                        double upx, double upy, double upz);
/**
 * 构建透视投影矩阵
 */
extern void gles_matrix_frustum(float *m, int left, int right, int bottom, int top, int near, int far);

/**
 * 构建平移变换矩阵
 */
extern void gles_matrix_translate_4(GLfloat *m, int offset,
                     double x, double y, double z);


/**
 * 矩阵乘向量
 */
extern void gles_matrix_mult_vec_4(const GLfloat *m, float *v, int offset);

/**
 * 矩阵求逆, 如果不可逆直接触发断言
 */
extern void gles_matrix_invert_4(const GLfloat *src, GLfloat *dst);

/**
 * 矩阵乘矩阵
 */
extern void gles_matrix_mult_matrix_4(GLfloat *left, const GLfloat *right);

/**
 * 附加平移变换
 */
extern void gles_matrix_post_translate(GLfloat *m, double tx, double ty);

/**
 * 附加旋转变换
 */
extern void gles_matrix_post_rotation(GLfloat *m, double rotate, double x, double y);

/**
 * 附加缩放变换
 */
extern void gles_matrix_post_scale(GLfloat *m, double scale, double x, double y);

#endif /*GLES_MATRIX_INCLUDE*/
