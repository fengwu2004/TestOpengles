#ifndef ARRAYREP_INCLUDE
#define ARRAYREP_INCLUDE


#define T array_t

struct T{
    int length;
    int size;
    char *array;
};

extern void arrayrep_init(T array,
                            int length,
                            int size,
                            void *ary);

#undef T

#endif /*ARRAYREP_INCLUDE*/
