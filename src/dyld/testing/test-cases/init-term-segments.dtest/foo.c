

#include <stdio.h>
#include <dlfcn.h>
#include <stdlib.h>
#include <dispatch/dispatch.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <assert.h>
#include <unistd.h>

bool ranInit = false;
bool* gRanTerm = NULL;

#define SUPPORT_CUSTOM_SEGMENTS !(__arm64e__ || (__arm64__ && __ARM64_ARCH_8_32__))

#if SUPPORT_CUSTOM_SEGMENTS
__attribute__((section(("__SOMETEXT,__text"))))
#endif
__attribute__((constructor))
void myinit()
{
	ranInit = true;
}

bool foo(bool* ptr) {
	if (!ranInit)
		return false;
	gRanTerm = ptr;
	return true;
}

#if SUPPORT_CUSTOM_SEGMENTS
__attribute__((section(("__MORETEXT,__text"))))
#endif
__attribute__((destructor))
void myterm()
{
	if ( gRanTerm != NULL )
		*gRanTerm = true;
}