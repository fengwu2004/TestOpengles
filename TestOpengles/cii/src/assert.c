#include "assert.h"
#include "except.h"

const except_t AssertFailedException       = {"AssertFailedException"};

/*
 * 要想跟踪到异常抛出地点, 就不能用assert函数, 要用宏, 否则总是跟踪到这里
void (assert)(int e) {
	assert(e);
}*/
