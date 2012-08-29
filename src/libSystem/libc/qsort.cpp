#include "qsort.h"

static int __darwin_qsort_r_helper(const void* a, const void* b, void* data)
{
	__darwin_qsort_r_context* ctx = (__darwin_qsort_r_context*)data;
	return ctx->compar(ctx->thunk, a, b);
}

void __darwin_qsort_r(void* base, size_t nel, size_t width, void* thunk,
                      int (*compar)(void*, const void*, const void*))
{
	__darwin_qsort_r_context ctx;
	ctx.compar = compar;
	ctx.thunk = thunk;
	qsort_r(base, nel, width, &__darwin_qsort_r_helper, &ctx);
}

