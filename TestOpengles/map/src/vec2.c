#include<math.h>
#include "assert.h"
#include "vec2.h"

struct vec2 
vec2_new
(double x, double y)
{
    struct vec2 retval;
    retval.x = x;
    retval.y = y;
    return retval;
}

struct vec2 
vec2_add
(struct vec2 a, struct vec2 b)
{
    struct vec2 retval;
    retval.x = a.x + b.x;
    retval.y = a.y + b.y;
    return retval;
}

struct vec2 
vec2_sub
(struct vec2 a, struct vec2 b)
{
    struct vec2 retval;
    retval.x = a.x - b.x;
    retval.y = a.y - b.y;
    return retval;
}

struct vec2 
vec2_mul
(struct vec2 a, double factor)
{
    struct vec2 retval;
    retval.x = a.x * factor;
    retval.y = a.y * factor;
    return retval;
}

struct vec2 
vec2_div(struct vec2 a, double divisor)
{
    struct vec2 retval;
    retval.x = a.x / divisor;
    retval.y = a.y / divisor;
    return retval;
}

struct vec2 
vec2_normalize
(struct vec2 v)
{
    struct vec2 retval;
    double m = sqrt(v.x*v.x + v.y*v.y);
    if (m > 0) {
        retval.x = v.x/m;
        retval.y = v.y/m;
    } else {
        retval.x = v.x;
        retval.y = v.y;
    }
    return retval;
}

double       
vec2_dot_prod
(struct vec2 a, struct vec2 b)
{
   return a.x * b.x + a.y * b.y;
}

double       
vec2_norm_sq
(struct vec2 a)
{
    return a.x * a.x + a.y * a.y;
}
double       
vec2_norm
(struct vec2 a)
{
    return sqrt(a.x * a.x + a.y * a.y);
}

struct vec2  
vec2_limit(struct vec2 v, double max)
{
    if (vec2_norm_sq(v) > max*max) {
        return vec2_mul(
                    vec2_normalize(v),
                    max);
    }else{
        return v;
    }
}

struct vec2             
vec2_normal_projection
(struct vec2 p, struct vec2 a, struct vec2 b)
{
    double r;
    struct vec2 ap;
    struct vec2 ab;
    
    ap  = vec2_sub(p, a);
    ab  = vec2_sub(b, a);
    r   = vec2_dot_prod(ap, ab) / vec2_norm_sq(ab);

    if(r <= 0){
        return a;
    }else if(r >= 1){
        return b;
    }else{
        return vec2_add(a, vec2_mul(ab, r)); 
    }
}

double                  
vec2_dist_sq(struct vec2 a, struct vec2 b)
{
    double dx, dy, retval;
    dx = a.x - b.x;
    dy = a.y - b.y;
    retval = dx * dx + dy * dy;
    if(retval < 0){
        retval = -retval;
    }
    return retval;
}


double       
vec2_dist
(struct vec2 a, struct vec2 b)
{
    double dx, dy, retval;
    dx = a.x - b.x;
    dy = a.y - b.y;
    retval = dx * dx + dy * dy;
    return sqrt(retval);
}

struct vec2  
vec2_mid
(struct vec2 a, struct vec2 b)
{
    struct vec2 retval;
    retval.x = (a.x + b.x) / 2.0;
    retval.y = (a.y + b.y) / 2.0;
    return retval;
}

double       
vec2_slope
(struct vec2 from, struct vec2 to)
{
    return (to.y - from.y) / (to.x - from.x);
}

struct vec2  
vec2_rotate
(struct vec2 v, float degree)
{
    float temp = v.x;
    float theta = (degree * 2 * M_PI)/360.0f;
    v.x = v.x*cos(theta) - v.y*sin(theta);
    v.y = temp*sin(theta) + v.y*cos(theta);
    return v;
}


double
vec2_lerp_value
(double start, double stop, double amt)
{
    return start + (stop-start) * amt;
}

struct vec2  
vec2_lerp_vec
(struct vec2 v1, struct vec2 v2, double amt)
{
    v1.x = vec2_lerp_value(v1.x, v2.x, amt);
    v1.y = vec2_lerp_value(v1.y, v2.y, amt);

    return v1;
}
