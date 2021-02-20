#ifndef _DARLING_EMULATION_FOR_XTRACE_H_
#define _DARLING_EMULATION_FOR_XTRACE_H_

#include <darling/emulation/base.h>
#include <darling/emulation/mman/duct_mman.h>

VISIBLE
void* _mmap_for_xtrace(void* start, unsigned long len, int prot, int flags, int fd, long pos);

VISIBLE
long _munmap_for_xtrace(void* addr, unsigned long len);

VISIBLE
long _abort_with_payload_for_xtrace(unsigned int reason_namespace, unsigned long long reason_code, void *payload, unsigned int payload_size, const char *reason_string, unsigned long long reason_flags);

#endif // _DARLING_EMULATION_FOR_XTRACE_H_
