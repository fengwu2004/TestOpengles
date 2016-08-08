#include"assert.h"
#include"mem.h"
#include"arena.h"
#include"qcdt.h"

#define T qcdt_t

/**
 * 没有划分子空间之前能存放的最大对象数目
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

struct extent{
    double   left;
    double   top;
    double   right;
    double   bottom;
};

struct objlist{
    struct extent   box;
    struct objlist   *next;
};

struct qnode{
    int             splited;    //本节点是否已经划分过子空间
    int             count;      //本节点要素列表存放的对象数目
    struct extent   subspace[5];//本空间节点的子空间    
    struct qnode   *children[5];//本空间节点的子节点
    struct objlist *boxlist;    //本空间节点要素列表
};

struct T{
    arena_t         arena;
    struct qnode   *root;
};

/**
 * 初始化节点
 */
static struct qnode * _init_node(T qcdt, double left, double top, double right, double bottom);

/**
 * 划分节点空间
 */
static void _split_space(T qcdt, struct qnode *qn);


/** 
 * 返回box属于或等于4个子空间的哪个
 * 如果box与复数个子空间相交，则返回NODE_SELF
 * 代表box属于本空间
 */
static int  _space_index(struct qnode *qn, double left, double top, double right, double bottom);


/**
 * 插入子空间
 */
static void _insert_to_subspace(T qcdt, struct qnode *qn, int spaceindex, struct objlist *al);

/**
 * 用于将已经插入空间盒子
 */
static void _insert(T qcdt, struct qnode *qn, struct objlist *al);

/**
 * 范围包含检定
 */
static int _extent_contains(struct extent *e, double left, double top, double right, double bottom);

/**
 * 快速跨立检定
 */
static int _fast_intersect_check(struct qnode *qn, double left, double top, double right, double bottom);

T           
qcdt_new
(double left, double top, double right, double bottom)
{
    assert(right >= left);
    assert(bottom >= top);

    T   qcdt = ALLOC(sizeof(*qcdt));
    qcdt->arena         = arena_new();
    qcdt->root          = _init_node(qcdt, left, top, right, bottom);

    return qcdt;
}

void        
qcdt_free
(T *qcdt)
{
    assert(qcdt);
    assert(*qcdt);

    arena_dispose(&((*qcdt)->arena));
    FREE(*qcdt);
}

void        
qcdt_reset
(T qcdt)
{
    assert(qcdt);
    struct extent bound = qcdt->root->subspace[NODE_SELF];
    arena_free(qcdt->arena);
    qcdt->root          = _init_node(qcdt, bound.left, bound.top, bound.right, bound.bottom);
}

int         
qcdt_cd_check
(T qcdt, double left, double top, double right, double bottom)
{
    int intersect_check;

    intersect_check = _fast_intersect_check(qcdt->root, left, top, right, bottom);
    /**
     * 没有碰撞, 插入box
     */
    if(!intersect_check){
        struct objlist *ol;
        ol = (struct objlist*)ARENA_ALLOC(qcdt->arena, sizeof(*ol));
        ol->next       = NULL;
        ol->box.left   = left;
        ol->box.top    = top;
        ol->box.right  = right;
        ol->box.bottom = bottom;

        _insert(qcdt, qcdt->root, ol);
        return 1;
    }else{
        return 0;
    }
}

static 
struct qnode *
_init_node(T qcdt, double left, double top, double right, double bottom)
{
    struct qnode * retval;

    retval = ARENA_ALLOC(qcdt->arena, sizeof(*retval));
    retval->splited = 0;
    retval->count = 0;
    retval->boxlist = NULL;

    retval->subspace[NODE_SELF].left   = left;
    retval->subspace[NODE_SELF].top    = top;
    retval->subspace[NODE_SELF].right  = right,
    retval->subspace[NODE_SELF].bottom = bottom;

    retval->subspace[NODE_I].left   = (left + right)/2.0f;
    retval->subspace[NODE_I].top    = top;
    retval->subspace[NODE_I].right  = right;
    retval->subspace[NODE_I].bottom = (top + bottom)/2.0f;

    retval->subspace[NODE_II].left   = left;
    retval->subspace[NODE_II].top    = top;
    retval->subspace[NODE_II].right  = (left + right)/2.0f;
    retval->subspace[NODE_II].bottom = (top + bottom)/2.0f;

    retval->subspace[NODE_III].left   = left;
    retval->subspace[NODE_III].top    = (top + bottom)/2.0f;
    retval->subspace[NODE_III].right  = (left + right)/2.0f;
    retval->subspace[NODE_III].bottom = bottom;

    retval->subspace[NODE_IV].left   = (left + right)/2.0f;
    retval->subspace[NODE_IV].top    = (top + bottom)/2.0f;
    retval->subspace[NODE_IV].right  = right;
    retval->subspace[NODE_IV].bottom = bottom;

    retval->children[NODE_SELF] = retval;
    retval->children[NODE_I]    = NULL;
    retval->children[NODE_II]   = NULL;
    retval->children[NODE_III]  = NULL;
    retval->children[NODE_IV]   = NULL;

    return retval;
}

static 
void 
_split_space(T qcdt, struct qnode *qn)
{
    struct objlist *tmplist;
    struct objlist *list;
    struct objlist *next;

    qn->count   = 0;
    qn->splited = 1;
    tmplist = NULL;
    list = qn->boxlist;


    /** 
     * 遍历处理本节点的对象列表
     */
    while(list){
        next = list->next;

        /**
         * 处于本空间的, 另建表存放
         * 处于子空间的, 将它移到子空间节点
         */
        int spaceindex = _space_index(qn, list->box.left, list->box.top, list->box.right, list->box.bottom);
        if(NODE_SELF == spaceindex){
            list->next = tmplist;
            tmplist = list;
            qn->count += 1;
        }else{
            _insert_to_subspace(qcdt, qn, spaceindex, list);
        }

        list = next;
    }

    qn->boxlist = tmplist;
}

static 
int     
_extent_contains
(struct extent *e, double left, double top, double right, double bottom)
{
    if(left >= e->left &&
            top >= e->top &&
            right <= e->right &&
            bottom <= e->bottom)
        return 1;
    else
        return 0;
}

static 
int  
_space_index(struct qnode *qn, double left, double top, double right, double bottom)
{
    int i;

    /**
     * 如果box包含于任一子空间则返回子空间号
     */
    for(i = 1; i < 5; i++){
        if(_extent_contains(&(qn->subspace[i]), left, top, right, bottom))
            return i;
    }
        
    /**
     * 不包含于任一子空间 说明box位置跨子空间存在
     * 则返回本节点号
     */
    return NODE_SELF;
}

static 
void
_insert_to_subspace(T qcdt, struct qnode *qn, int spaceindex, struct objlist *ol)
{
    struct qnode *child;

    /**
     * 先检查子空间是否已经存在, 否则建一个新的
     */
    child = qn->children[spaceindex];
    if(!child){
        child = _init_node(qcdt, 
                qn->subspace[spaceindex].left, qn->subspace[spaceindex].top,
                qn->subspace[spaceindex].right, qn->subspace[spaceindex].bottom);
                                
        qn->children[spaceindex] = child;
    }
    _insert(qcdt, child, ol);
}

static 
void
_insert(T qcdt, struct qnode *qn, struct objlist *ol)
{
    /**
     * 空间没有划分过, 本节点要素数目也没达到阈值
     * 简单插入要素表即可
     */
    if(!qn->splited && qn->count < THRESHOLD){
        ol->next = qn->boxlist; 
        qn->boxlist = ol;
        qn->count++;
    }else{

        /**
         * 首次达到阈值，开始划分空间
         */
        if(!qn->splited)
            _split_space(qcdt, qn);

        /**
         * 根据情况插入本节点或子节点
         */

        int spaceindex = _space_index(qn, ol->box.left, ol->box.top, ol->box.right, ol->box.bottom);
        if(NODE_SELF == spaceindex){
            ol->next = qn->boxlist; 
            qn->boxlist = ol;
            qn->count++;
        }else{
            _insert_to_subspace(qcdt, qn, spaceindex, ol);
        }
    }
}

int     
_intersect_box
(struct extent *e, double left, double top, double right, double bottom)
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
static 
int 
_fast_intersect_check(struct qnode *qn, double left, double top, double right, double bottom)
{
    int i;
    struct objlist *list;

    assert(qn);
    list = qn->boxlist;

    /** 
     * 遍历处理本节点的对象列表
     */
    while(list){

        if(_intersect_box(&(list->box), left, top, right, bottom)){
            return 1;
        }
        list = list->next;
    }

    /**
     * 递归检查子节点
     */
    for(i = 1; i < 5; i++){
        if(_intersect_box(&(qn->subspace[i]), left, top, right, bottom)){
            if(qn->children[i] && 
                _fast_intersect_check(qn->children[i], left, top, right, bottom)){
                return 1;
            }
        }
    }
    return 0;
}

#undef THRESHOLD  
#undef NODE_I     
#undef NODE_II    
#undef NODE_III   
#undef NODE_IV    
#undef NODE_SELF  
