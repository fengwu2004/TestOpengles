#include"assert.h"
#include"mem.h"
#include"envelop.h"

#define T envelop_t

struct T {
    int     allocbyarena;
    double  left;
    double  top;
    double  right;
    double  bottom;
};

T       
envelop_new
(arena_t arena, double left, double top, double right, double bottom)
{
    T e;

    assert(arena);
    e = (T)ARENA_ALLOC(arena, sizeof(*e));
    e->allocbyarena = 1;
    e->left  = left;
    e->top   = top;
    e->right = right;
    e->bottom= bottom;
    return e;
}


T       
envelop_new_point
(double x, double y)
{
    T e;

    NEW0(e);
    e->left     = x;
    e->top      = y;
    e->right    = x;
    e->bottom   = y;
    return e;
}


T       
envelop_new_box
(double left, double top, double right, double bottom)
{
    T e;

    NEW0(e);
    e->left     = left;
    e->top      = top;
    e->right    = right;
    e->bottom   = bottom;
    return e;
}


void       
envelop_free
(T *e)
{
    assert(e && *e);
    assert((*e)->allocbyarena == 0);

    FREE(*e);
}


T
envelop_clone_point
(T e, double x, double y)
{
    assert(e);
    e->left     = x;
    e->top      = y;
    e->right    = x;
    e->bottom   = y;
    return e;
}


int     
envelop_contains_point
(T e, double x, double y)
{
    assert(e);
    if(x >= e->left && 
            x <= e->right &&
            y >= e->top &&
            y <= e->bottom)
        return 1;
    else
        return 0;
}


int     
envelop_contains
(T e, T target)
{
    assert(e);
    assert(target);

    if(target->left >= e->left &&
            target->top >= e->top &&
            target->right <= e->right &&
            target->bottom <= e->bottom)
        return 1;
    else
        return 0;
}


double  
envelop_distance_sq
(T e, T target)
{
    T mostleft, mostright, upper, lower;
    double xdiff, ydiff;

    assert(e);
    assert(target);
    if(envelop_intersect(e, target))
            return 0;

    mostleft    = e->left   < target->left ? e : target;
    mostright   = e->right  > target->right ? e : target;
    upper       = e->top    < target->top   ? e : target;
    lower       = e->bottom    > target->bottom   ? e : target;
    
    xdiff = mostleft->left == mostright->left ? 0 : mostright->left - mostleft->right;
    if(xdiff < 0)
        xdiff *= -1;

    ydiff = upper->top == lower->top ? 0 : lower->top - upper->bottom;
    if(ydiff < 0)
        ydiff *= -1;

    return xdiff * xdiff + ydiff * ydiff;
}


int     
envelop_intersect
(T e, T target)
{
    assert(target);
    return envelop_intersect_box(e, target->left, target->top,
            target->right, target->bottom);
}


int     
envelop_intersect_box
(T e, double left, double top, double right, double bottom)
{
    assert(e);

    if(!(left > e->right ||
            right < e->left ||
            top > e->bottom ||
            bottom < e->top))
        return 1;
    else 
        return 0;
}


inline
double  
envelop_left
(T e)
{
    assert(e);
    return e->left;
}

inline
double  
envelop_top
(T e)
{
    assert(e);
    return e->top;
}


inline
double  
envelop_right
(T e)
{
    assert(e);
    return e->right;
}


inline
double  
envelop_bottom
(T e)
{
    assert(e);
    return e->bottom;
}

inline 
double  
envelop_width
(T e)
{
    assert(e);
    return e->right - e->left;
}


inline
double  
envelop_height
(T e)
{
    assert(e);
    return e->bottom - e->top;
}


inline
double  
envelop_center_x
(T e)
{
    assert(e);
    return (e->left + e->right) / 2.0;
}


inline
double  
envelop_center_y
(T e)
{
    assert(e);
    return (e->top + e->bottom) / 2.0;
}
