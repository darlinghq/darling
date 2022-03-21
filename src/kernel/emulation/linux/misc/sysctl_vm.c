#include "sysctl_vm.h"
#include "../readline.h"
#include "../elfcalls_wrapper.h"
#include "../unistd/close.h"
#include "../fcntl/open.h"
#include <errno.h>
#include "../string.h"
#include "../simple.h"

#define LINUX_SC_PAGESIZE 30

enum {
	// the OID for this sysctl is OID_AUTO, so we can assign anything to it
	_VM_PAGE_PURGEABLE_COUNT = 1000,
	_VM_PAGE_PAGEABLE_INTERNAL_COUNT,
};

static sysctl_handler(handle_page_purgeable_count);
static sysctl_handler(handle_page_pageable_internal_count);

const struct known_sysctl sysctls_vm[] = {
	{ .oid = _VM_PAGE_PURGEABLE_COUNT, .type = CTLTYPE_INT, .exttype = "I", .name = "page_purgeable_count", .handler = handle_page_purgeable_count },
	{ .oid = _VM_PAGE_PAGEABLE_INTERNAL_COUNT, .type = CTLTYPE_INT, .exttype = "IU", .name = "page_pageable_internal_count", .handler = handle_page_pageable_internal_count },
	{ .oid = -1 },
};

struct meminfo {
	// all of these fields are in KiB
	uint64_t total;
	uint64_t free;
	uint64_t available;
	uint64_t buffers;
	uint64_t cached;
	uint64_t slab_reclaimable;
	uint64_t shmem;
};

static const char* find_first_not(const char* haystack, char needle) {
	// skip any characters matching `needle`
	for (; *haystack == needle; ++haystack);
	if (*haystack == '\0') {
		// if we reached the end of the string, we didn't find any characters other than `needle`;
		// return `NULL` to indicate this
		return NULL;
	} else {
		// otherwise, `haystack` points to a non-`needle` character
		return haystack;
	}
};

static bool parse_meminfo_line(const char* line, const char* key, size_t key_length, uint64_t* value) {
	if (strncmp(line, key, key_length) == 0) {
		const char* value_string = find_first_not(line + key_length, ' ');
		if (value_string) {
			*value = __simple_atoi(value_string, NULL);
			return true;
		}
	}
	return false;
};

static int read_meminfo(struct meminfo* out_info) {
	int infofd = sys_open_nocancel("/proc/meminfo", BSD_O_RDONLY, 0);
	struct rdline_buffer rbuf;
	const char* line;
	int status = 0;

	if (infofd < 0) {
		status = infofd;
		goto out;
	}

	memset(out_info, 0, sizeof(*out_info));

	_readline_init(&rbuf);

	while ((line = _readline(infofd, &rbuf)) != NULL) {
		uint64_t value;

		#define PARSE(_key, _member) \
			if (parse_meminfo_line(line, _key, sizeof(_key) - 1, &value)) { \
				out_info->_member = value; \
			}

		PARSE("MemTotal:", total)
			else PARSE("MemFree:", free)
			else PARSE("MemAvailable:", available)
			else PARSE("Buffers:", buffers)
			else PARSE("Cached:", cached)
			else PARSE("SReclaimable:", slab_reclaimable)
			else PARSE("Shmem:", shmem)
			;

		#undef PARSE
	}

out:
	if (infofd >= 0) {
		close_internal(infofd);
	}
	return status;
};

static sysctl_handler(handle_page_purgeable_count) {
	if (old && oldlen && *oldlen >= sizeof(uint32_t)) {
		// pretend we have nothing purgeable
		*(uint32_t*)old = 0;
	}
	if (oldlen) {
		*oldlen = sizeof(uint32_t);
	}
	return 0;
};

static sysctl_handler(handle_page_pageable_internal_count) {
	struct meminfo meminfo;
	uint32_t result = 0;

	if (read_meminfo(&meminfo) == 0) {
		uint64_t actual_used_kib;

		if (meminfo.available > 0) {
			actual_used_kib = meminfo.total - meminfo.available;
		} else {
			// see https://stackoverflow.com/a/41251290/6620880
			uint64_t used_kib = meminfo.total - meminfo.free;
			uint64_t cached_kib = meminfo.cached + meminfo.slab_reclaimable - meminfo.shmem;
			actual_used_kib = used_kib - (meminfo.buffers + cached_kib);
		}

		result = (actual_used_kib * 1024) / native_sysconf(LINUX_SC_PAGESIZE);
	}

	if (old && oldlen && *oldlen >= sizeof(uint32_t)) {
		*(uint32_t*)old = result;
	}
	if (oldlen) {
		*oldlen = sizeof(uint32_t);
	}

	return 0;
};
