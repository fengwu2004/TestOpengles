#ifndef MEM_INCLUDE
#define MEM_INCLUDE
#include<stdlib.h>
#include"except.h"

extern const except_t MemFailedException;

void *mem_alloc(int len, 
                const char *file,
                const char *func,
                int line);

void *mem_calloc(int count,
                int len,
                const char *file,
                const char *func,
                int line);

void *mem_resize(int len,
                void *ptr,
                const char *file,
                const char *func,
                int line);

void mem_free(void *ptr,
                const char *file,
                const char *func,
                int line);

void mem_leak(void (*apply)(const void *ptr,
                            int size,
                            const char *file,
                            const char *func,
                            int line,
                            void *cl),
                void *cl);

#define ALLOC(len)\
        mem_alloc((len), __FILE__, __func__, __LINE__)

#define CALLOC(count, len) \
	mem_calloc((count), (len), __FILE__, __func__, __LINE__)

#define  NEW(p) ((p) = ALLOC((int)sizeof(*(p))))
#define NEW0(p) ((p) = CALLOC(1, (int)sizeof(*(p))))
#define NEWARRAY(p, count) ((p) = CALLOC((int)(count), (int)sizeof(*(p))))

#define FREE(ptr) ((void)(mem_free((ptr), __FILE__, __func__, __LINE__), (ptr) = 0))

#define RESIZE(ptr, len) 	((ptr) = mem_resize((len), \
	(ptr), __FILE__, __func__, __LINE__))

#endif /*MEM_INCLUDE*/

