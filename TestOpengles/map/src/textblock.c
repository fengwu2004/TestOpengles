#include<stdlib.h>
#include<string.h>
#include<float.h>
#include"assert.h"
#include"arena.h"
#include"textblock.h"

#define MAX_LINES 8
#define T textblock_t

struct textline{
    float left;
    float top;
    float right;
    float bottom;
    const char *text;
};

struct T{
    float       left;
    float       top;
    float       right;
    float       bottom;
    int         linecnt;
    struct textline  lines[MAX_LINES];
    char       *text;
};




T                
textblock_new       
(arena_t arena, const char *text,
struct textbound (*text_bounds)(const char *line, float x, float y, void *cl),
void *cl)
{
    T retval;
    int len,i; 
    char *head;
    float offsety;

    assert(arena);
    assert(text);
    assert(text_bounds);

    len = strlen(text);
    retval = ARENA_ALLOC(arena, sizeof(*retval) + sizeof(*text)*(len+1));
    retval->text = (char*) (retval + 1);

    strncpy(retval->text, text, len+1);

    i = 0;
    head = strtok(retval->text, "\n");
    offsety = 0;

    retval->left    = FLT_MAX;
    retval->top     = FLT_MAX;
    retval->right   = FLT_MIN;
    retval->bottom  = FLT_MIN;

    while(head && i < MAX_LINES){
        textline_t  line = &(retval->lines[i++]);
        line->text = head;

        /**
         * 算单行的文本范围
         */
        struct textbound tbound = text_bounds(head, 0, offsety, cl);
        line->left = tbound.left;
        line->top = tbound.top;
        line->right = tbound.right;
        line->bottom = tbound.bottom;
        /**
         * 把行的文本范围JOIN到块的范围
         */
        if(retval->left > tbound.left){
            retval->left = tbound.left;
        }
        if(retval->top > tbound.top){
            retval->top = tbound.top;
        }
        if(retval->right < tbound.right){
            retval->right = tbound.right;
        }
        if(retval->bottom < tbound.bottom){
            retval->bottom = tbound.bottom;
        }
        offsety += (tbound.bottom - tbound.top);
        head = strtok(NULL, "\n");
    }
    retval->linecnt = i;

    return retval;
}

struct textbound 
textblock_bound     
(T tb)
{
    assert(tb);
    struct textbound retval;
    retval.left = tb->left;
    retval.top  = tb->top;
    retval.right= tb->right;
    retval.bottom=tb->bottom;
    return retval;
}

int              
textblock_line_count
(T tb)
{
    assert(tb);
    return tb->linecnt;
}

textline_t       
textblock_get_line  
(T tb, int index)
{
    assert(tb);
    assert(index>=0 && index < MAX_LINES);
    return &(tb->lines[index]);
}

const char *
textline_get_text   
(textline_t tl)
{
    assert(tl);
    return tl->text;
}

struct textbound 
textline_bound      
(textline_t tl)
{
    assert(tl);
    struct textbound retval;
    retval.left = tl->left;
    retval.top  = tl->top;
    retval.right= tl->right;
    retval.bottom=tl->bottom;
    return retval;
}
