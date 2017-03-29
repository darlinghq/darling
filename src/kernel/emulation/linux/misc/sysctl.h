#ifndef LINUX_SYSCTL_H
#define LINUX_SYSCTL_H
#include <sys/sysctl.h>

long sys_sysctl(int* name, unsigned int nlen, void* old, unsigned long* oldlen, void* _new, unsigned long newlen);
long sys_sysctlbyname(const char* name, unsigned long namelen, void* old, unsigned long* oldlen, void* _new, unsigned long newlen);

#define sysctl_handle_size(size) \
	if (old == NULL) { \
		*oldlen = size; \
		return 0; \
	} else if (*oldlen < size) \
		return -EINVAL; \
	else \
		*oldlen = size;

typedef long (*sysctl_handler_t)(const int* name, int nlen, void* old, unsigned long* oldlen, void* _new, unsigned long newlen);

#define sysctl_handler(_hname) long _hname(const int* name, int nlen, void* old, unsigned long* oldlen, void* _new, unsigned long newlen)
void copyout_string(const char* str, char* out, unsigned long* out_len);

struct known_sysctl
{
	int oid; // e.g. HW_MEMSIZE, terminating entry has oid -1
	int type; // e.g. CTLTYPE_QUAD
	const char* exttype; // e.g. "UQ"
	const char* name; // e.g. "memsize"

	union {
		const struct known_sysctl* subctls; // for CTLTYPE_NODE
		sysctl_handler_t handler; // for everything else
	};
};
extern const struct known_sysctl sysctls_root;

#endif

