#include<stdlib.h>
#include<string.h>
#include"assert.h"
#include"mem.h"
#include"ring.h"
#include"table.h"
#include"sparsearray.h"

#define T sparsearray_t

struct T{
    ring_t ring;
    table_t table;
};


T
sparsearray_new(long hlong)
{
    T sarray;
    NEW(sarray);
    
    sarray->ring =  ring_new();
    sarray->table = table_new(hlong, NULL, NULL);

    return sarray;
}


void
sparsearray_free(T *sarray)
{
    assert(sarray && *sarray);

    ring_free(&(*sarray)->ring);
    table_free(&(*sarray)->table);

    FREE(*sarray);
}

long
sparsearray_length(T sarray)
{
    return table_length(sarray->table);
}


void *
sparsearray_get(T sarray, long key)
{
    return table_get(sarray->table, (const void *)key);
}


void *
sparsearray_put(T sarray, long key, void *value)
{
    void *prev;
    prev = table_put(sarray->table, (const void *)key, value);
    
    if(NULL == prev)
        ring_add_high(sarray->ring, (void *)key);
    return prev;
}


void *
sparsearray_get_at(T sarray, long index)
{
    long key;
    key = sparsearray_key_at(sarray, index);
    return table_get(sarray->table, (const void *)key);
}


void *
sparsearray_put_at(T sarray, long index, void *value)
{
    long key;
    key = sparsearray_key_at(sarray, index);

    return sparsearray_put(sarray, key, value);
}


long
sparsearray_key_at(T sarray, long index)
{
    return (long)ring_get(sarray->ring, index);
}

long
sparsearray_index_of_key(T sarray, long key)
{
    long index, length, key_at;
    length = ring_length(sarray->ring);

    for(index = 0; index < length; index++){
        key_at = (long)ring_get(sarray->ring, index); 
        if(key_at == key)
            return index;
    }
    return INDEX_NO_KEY;
}

void *
sparsearray_remove(T sarray, long key)
{
    void *prev;
    long index;

    prev = table_remove(sarray->table, (const void *)key);
    if(NULL != prev){
        index = sparsearray_index_of_key(sarray, key);

        if(INDEX_NO_KEY != index)
            ring_remove(sarray->ring, index);
    }

    return prev;
}


void *
sparsearray_remove_at(T sarray, long index)
{
    void *prev;
    long key;

    key = sparsearray_key_at(sarray, index);
    ring_remove(sarray->ring, index);
    
    prev = table_remove(sarray->table, (const void *)key);

    return prev;
}

