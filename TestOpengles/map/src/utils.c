#include<math.h>
#include"assert.h"
#include"utils.h"

#define BE  0
#define LE  1
#define WTF 2

#define FLT_ABS_ERROR 1e-5
#define FLT_REL_ERROR 1e-8


/**
 * 区间映射
 */
double       
utils_math_map
(double value, double istart, double istop, double ostart, double ostop)
{
    return ostart + (ostop - ostart) * ((value - istart) / (istop - istart));
}

/**
 * 弧度度转换
 */
double       
utils_math_deg_2_radian
(double deg)
{
    return (deg * 2 * M_PI)/360.0;
}
double       
utils_math_radian_2_deg
(double radian)
{
    return 360.0 * radian / (2 * M_PI);
}

int          
utils_math_float_equals
(float x, float y)
{
    if(fabs(x-y) <= FLT_ABS_ERROR){
        return 1;
    }else {
        float r = (float)fabs(x) > (float)fabs(y) ?
            (float)fabs((x-y) / x) : (float)fabs((x-y) / y);
        return r <= FLT_REL_ERROR;
    }
}

int          
utils_math_float_equals_delta
(float x, float y, float delta)
{
    if(fabs(x-y) <= delta){
        return 1;
    }else{
        return 0;
    }
}

/**
 * 判断字节序 返回0为大端 1为小端 其他情况将会触发断言
 */
int          
utils_machine_byte_order()
{
    int byteoder;
    union
    {
        short value;
        char union_bytes[sizeof(short)];
    } test;

    test.value = 0x0102;
    if ((test.union_bytes[0] == 1) && (test.union_bytes[1] == 2)){
        byteoder    = BE;
    }else if((test.union_bytes[0] == 2) && (test.union_bytes[1] == 1)){
        byteoder    = LE;
    }else{
        byteoder    = WTF;
    }
    assert(WTF != byteoder);
    return byteoder;
}

#undef BE
#undef LE
#undef WTF
#undef FLT_ABS_ERROR
#undef FLT_REL_ERROR
