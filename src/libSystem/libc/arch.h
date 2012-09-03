#ifndef LIBC_ARCH_H
#define LIBC_ARCH_H
#include <stdint.h>

struct NXArchInfo
{
	const char* name;
	int32_t cputype;
	int32_t cpusubtype;
	int byteorder;
	const char* description;
};

extern "C"
{

const NXArchInfo* NXGetAllArchInfos();
const NXArchInfo* NXGetLocalArchInfo();
const NXArchInfo* NXGetArchInfoFromName(const char *name);

}

#endif

