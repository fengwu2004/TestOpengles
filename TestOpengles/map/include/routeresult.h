#ifndef ROUTERESULT_INCLUDE
#define ROUTERESULT_INCLUDE



#define ROUTE_SUG_FRONT            0
#define ROUTE_SUG_LEFT             1
#define ROUTE_SUG_BACKWARD         2
#define ROUTE_SUG_RIGHT            3

#define ROUTE_NEXT_SUG_FRONT     0
#define ROUTE_NEXT_SUG_LEFT     1
#define ROUTE_NEXT_SUG_RIGHT    2
#define ROUTE_NEXT_SUG_ARRIVE   3

#define T routeresult_t


typedef struct T *T;


typedef struct route_seg_t{
    float   startx;
    float   starty;
    float   endx;
    float   endy;
    float   azimuth;
} *route_seg_t;

struct nav_status{
    float   projx;
    float   projy;
    float   errordist;
    float   pathdist;
    float   serialdist;
    float   azimuth;
    int     segindex;
    int     sug;
    int     nextsug;
    int     validate;
};

/**
 * 新建一个路径规划结果
 * 传入建筑物和路径规划产生的路径
 * 以及地图方位角
 */
extern T                    routeresult_new(float mapazimuth);

/**
 * 释放路径规划结果
 */
extern void                 routeresult_free(T *rr);

/**
 * 设置路径数据
 */
extern void                 routeresult_set_data(T rr, const float *pts, int cnt);

/**
 * 清除路径数据
 */
extern void                 routeresult_clean_data(T rr);

/**
 * 获取路段数目
 */
extern int                  routeresult_seg_count(T rr);


/**
 * 获取路段
 */
extern const route_seg_t    routeresult_get_seg(T rr, int index);


/**
 * 根据传入的位置和设备方向获取导航状态
 * 偏离路径的距离,
 * 方向修正
 */
extern  struct nav_status   routeresult_get_status(T rr, float x, float y, float azimuth);


#undef T
#endif /*ROUTERESULT_INCLUDE*/
