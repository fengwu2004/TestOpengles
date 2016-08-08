#include<stdlib.h>
#include"assert.h"
#include"quadtree.h"
#include"sparsearray.h"

#define T quadtree_t

struct T{
    int             maxdepth;
    int             degenerate;
    quadnode_t      root;
    sparsearray_t   objlist;
};


T           
quadtree_new
(arena_t arena, double left, double top, double right, double bottom)
{
    T qt;

    assert(arena);

    qt = (T)ARENA_ALLOC(arena, sizeof(*qt));
    qt->root = quadnode_new(arena, 0, left, top, right, bottom);
    qt->objlist = sparsearray_new(1024);
    qt->degenerate = 0;
    qt->maxdepth = 0;
    return qt;
}


T           
quadtree_new_space
(arena_t arena, envelop_t extent)
{
    T qt;

    assert(arena);

    qt = (T)ARENA_ALLOC(arena, sizeof(*qt));
    qt->root = quadnode_new_space(arena, 0, extent);
    qt->objlist = sparsearray_new(1024);
    qt->degenerate = 0;
    return qt;
}


void        
quadtree_free
(T qt)
{
    assert(qt);

    sparsearray_free(&(qt->objlist));
}

void         
quadtree_degenerate(T qt)
{
    assert(qt);
    qt->degenerate = 1;
}

int         
quadtree_get_count
(T qt)
{

    assert(qt);
    return sparsearray_length(qt->objlist);
}


void *
quadtree_get_by_index
(T qt, int index)
{
    int len;
    assert(qt);
    assert(index >= 0 && index < sparsearray_length(qt->objlist));
    return sparsearray_get_at(qt->objlist, index);
}

void *
quadtree_get
(T qt, int id)
{
    assert(qt);
    assert(id >= 0);
    return sparsearray_get(qt->objlist, id);
}

int          
quadtree_depth
(T qt)
{
    assert(qt);
    return qt->maxdepth;
}

void        
quadtree_insert
(T qt, int id, void *obj, double left, double top, double right, double bottom)
{
    assert(qt);
    int depth;
    sparsearray_put(qt->objlist, id, obj);

    /**
     * 如果空间树退化就不插入到其中
     */
    if(!qt->degenerate){
        depth = quadnode_insert(qt->root, obj, left, top ,right, bottom);

        /**
         * 更新空间树的深度
         */
        if(depth > qt->maxdepth){
            qt->maxdepth = depth;
        }
    }
}

int         
quadtree_fast_contains_check
(T qt, double x, double y)
{
    assert(qt);

    return quadnode_fast_contains_check(qt->root, x, y);
}

int         
quadtree_fast_intersect_check
(T qt, double left, double top, double right, double bottom)
{
    int retval;
    envelop_t e;

    assert(qt);
    e = envelop_new_box(left, top, right, bottom);
    retval = quadtree_fast_intersect_check_space(qt, e);

    envelop_free(&e);

    return retval;
}

int         
quadtree_fast_intersect_check_space
(T qt, envelop_t e)
{
    assert(qt);
    assert(e);

    return quadnode_fast_intersect_check(qt->root, e);
}

seq_t       
quadtree_search
(T qt, double left, double top, double right, double bottom, 
    double maxdistance, int maxcount, seq_t retlist){

    seq_t retval;
    envelop_t e;

    assert(qt);
    e = envelop_new_box(left, top, right, bottom);
    retval = quadtree_search_space(qt, e, maxdistance, maxcount, retlist);
    envelop_free(&e);

    return retval;
}

seq_t       
quadtree_search_space
(T qt, envelop_t searchbox, double maxdistance, int maxcount, seq_t retlist)
{
    assert(qt);
    assert(searchbox);
    
    if(NULL == retlist){
        retlist = seq_new(32);
    }
    
    quadnode_search(qt->root, searchbox, maxdistance, maxcount, retlist);

    return retlist;
}


void        
quadtree_render
(T qt,
 double left, double top, double right, double bottom,
void (*render)(void *obj, envelop_t e, void *cl), 
void *cl)
{

    envelop_t e;
    e = envelop_new_box(left, top, right, bottom);
    quadnode_render(qt->root, e, render, cl);
    envelop_free(&e);
}

void        
quadtree_render_depth
(T qt, int depth,
 double left, double top, double right, double bottom,
void (*render)(void *obj, envelop_t e, void *cl), 
void *cl)
{

    envelop_t e;
    e = envelop_new_box(left, top, right, bottom);
    quadnode_render_depth(qt->root, depth, e, render, cl);
    envelop_free(&e);
}

