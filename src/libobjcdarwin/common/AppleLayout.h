#ifndef COMMON_APPLELAYOUT_H
#define COMMON_APPLELAYOUT_H
#include <objc/runtime.h>

union selref
{
	const char* selName;
	SEL sel;
};
struct msgref
{
	void* objc_msgSend_fixup; // function pointer
	selref sel;
};

#endif
