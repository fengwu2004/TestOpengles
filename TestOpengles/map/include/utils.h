#ifndef UTILS_INCLUDE
#define UTILS_INCLUDE

/**
 * 区间映射
 */
extern double       utils_math_map(double value, double istart, double istop, double ostart, double ostop);

/**
 * 弧度度转换
 */
extern double       utils_math_deg_2_radian(double deg);
extern double       utils_math_radian_2_deg(double radian);

extern int          utils_math_float_equals(float x, float y);

extern int          utils_math_float_equals_delta(float x, float y, float delta);

/**
 * 判断字节序 返回0为大端 1为小端 其他情况将会触发断言
 */
extern int          utils_machine_byte_order();

#endif /*UTILS_INCLUDE*/
