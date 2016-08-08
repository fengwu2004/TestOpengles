#ifndef VEC2_INCLUDE
#define VEC2_INCLUDE


/**
 * 为了简化2维矢量运算 弄个简单的实现
 */


struct vec2{
    double x;
    double y;
};

/**
 * 创建
 */
extern struct vec2 vec2_new(double x, double y);

/**
 * 矢量加 减 数乘
 */
extern struct vec2 vec2_add(struct vec2 a, struct vec2 b);
extern struct vec2 vec2_sub(struct vec2 a, struct vec2 b);
extern struct vec2 vec2_mul(struct vec2 a, double factor);
extern struct vec2 vec2_div(struct vec2 a, double divisor);


/**
 * 归一化
 */
extern struct vec2 vec2_normalize(struct vec2 v);

/**
 * 內积
 */
extern double       vec2_dot_prod(struct vec2 a, struct vec2 b);

/**
 * 范数的平方
 */
extern double       vec2_norm_sq(struct vec2 a);

/**
 * 范数
 */
extern double       vec2_norm(struct vec2 a);

/**
 * 将向量的长度限制在max之内
 */
extern struct vec2  vec2_limit(struct vec2 v, double max);

/**
 * 将p正交投影到a和b代表的线段
 *
 */
extern struct vec2  vec2_normal_projection(struct vec2 p, struct vec2 a, struct vec2 b);


/**
 * a b所代表的线段的长度的平方
 */
extern double       vec2_dist_sq(struct vec2 a, struct vec2 b);

extern double       vec2_dist(struct vec2 a, struct vec2 b);

/**
 * 计算中点
 */
extern struct vec2  vec2_mid(struct vec2 a, struct vec2 b);

/**
 * 计算斜率
 */
extern double       vec2_slope(struct vec2 from, struct vec2 to);


/**
 * 向量旋转
 */
extern struct vec2  vec2_rotate(struct vec2 v, float degree);


/**
 * 线性插值
 */
extern double       vec2_lerp_value(double start, double stop, double amt);
extern struct vec2  vec2_lerp_vec(struct vec2 v1, struct vec2 v2, double amt);
#endif /*VEC2_INCLUDE*/
