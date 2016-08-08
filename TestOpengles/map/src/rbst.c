#include<stdio.h>
#include<float.h>
#include"assert.h"
#include"arena.h"
#include"mem.h"
#include"rbst.h"

#define T rbst_t

struct T{
    arena_t         arena;
    struct rnode   *root;
};

struct rnode{
    float   start;
    float   end;
    struct rnode *left;
    struct rnode *right;
};

static void *_gap_search(struct rnode *node, int *has, float start, float end);
static void  _gap_insert(T rbst, struct rnode *node, float start, float end);

T           
rbst_new
()
{
    T rbst;
    rbst = ALLOC(sizeof(*rbst));
    rbst->arena = arena_new();
    rbst->root = NULL;

    return rbst;
}

void        
rbst_free
(T *rbst)
{
    assert(rbst);
    assert(*rbst);

    arena_dispose(&((*rbst)->arena));
    FREE(*rbst);
}

void         
rbst_reset
(T rbst)
{
    assert(rbst);
    arena_free(rbst->arena);
    rbst->root = NULL;
}

void *
rbst_gap_search
(T rbst, int *has, float start, float end)
{
    assert(rbst);
    if(NULL == rbst->root){
        *has = 1;
        return NULL;
    }else{
        return _gap_search(rbst->root, has, start, end);
    }
}

void         
rbst_gap_insert
(T rbst, void *pos, float start, float end)
{
    assert(rbst);

    /**
     * 这是树的根为空, 直接新建根节点
     */
    if(NULL == pos){
        
        rbst->root = ARENA_ALLOC(rbst->arena, sizeof(struct rnode));
        rbst->root->start = start;
        rbst->root->end = end;
        rbst->root->left = NULL;
        rbst->root->right= NULL;
    }else{
        _gap_insert(rbst, pos, start, end);
    }
}

static 
void  
_gap_insert(T rbst, struct rnode *node, float start, float end)
{
    /**
     * 范围在本节点范围的右边
     * 如果有右节点, 递归插入右节点
     * 否则新建右节点
     */
    if(start > node->end){
        if(NULL == node->right){
            node->right = ARENA_ALLOC(rbst->arena, sizeof(struct rnode));
            node->right->start = start;
            node->right->end = end;
            node->right->right = NULL;
            node->right->left = NULL;
            return;
        }else{
            return _gap_insert(rbst, node->right, start, end);
        }
    }
    /**
     * 范围在本节点范围的左边
     * 如果有左节点, 递归搜索左节点
     * 否则返回有空
     */
    else if(end < node->start){
        if(NULL == node->left){
            node->left = ARENA_ALLOC(rbst->arena, sizeof(struct rnode));
            node->left->start = start;
            node->left->end = end;
            node->left->right = NULL;
            node->left->left = NULL;
            return;
        }else{
            return _gap_insert(rbst, node->left, start, end);
        }
    }
    /**
     * 和本节点范围冲突
     */
    else{
        assert(0);
    }
}

static 
void *
_gap_search(struct rnode *node, int *has, float start, float end)
{
    /**
     * 范围在本节点范围的右边
     * 如果有右节点, 递归搜索右节点
     * 否则返回有空
     */
    if(start > node->end){
        if(NULL == node->right){
            *has = 1;
            return node;
        }else{
            return _gap_search(node->right, has, start, end);
        }
    }
    /**
     * 范围在本节点范围的左边
     * 如果有左节点, 递归搜索左节点
     * 否则返回有空
     */
    else if(end < node->start){
        if(NULL == node->left){
            *has = 1;
            return node;
        }else{
            return _gap_search(node->left, has, start, end);
        }
    }
    /**
     * 和本节点范围冲突
     */
    else{
        *has = 0;
        return NULL;
    }
}
