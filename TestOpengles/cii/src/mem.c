#include <stdlib.h>
#include <stddef.h>
#include "assert.h"
#include "except.h"
#include "mem.h"

const except_t MemFailedException          = {"MemFailedException"};

void *
mem_alloc(int len, 
                const char *file,
                const char *func,
                int line)
{
    void *ptr;

    assert(len > 0);

    ptr = malloc(len);
    if(NULL == ptr){
       except_raise(&MemFailedException,
                    file,
                    func,
                    "malloc",
                    line);
    }
    return ptr;
}

void *
mem_calloc(int count,
                int len,
                const char *file,
                const char *func,
                int line)
{
    void *ptr;

    assert(len > 0);
    assert(count > 0);

    ptr = calloc(count, len);

    if(NULL == ptr){
       except_raise(&MemFailedException,
                    file,
                    func,
                    "calloc",
                    line);
    }
    return ptr;
}

void *
mem_resize(int len,
                void *ptr,
                const char *file,
                const char *func,
                int line)
{
    assert(ptr);
    assert(len > 0);

    ptr = realloc(ptr, len);

    if(NULL == ptr){
       except_raise(&MemFailedException,
                    file,
                    func,
                    "realloc",
                    line);
    }
    return ptr;
}

void 
mem_free(void *ptr,
                const char *file,
                const char *func,
                int line)
{
    if(ptr)
        free(ptr);
}
