#include "sysctl.h"
#include "../base.h"
#include "../errno.h"
#include "../simple.h"
#include "../../../../../platform-include/sys/errno.h"
#include "sysctl_inc.h"
#include <stddef.h>
#include <limits.h>
#include "sysctl_hw.h"
#include "sysctl_unspec.h"
#include "sysctl_kern.h"
#include "sysctl_machdep.h"

extern char *strchr(const char *s, int c);
extern int strncmp(const char *s1, const char *s2, __SIZE_TYPE__ n);
extern int strcmp(const char *s1, const char *s2);
extern int strlcpy(char* dst, const char* src, __SIZE_TYPE__ n);
extern __SIZE_TYPE__ strlen(const char* src);
extern char* strtok_r(char* str, const char* delim, char** saveptr);

const struct known_sysctl sysctls_global[] = {
	{ .oid = CTL_UNSPEC, .type = CTLTYPE_NODE, .exttype = "", .name = "unspec", .subctls = sysctls_unspec },
	{ .oid = CTL_HW, .type = CTLTYPE_NODE, .exttype = "", .name = "hw", .subctls = sysctls_hw },
	{ .oid = CTL_KERN, .type = CTLTYPE_NODE, .exttype = "", .name = "kern", .subctls = sysctls_kern },
	{ .oid = CTL_MACHDEP, .type = CTLTYPE_NODE, .exttype = "", .name = "machdep", .subctls = sysctls_machdep },
	{ .oid = -1 }, /* terminating entry */
};

const struct known_sysctl sysctls_root = {
	.oid = 0, .type = CTLTYPE_NODE, .exttype = "", .name = "", .subctls = sysctls_global
};

void copyout_string(const char* str, char* out, unsigned long* out_len)
{
	unsigned long len;
	if (out != NULL)
		len = strlcpy(out, str, *out_len);
	else
		len = strlen(str);

	if (out_len != NULL)
		*out_len = len;
}

long sys_sysctl(int* name, unsigned int nlen, void* old,
		unsigned long* oldlen, void* _new, unsigned long newlen)
{
	int i;
	const struct known_sysctl* current = &sysctls_root;

	// __simple_printf("sysctl %d,%d\n", name[0], name[1]);

	for (i = 0; i < nlen; i++)
	{
		int j;
		const struct known_sysctl* next = NULL;

		for (j = 0; current->subctls[j].oid != -1; j++)
		{
			if (current->subctls[j].oid == name[i])
			{
				next = &current->subctls[j];
				break;
			}
		}

		if (!next)
		{
			// __simple_printf("%d not found in %s\n", name[i], current->name);
			return -ENOENT;
		}

		current = next;
		if (next->type != CTLTYPE_NODE)
			break;
	}

	if (current->type == CTLTYPE_NODE)
	{
		// __simple_printf("-->ISDIR: %s\n", current->name);
		return -EISDIR;
	}

	long rv = current->handler(name, nlen, old, oldlen, _new, newlen);
	// __simple_printf("sysctl rv %d\n", rv);
	return rv;
}


long sys_sysctlbyname(const char* name, unsigned long namelen, void* old, unsigned long* oldlen, void* _new, unsigned long newlen)
{
	const struct known_sysctl* current = &sysctls_root;
	char* saveptr;
	const char* token;
	// __simple_printf("sysctlbyname: %s\n", name);

	// Used by launchd, assumed to succeed
	if (strcmp(name, "vfs.generic.noremotehang") == 0)
		return 0;
	
	token = strtok_r((char*) _new, ".", &saveptr);

	while (token != NULL)
	{
		const struct known_sysctl* next = NULL;
		int i;

		if (current->type != CTLTYPE_NODE)
			return -ENOTDIR;

		for (i = 0; current->subctls[i].oid != -1; i++)
		{
			if (strcmp(current->subctls[i].name, token) == 0)
			{
				next = &current->subctls[i];
				break;
			}
		}

		if (next == NULL)
			return -ENOENT;

		current = next;
		token = strtok_r(NULL, ".", &saveptr);
	}

	if (current->type == CTLTYPE_NODE)
		return -EISDIR;

	return current->handler(NULL, 0, old, oldlen, _new, newlen);
}


