#ifndef TEXTBLOCK_INCLUDE
#define TEXTBLOCK_INCLUDE

#include"arena.h"

#define T textblock_t

struct textbound{
    float left;
    float top;
    float right;
    float bottom;
};

/**
 * 文本块封装
 */
typedef struct T *T;

/**
 * 文本行封装
 */
typedef struct textline     *textline_t;


/**
 * 新建文本块
 * 为了解除和字体库的耦合
 * 要求创建者传入一个回调函数以计算文本块的范围
 */
extern T                textblock_new       (arena_t arena, 
                                            const char *text,
                                            struct textbound (*text_bounds)(const char *line, float x, float y, void *cl),
                                            void *cl);

/**
 * 获取文本块范围
 */
extern struct textbound textblock_bound     (T tb);

/**
 * 获取文本块行数
 */
extern int              textblock_line_count(T tb);

/**
 * 获取文本行
 */
extern textline_t       textblock_get_line  (T tb, int index);

/**
 * 获取该行文本
 */
extern const char      *textline_get_text   (textline_t tl);

/**
 * 获取文本行范围
 */
extern struct textbound textline_bound      (textline_t tl);

#undef T

#endif /*DIGRAPH_INCLUDE*/
