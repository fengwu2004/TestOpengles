#ifndef SPARSEARRAY_INCLUDE
#define SPARSEARRAY_INCLUDE

#define T sparsearray_t

#define INDEX_NO_KEY -1

typedef struct T *T;


extern T        sparsearray_new         (long hint);
extern void     sparsearray_free        (T *sarray);

extern long    sparsearray_length      (T sarray);

extern void    *sparsearray_get         (T sarray, long key);
extern void    *sparsearray_put         (T sarray, long key, void *value);

extern void    *sparsearray_get_at      (T sarray, long index);
extern void    *sparsearray_put_at      (T sarray, long index, void *value);

extern long    sparsearray_key_at      (T sarray,  long index);

/* 
 * if use ring to record key, traverse for index of key is to slow
 */
extern long    sparsearray_index_of_key(T sarray, long key);

extern void    *sparsearray_remove      (T sarray, long key);
extern void    *sparsearray_remove_at   (T sarray, long index);


#undef T
#endif /*SPARSEARRAY_INCLUDE*/
