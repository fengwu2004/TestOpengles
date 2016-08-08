#ifndef QCDT_INCLUDE
#define QCDT_INCLUDE

#include"envelop.h"

/**
 * 空间碰撞检测树
 */
#define T qcdt_t

typedef struct T *T;

extern  T           qcdt_new(double left, double top, double right, double bottom);

extern  void        qcdt_free(T *qcdt);

extern  void        qcdt_reset(T qcdt);

extern  int         qcdt_cd_check(T qcdt, double left, double top, double right, double bottom);



#undef T
#endif /*QDCT_INCLUDE*/
