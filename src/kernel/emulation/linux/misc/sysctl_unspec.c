#include "sysctl_unspec.h"
#include <sys/errno.h>

enum {
	_OID_TO_NAME = 1,
	// TODO: implement 2 - for listing?
	_NAME_TO_OID = 3,
	_GET_DATA_TYPE = 4,
};

extern char* strtok_r(char* str, const char* delim, char** saveptr);
extern void* memcpy(void* dst, const void* src, __SIZE_TYPE__ len);
extern char* strcpy(char* dst, const char* src);
extern char* strcat(char* dst, const char* src);
extern __SIZE_TYPE__ strlen(const char* str);
extern int strcmp(const char* str1, const char* str2);

static sysctl_handler(handle_oid_to_name);
static sysctl_handler(handle_name_to_oid);
static sysctl_handler(handle_get_data_type);

const struct known_sysctl sysctls_unspec[] = {
	{ .oid = _OID_TO_NAME, .type = CTLTYPE_STRING, .exttype = "S", .name = "oid_to_name", .handler = handle_oid_to_name },
	{ .oid = _NAME_TO_OID, .type = CTLTYPE_STRUCT, .exttype = "I,oid", .name = "name_to_oid", .handler = handle_name_to_oid },
	{ .oid = _GET_DATA_TYPE, .type = CTLTYPE_STRUCT, .exttype = "I,type", .name = "get_data_type", .handler = handle_get_data_type },
	{ .oid = -1 }
};

sysctl_handler(handle_oid_to_name)
{
	const int* oid = name + 2;
	const int oid_len = nlen - 2;
	char sname[64] = "";
	int i;
	const struct known_sysctl* current = &sysctls_root;

	for (i = 0; i < oid_len; i++)
	{
		int j;
		const struct known_sysctl* next = NULL;

		for (j = 0; current->subctls[j].oid != -1; j++)
		{
			if (current->subctls[j].oid == oid[i])
			{
				next = &current->subctls[j];
				break;
			}
		}

		if (!next)
			return -ENOENT;

		if (sname[0])
			strcat(sname, ".");
		strcat(sname, next->name);

		if (next->type != CTLTYPE_NODE)
			break;

		current = next;
	}

	copyout_string(sname, old, oldlen);
	return 0;
}

sysctl_handler(handle_name_to_oid)
{
	const struct known_sysctl* current = &sysctls_root;
	int oid[4];
	int oid_len = 0;
	char* saveptr;
	const char* token;

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

		oid[oid_len++] = next->oid;
		current = next;
		token = strtok_r(NULL, ".", &saveptr);
	}

	if (old == NULL)
	{
		*oldlen = sizeof(int) * oid_len;
	}
	else
	{
		if (*oldlen < sizeof(int) * oid_len)
			return -EINVAL;
		*oldlen = sizeof(int) * oid_len;
		memcpy(old, oid, *oldlen);
	}

	return 0;
}

sysctl_handler(handle_get_data_type)
{
	const int* oid = name + 2;
	const int oid_len = nlen - 2;
	int i;
	const struct known_sysctl* current = &sysctls_root;

	for (i = 0; i < oid_len; i++)
	{
		int j;
		const struct known_sysctl* next = NULL;

		if (current->type != CTLTYPE_NODE)
			return -ENOTDIR;

		for (j = 0; current->subctls[j].oid != -1; j++)
		{
			if (current->subctls[j].oid == oid[i])
			{
				next = &current->subctls[j];
				break;
			}
		}

		if (!next)
			return -ENOENT;

		current = next;
	}

	int typelen = sizeof(int) + strlen(current->exttype) + 1;
	if (old == NULL)
	{
		*oldlen = typelen;
	}
	else
	{
		if (*oldlen < typelen)
			return -EINVAL;
		*oldlen = typelen;

		*((int*)old) = current->type;
		strcpy((char*) (((int*)old)+1), current->exttype);
	}
	return 0;
}

