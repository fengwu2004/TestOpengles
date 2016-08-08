#include"mem.h"
#include"assert.h"
#include"mover.h"

#define DEFAULT_MASS        1
#define DEFAULT_MAX_FORCE   4
#define DEFAULT_MAX_SPEED   4
#define T mover_t

struct T{
    float mass;
    float maxforce;
    float maxspeed;
    struct vec2 location;
    struct vec2 velocity;
    struct vec2 acceleration;
};

/**
 * 映射函数
 * 输入一个value
 * 它的
 * 定义域 是istart-->istop
 * 值域   是ostart-->ostop
 */
static double _map(double value,
          double istart, double istop,
          double ostart, double ostop){
    return ostart + (ostop - ostart) * ((value - istart) / (istop - istart));
}

inline 
T            
mover_new_def
(float x, float y)
{
    return mover_new(x, y, DEFAULT_MAX_SPEED);
}

T            
mover_new
(float x, float y, float maxspeed)
{
    T mv;
    mv = ALLOC(sizeof(*mv));
    mv->mass        = DEFAULT_MASS;
    mv->maxforce    = DEFAULT_MAX_FORCE;
    mv->maxspeed    = DEFAULT_MAX_SPEED;
    
    mv->location.x  = x;
    mv->location.y  = y;
    mv->velocity.x  = 0;
    mv->velocity.y  = 0;
    mv->acceleration.x = 0;
    mv->acceleration.y = 0;

    return mv;
}


void
mover_set_maxspeed
(T mv, float maxspeed)
{
    assert(mv);
    mv->maxspeed = maxspeed;
}
float        
mover_get_maxspeed
(T mv)
{
    assert(mv);
    return mv->maxspeed;
}

void         
mover_set_location
(T mv, float x, float y)
{
    assert(mv);
    mv->location.x = x;
    mv->location.y = y;
}
struct vec2  
mover_get_location
(T mv)
{
    assert(mv);
    return mv->location;
}

void         
mover_update
(T mv)
{
    assert(mv);
    mv->velocity = vec2_add(mv->velocity, mv->acceleration);
    mv->velocity = vec2_limit(mv->velocity, mv->maxspeed);
    mv->location = vec2_add(mv->location, mv->velocity);

    mv->acceleration.x = 0;
    mv->acceleration.y = 0;
}

void         
mover_apply_force
(T mv, struct vec2 force)
{
    struct vec2 f;
    assert(mv);
    f = vec2_div(force, mv->mass);
    mv->acceleration = vec2_add(mv->acceleration, f);
}


void         
mover_seek
(T mv, struct vec2 target)
{
    struct vec2 desired;
    struct vec2 steer;
    assert(mv);

    desired = vec2_normalize(vec2_sub(target, mv->location));
    desired = vec2_mul(desired, mv->maxspeed);

    steer = vec2_limit(vec2_sub(desired, mv->velocity),
                        mv->maxforce);
    mover_apply_force(mv, steer);
}

void         
mover_flee(T mv, struct vec2 target)
{
    struct vec2 desired;
    struct vec2 steer;
    assert(mv);

    desired = vec2_normalize(vec2_sub(target, mv->location));
    desired = vec2_mul(desired, -mv->maxspeed);//和seek相比就是反向了一下

    steer = vec2_limit(vec2_sub(desired, mv->velocity),
                        mv->maxforce);
    mover_apply_force(mv, steer);
}

int          
mover_arrive
(T mv, struct vec2 target, float dist)
{
    struct vec2 desired;
    struct vec2 steer;
    double d, m;

    assert(mv);
    desired = vec2_sub(target, mv->location);
    d = vec2_norm(desired);
    if(d <= dist)
        return 1;

    desired = vec2_normalize(desired);
    if(d < 100){
        m = _map(d, 0, 100, 0, mv->maxspeed);
        desired = vec2_mul(desired, m);
    }else{
        desired = vec2_mul(desired, mv->maxspeed);
    }
    steer = vec2_sub(desired, mv->velocity);
    steer = vec2_limit(steer, mv->maxforce);

    mover_apply_force(mv, steer);
    return 0;
}
