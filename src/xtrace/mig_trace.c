#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include <dlfcn.h>
#include <stdbool.h>

#include <mach/mach.h>

#include "simple.h"
#include "xtracelib.h"
#include "mach_trace.h"
#include "bsd_trace.h"
#include "xtrace/xtrace-mig-types.h"
#include "tls.h"

#define XTRACE_MIG_DIR_PATH "/usr/lib/darling/xtrace-mig"

static size_t subsystems_cnt = 0;
static struct xtrace_mig_subsystem** subsystems = NULL;

static mach_port_name_t host_port;

void xtrace_setup_mig_tracing(void)
{
	// This runs before the syscall tracing is enabled, so we can
	// freely use libSystem and make syscalls.

	host_port = mach_host_self();

	DIR* xtrace_mig_dir = opendir(XTRACE_MIG_DIR_PATH);
	if (xtrace_mig_dir == NULL)
	{
		perror("xtrace: failed to open " XTRACE_MIG_DIR_PATH);
		return;
	}
	// Count the number of files, and allocate this many subsystem pointers;
	for (struct dirent* dirent; (dirent = readdir(xtrace_mig_dir)) != NULL; subsystems_cnt++);
	subsystems = malloc(subsystems_cnt * sizeof(struct xtrace_mig_subsystem*));

	rewinddir(xtrace_mig_dir);
	for (size_t i = 0; i < subsystems_cnt; i++)
	{
		struct dirent* dirent = readdir(xtrace_mig_dir);
		if (dirent == NULL)
		{
			perror("xtrace: readdir");
			subsystems_cnt = i;
			break;
		}
		if (dirent->d_type != DT_REG)
		{
			subsystems[i] = NULL;
			continue;
		}
		size_t path_size = strlen(XTRACE_MIG_DIR_PATH) + 1 + strlen(dirent->d_name) + 1;
		char path[path_size];
		strcpy(path, XTRACE_MIG_DIR_PATH "/");
		strcat(path, dirent->d_name);

		void* dylib_handle = dlopen(path, RTLD_LOCAL);
		if (dylib_handle == NULL)
		{
			fprintf(stderr, "xtrace: failed to dlopen %s: %s\n", path, dlerror());
			subsystems[i] = NULL;
			continue;
		}
		subsystems[i] = (struct xtrace_mig_subsystem*) dlsym(dylib_handle, "xtrace_mig_subsystem");
		if (subsystems[i] == NULL)
		{
			fprintf(stderr, "xtrace: failed to dlsym(%s, \"xtrace_mig_subsystem\"): %s\n", path, dlerror());
			// Leave NULL subsystem in place and continue.
		}
	}

	closedir(xtrace_mig_dir);
}

DEFINE_XTRACE_TLS_VAR(bool, is_first_arg);

#define BEFORE if (!get_is_first_arg()) __simple_printf(", ")
#define AFTER set_is_first_arg(false)

static void add_raw_arg(const char* format, ...)
{
	va_list vl;
	va_start(vl, format);

	BEFORE;
	char buf[100];
	__simple_vsprintf(buf, format, vl);
	AFTER;

	va_end(vl);
}

static void add_num_arg(unsigned long long n)
{
	BEFORE;
	__simple_printf("%llu", n);
	AFTER;
}

static void add_ptr_arg(void* ptr)
{
	BEFORE;
	__simple_printf("%p", ptr);
	AFTER;
}

static void add_string_arg(const char* s)
{
	BEFORE;
	char buf[1024];
	xtrace_format_string_literal(buf, s);
	__simple_printf("%s", buf);
	AFTER;
}

static void add_bytes_arg(const void* bytes, unsigned long cnt)
{
	BEFORE;
	const unsigned char* b = (const unsigned char*) bytes;
	__simple_printf("bytes ");
	for (int i = 0; i < cnt; i++)
		__simple_printf("%x", b[i]);
	AFTER;
}

static void add_return_code_arg(kern_return_t code)
{
	BEFORE;
	char buf[100];
	xtrace_kern_return_to_str(buf, code);
	__simple_printf("return %s", buf);
	AFTER;
}

static void add_port_arg(mach_port_name_t port_name, mach_msg_type_name_t disposition)
{
	BEFORE;
	__simple_printf("%s %u", xtrace_msg_type_to_str(disposition, 0), port_name);
	AFTER;
}

static void add_ool_mem_arg(const void* ptr, unsigned long size)
{
	BEFORE;
	__simple_printf("mem [%p; %lu]", ptr, size);
	AFTER;
}

static void add_ool_ports_arg(const void* ptr, unsigned long cnt, mach_msg_type_name_t disposition)
{
	BEFORE;
	__simple_printf("%s [%p; x%lu]", xtrace_msg_type_to_str(disposition, 0), ptr, cnt);
	AFTER;
}

static unsigned long long read_integer(const void* ptr, unsigned int size)
{
	switch (size / 8)
	{
		case 1:
			return *(unsigned char*) ptr;
		case sizeof(short):
			return *(unsigned short*) ptr;
		case sizeof(int):
			return *(unsigned int*) ptr;
/*
		case sizeof(long):
			return *(unsigned long*) ptr;
*/
		case sizeof(long long):
			return *(unsigned long long*) ptr;
		default:
			return -1;
	}
}

static void add_struct_arg(const void* ptr, unsigned long cnt, unsigned long item_size)
{
	BEFORE;
	unsigned char* p = (unsigned char*) ptr;
	__simple_printf("{");
	for (unsigned long i = 0; i < cnt; i++)
	{
		if (i != 0)
			__simple_printf(", ");
		__simple_printf("%llu", read_integer((void*) p, item_size));
		p += item_size;
	}
	__simple_printf("}");
	AFTER;
}

static void add_array_arg(const void* ptr, unsigned long cnt, unsigned long item_size)
{
	BEFORE;
	unsigned char* p = (unsigned char*) ptr;
	__simple_printf("[");
	for (unsigned long i = 0; i < cnt; i++)
	{
		if (i != 0)
			__simple_printf(", ");
		__simple_printf("%llu", read_integer((void*) p, item_size));
		p += item_size;
	}
	__simple_printf("]");
	AFTER;
}

static void set_return_code(kern_return_t code)
{
	BEFORE;
	char buf[100];
	xtrace_kern_return_to_str(buf, code);
	__simple_printf("%s", buf);
	AFTER;
}

#undef BEFORE
#undef AFTER

static const struct xtrace_mig_callbacks callbacks = {
	.add_raw_arg = add_raw_arg,
	.add_num_arg = add_num_arg,
	.add_ptr_arg = add_ptr_arg,
	.add_string_arg = add_string_arg,
	.add_bytes_arg = add_bytes_arg,
	.add_return_code_arg = add_return_code_arg,
	.add_port_arg = add_port_arg,
	.add_ool_mem_arg = add_ool_mem_arg,
	.add_ool_ports_arg = add_ool_ports_arg,
	.add_struct_arg = add_struct_arg,
	.add_array_arg = add_array_arg,
	.set_return_code = set_return_code
};

static const struct xtrace_mig_routine_desc* find_routine(mach_msg_id_t id, const struct xtrace_mig_subsystem* s, int* out_is_reply)
{
	if (s == NULL)
		return NULL;

	const struct xtrace_mig_routine_desc* r = NULL;
	int is_reply;

	if (s->base <= id && id < (s->base + s->routine_cnt))
	{
		r = &s->routines[id - s->base];
		is_reply = 0;
	}
	else if (s->base + 100 <= id && id < (s->base + 100 + s->routine_cnt))
	{
		r = &s->routines[id - (s->base + 100)];
		is_reply = 1;
	}

	if (r == NULL)
		return NULL;
	if (!r->present)
		return NULL;
	if (is_reply && !r->reply_present)
		return NULL;

	*out_is_reply = is_reply;
	return r;
}

static int filter(const struct xtrace_mig_subsystem* s, mach_port_name_t request_port)
{
	if (s == NULL)
		return 0;

	// mach_host.defs and job.defs use the same msgids,
	// so use the request port to distinguish them.
	if (request_port == bootstrap_port)
		return strncmp(s->name, "job", 3) == 0;
	if (request_port == host_port)
		return strncmp(s->name, "host", 4) == 0;

	return 1;
}

static int find_subsystem(
	mach_msg_id_t id,
	mach_port_name_t request_port,
	int do_filter,
	const struct xtrace_mig_subsystem** out_s,
	const struct xtrace_mig_routine_desc** out_r,
	int* out_is_reply
)
{
	// First, check if it matches any routine that has both a request and a reply.
	// The reason for doing it like this is that many subsystems are actually
	// "reply" and "forward" versions of other subsystems/routines, consisting only
	// of simpleroutines; and we want to find the original ones if possible.
	for (size_t i = 0; i < subsystems_cnt; i++)
	{
		if (do_filter && !filter(subsystems[i], request_port))
			continue;

		const struct xtrace_mig_routine_desc* r = find_routine(id, subsystems[i], out_is_reply);
		if (r != NULL && r->reply_present)
		{
			*out_s = subsystems[i];
			*out_r = r;
			return 1;
		}
	}

	// Now, just see if it matches anything.
	for (size_t i = 0; i < subsystems_cnt; i++)
	{
		if (do_filter && !filter(subsystems[i], request_port))
			continue;

		const struct xtrace_mig_routine_desc* r = find_routine(id, subsystems[i], out_is_reply);
		if (r != NULL)
		{
			*out_s = subsystems[i];
			*out_r = r;
			return 1;
		}
	}

	return 0;
}

void xtrace_print_mig_message(const mach_msg_header_t* message, mach_port_name_t request_port)
{
	if (message == NULL)
		return;

	const struct xtrace_mig_subsystem* s;
	const struct xtrace_mig_routine_desc* r;
	int is_reply;

	int res = find_subsystem(message->msgh_id, request_port, 1, &s, &r, &is_reply);
	if (!res)
		res = find_subsystem(message->msgh_id, request_port, 0, &s, &r, &is_reply);
	if (!res)
		return;

	if (!is_reply)
		__simple_printf("%s::%s(", s->name, r->name);
	else
	{
		xtrace_set_gray_color();
		__simple_printf("%s::%s() -> ", s->name, r->name);
		xtrace_reset_color();
	}

	set_is_first_arg(true);
	r->routine(message, is_reply, &callbacks);

	if (!is_reply)
		__simple_printf(")");
	else
		__simple_printf(" ");
}
