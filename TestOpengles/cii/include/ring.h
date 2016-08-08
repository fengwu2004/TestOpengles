#ifndef RING_INCLUDE
#define RING_INCLUDE

#define T ring_t

typedef struct T *T;


extern T        ring_new(void);
extern T        ring_ring(void *x, ...);

extern void     ring_free(T *ring);
extern int      ring_length(T ring);

extern void    *ring_get(T ring, int position);
extern void    *ring_put(T ring, int position, void *x);

extern void    *ring_add(T ring, int position, void *x);
extern void    *ring_add_low(T ring, void *x);
extern void    *ring_add_high(T ring, void *x);

extern void    *ring_remove(T ring, int position);
extern void    *ring_remove_low(T ring);
extern void    *ring_remove_high(T ring);

extern void     ring_rotate(T ring, int n);

#undef T
#endif /*RING_INCLUDE*/
