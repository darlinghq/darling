#include <stdlib.h>
#include <Block_private.h>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wimplicit-function-declaration"
#pragma clang diagnostic ignored "-Wstrict-prototypes"
#pragma clang diagnostic ignored "-Wint-conversion"

typedef int cmp_t(const void *, const void *);

void
qsort_b(void *base, size_t nel, size_t width, cmp_t ^cmp_b)
{
	void *cmp_f = ((struct Block_layout *)cmp_b)->invoke;
	qsort_r(base, nel, width, cmp_b, (void*)cmp_f);
}
#pragma clang diagnostic pop
