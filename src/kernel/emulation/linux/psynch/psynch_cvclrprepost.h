#ifndef LINUX_PSYNCH_CVCLRPREPOST_H
#define LINUX_PSYNCH_CVCLRPREPOST_H
#include <stdint.h>

long sys_psynch_cvclrprepost(void* cv, uint32_t cvgen, uint32_t cvugen, uint32_t cvsgen, uint32_t prepocnt, uint32_t preposeq, uint32_t flags);

#endif

