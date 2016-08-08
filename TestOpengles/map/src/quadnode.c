#include"stdlib.h"
#include"assert.h"
#include"arena.h"
#include"seq.h"
#include"envelop.h"
#include"quadnode.h"


/**
 * 本节点没有划分子空间之前能存放的最大要素数目
 */
#define THRESHOLD   4

/** 
 * 子空间节点莫顿编码
 */
#define NODE_I      3
#define NODE_II     1
#define NODE_III    2
#define NODE_IV     4
#define NODE_SELF   0


#define T quadnode_t

/**
 * 在内存池上分配的要素列表节点包装
 */
struct alist{
    struct alist *next;
    void         *data;
    envelop_t     extent; 
};

struct T {
    int             depth;      //本空间节点的深度, 根节点深度为0
    int             splited;    //本空间节点是否已经划分过子空间
    int             count;      //本空间节点要素列表存放的要素数目
    arena_t         arena;      //本空间节点用的内存池
    envelop_t       subspace[5];//本空间节点的子空间    
    T               children[5];//本空间节点的子节点
    struct alist   *objlist;    //本空间节点要素列表
};

/**
 * 划分节点空间
 */
static void _split_space(T qn);


/** 
 * 返回box属于或等于4个子空间的哪个
 * 如果box与复数个子空间相交，则返回NODE_SELF
 * 代表box属于本空间
 */
static int  _space_index(T qn, envelop_t box);


/**
 * 插入子空间
 */
static int _insert_to_subspace(T qn, int spaceindex, struct alist *al);

/**
 * 用于将已经插入到某节点的对象
 * 连同其包装的列表节点一起插入
 * 到其他节点, 以免泄漏内存
 */
static int _insert(T qn, struct alist *al);


T       
quadnode_new
(arena_t arena, int depth, double left, double top, double right, double bottom)
{
    T qn;

    assert(arena);
    qn = (T)ARENA_ALLOC(arena, sizeof(*qn));
    qn->depth = depth;
    qn->splited = 0;
    qn->count = 0;
    qn->arena = arena;
    qn->objlist = NULL;
    qn->subspace[NODE_SELF] = envelop_new(arena, left, top, right, bottom);
    qn->subspace[NODE_I]    = envelop_new(arena, 
                            (left + right)/2,top, right, (top + bottom)/2);
    qn->subspace[NODE_II]   = envelop_new(arena,
                            left, top, (left + right)/2, (top + bottom)/2);
    qn->subspace[NODE_III]  = envelop_new(arena,
                            left, (top + bottom)/2, (left + right)/2, bottom);
    qn->subspace[NODE_IV]   = envelop_new(arena,
                            (left + right)/2, (top + bottom)/2, right, bottom);
    qn->children[NODE_SELF] = qn;
    qn->children[NODE_I]    = NULL;
    qn->children[NODE_II]   = NULL;
    qn->children[NODE_III]  = NULL;
    qn->children[NODE_IV]   = NULL;
    return qn;
}


T       
quadnode_new_space
(arena_t arena, int depth, envelop_t space)
{
    assert(space);
    return quadnode_new(arena, depth, envelop_left(space), envelop_top(space),
                               envelop_right(space), envelop_bottom(space));
}

int
quadnode_insert
(T qn, void *obj, double left, double top, double right, double bottom)
{

    assert(qn);

    struct alist *objwrap;
    objwrap = (struct alist*)ARENA_ALLOC(qn->arena, sizeof(*objwrap));
    objwrap->next   = NULL;
    objwrap->data   = obj;
    objwrap->extent = envelop_new(qn->arena, left, top, right, bottom);

    return _insert(qn, objwrap);
}


/**
 * 对节点及子节点包含的空间对象做快速包含检定
 */
int     
quadnode_fast_contains_check
(T qn, double x, double y)
{
    int i;
    struct alist *list;

    assert(qn);

    list = qn->objlist;

    /** 
     * 遍历处理本节点的对象列表
     */
    while(list){
        if(envelop_contains_point(list->extent, x, y)){
            return 1;
        }
        list = list->next;
    }

    /**
     * 递归检查子节点
     */
    for(i = 1; i < 5; i++){
        if(envelop_contains_point(qn->subspace[i], x, y)){
            if(qn->children[i] && 
                quadnode_fast_contains_check(qn->children[i], x, y)){
                return 1;
            }
        }
    }
    return 0;
}


/**
 * 对节点及子节点包含的空间对象做快速跨立检定
 */
int     
quadnode_fast_intersect_check
(T qn, envelop_t checkbox)
{
    int i;
    struct alist *list;

    assert(qn);
    list = qn->objlist;

    /** 
     * 遍历处理本节点的对象列表
     */
    while(list){

        if(envelop_intersect(list->extent, checkbox)){
            return 1;
        }
        list = list->next;
    }

    /**
     * 递归检查子节点
     */
    for(i = 1; i < 5; i++){
        if(envelop_intersect(qn->subspace[i], checkbox)){
            if(qn->children[i] && 
                quadnode_fast_intersect_check(qn->children[i], checkbox)){
                return 1;
            }
        }
    }
    return 0;
}


int     
quadnode_search
(T qn, envelop_t searchbox, double maxdistance, int maxcount, seq_t retlist)
{
    int count, i;
    double maxdistsq;
    struct alist *list;
    assert(qn);
    assert(searchbox);
    assert(retlist);
    assert(maxdistance >= 0);
    assert(maxcount >= 0);

    /** 定额满了就返回false*/
    count = maxcount - seq_length(retlist);
    if(count <= 0)
        return 0;

    maxdistsq = maxdistance * maxdistance;

        
    /** 搜索子空间如果返回false代表达到定额，马上返回false*/
    for(i = 1; i < 5; i++){
        if(envelop_intersect(qn->subspace[i], searchbox) &&
                qn->children[i]){
            int ret = quadnode_search(qn->children[i], 
                                    searchbox,
                                    maxdistance,
                                    maxcount,
                                    retlist);
            if(!ret)
                return 0;
        }
    }

    /** 定额还没满先搜索本节点的要素*/
    list = qn->objlist;
    while(list){
        double distsq   = envelop_distance_sq(searchbox, list->extent);
        if(distsq <= maxdistsq && count > 0){
            seq_add_low(retlist, list->data);
            count --;
            if(count <= 0)
                return 0;
        }
        list = list->next;
    }

    /** 到达这里，代表没有达到定额，返回true*/
    return 1;
}



/**
 * 划分节点空间
 */
static 
void 
_split_space(T qn)
{
    struct alist *tmplist;
    struct alist *list;
    struct alist *next;
    assert(qn);

    qn->count   = 0;
    qn->splited = 1;
    tmplist = NULL;
    list = qn->objlist;



    /** 
     * 遍历处理本节点的对象列表
     */
    while(list){
        envelop_t e = list->extent;
        next = list->next;

        /**
         * 处于本空间的, 另建表存放
         * 处于子空间的, 将它移到子空间节点
         */
        int spaceindex = _space_index(qn, e);
        if(NODE_SELF == spaceindex){
            list->next = tmplist;
            tmplist = list;
            qn->count += 1;
        }else{
            _insert_to_subspace(qn, spaceindex, list);
        }

        list = next;
    }

    qn->objlist = tmplist;
}


/** 
 * 返回box属于或等于4个子空间的哪个
 * 如果box与复数个子空间相交，则返回0
 * 代表box属于本空间
 */
static
int  
_space_index(T qn, envelop_t box)
{
    int i;

    /**
     * 如果box包含于任一子空间则返回子空间号
     */
    for(i = 1; i < 5; i++){
        if(envelop_contains(qn->subspace[i], box))
            return i;
    }
        
    /**
     * 不包含于任一子空间 说明box位置跨子空间存在
     * 则返回本节点号
     */
    return NODE_SELF;
}


/**
 * 用于将已经插入到某节点的对象
 * 连同其包装的列表节点一起插入
 * 到其他节点, 以免泄漏内存
 *
 * 由于次方法是内部调用, qn和al已经经过了断言
 */
static 
int
_insert(T qn, struct alist *al)
{
    int retval = qn->depth;
    /**
     * 空间没有划分过, 本节点要素数目也没达到阈值
     * 简单插入要素表即可
     */
    if(!qn->splited && qn->count < THRESHOLD){
        al->next = qn->objlist; 
        qn->objlist = al;
        qn->count++;
    }else{

        /**
         * 首次达到阈值，开始划分空间
         */
        if(!qn->splited)
            _split_space(qn);

        /**
         * 根据情况插入本节点或子节点
         */

        int spaceindex = _space_index(qn, al->extent);
        if(NODE_SELF == spaceindex){
            al->next = qn->objlist; 
            qn->objlist = al;
            qn->count++;
        }else{
            retval = _insert_to_subspace(qn, spaceindex, al);
        }
    }
    return retval;
}


/**
 * 插入子空间
 */
static 
int
_insert_to_subspace(T qn, int spaceindex, struct alist *alist)
{
    T child;

    /**
     * 先检查子空间是否已经存在, 否则建一个新的
     * 子空间深度为本空间节点深度+1
     */
    child = qn->children[spaceindex];
    if(!child){
        child = quadnode_new_space(qn->arena, qn->depth+1, qn->subspace[spaceindex]);
        qn->children[spaceindex] = child;
    }
    return _insert(child, alist); 
}


void    
quadnode_render
(T qn,
 envelop_t range,
void (*render)(void *obj, envelop_t e, void *cl), 
void *cl)
{
    int i;
    struct alist *list;

    assert(qn);


    list = qn->objlist;

    /** 
     * 遍历处理本节点的对象列表
     */
    while(list){

        render(list->data, list->extent, cl);

        list = list->next;
    }


    /**
     * 调用相交的子节点的渲染
     */
    for(i = 1; i < 5; i++){
        if(envelop_intersect(qn->subspace[i], range)){

            if(qn->children[i]){
                quadnode_render(qn->children[i], range, render, cl);
            }
        }
    }

}

void    
quadnode_render_depth
(T qn, int depth,
 envelop_t range,
void (*render)(void *obj, envelop_t e, void *cl), 
void *cl)
{
    int i, stop;
    struct alist *list;

    assert(qn);
    
    stop = qn->depth == depth;


    list = qn->objlist;

    /** 
     * 遍历处理本节点的对象列表
     */
    while(list){

        render(list->data, list->extent, cl);
        list = list->next;

        if(stop){
            break;
        }
    }

    if(stop){
        return;
    }


    /**
     * 调用相交的子节点的渲染
     */
    for(i = 1; i < 5; i++){
        if(envelop_intersect(qn->subspace[i], range)){

            if(qn->children[i]){
                quadnode_render_depth(qn->children[i], depth, range, render, cl);
            }
        }
    }

}
