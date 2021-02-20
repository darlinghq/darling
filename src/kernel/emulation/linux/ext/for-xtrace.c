#include "for-xtrace.h"
#include "../mman/mman.h"
#include "../misc/abort_with_payload.h"

VISIBLE
void* _mmap_for_xtrace(void* start, unsigned long len, int prot, int flags, int fd, long pos) {
	return sys_mmap(start, len, prot, flags, fd, pos);
};

VISIBLE
long _munmap_for_xtrace(void* addr, unsigned long len) {
	return sys_munmap(addr, len);
};

VISIBLE
long _abort_with_payload_for_xtrace(unsigned int reason_namespace, unsigned long long reason_code, void *payload, unsigned int payload_size, const char *reason_string, unsigned long long reason_flags) {
	return sys_abort_with_payload(reason_namespace, reason_code, payload, payload_size, reason_string, reason_flags);
};
