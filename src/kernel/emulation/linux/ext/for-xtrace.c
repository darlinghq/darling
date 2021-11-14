#include "for-xtrace.h"
#include "../mman/mman.h"
#include "../misc/abort_with_payload.h"
#include "../fcntl/open.h"
#include "../unistd/close.h"

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

// __write_for_xtrace is in unistd/write.c

long _open_for_xtrace(const char* filename, int flags, unsigned int mode) {
	return sys_open_nocancel(filename, flags, mode);
};

long _close_for_xtrace(int fd) {
	return sys_close_nocancel(fd);
};

extern size_t default_sigaltstack_size;

long _sigaltstack_set_default_size_for_xtrace(size_t new_size) {
	default_sigaltstack_size = new_size;
	return 0;
};

long _sigaltstack_for_xtrace(const struct bsd_stack* ss, struct bsd_stack* oss) {
	return sys_sigaltstack(ss, oss);
};
