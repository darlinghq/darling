/*
 * Copyright (c) 2008-2011 Apple Inc.  All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 * 
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this
 * file.
 * 
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 * 
 * @APPLE_LICENSE_HEADER_END@
 */

#include <si_module.h>
#include <paths.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <dirent.h>
#include <errno.h>
#include <notify.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <sys/param.h>
#include <sys/mount.h>
#include <sys/stat.h>
#include <ils.h>
#include <dispatch/dispatch.h>
#include <TargetConditionals.h>

/* notify SPI */
uint32_t notify_peek(int token, uint32_t *val);

extern uint32_t gL1CacheEnabled;

/* These really should be in netdb.h & etc. */
#define _PATH_RPCS "/etc/rpc"
#define _PATH_ALIASES "/etc/aliases"
#define _PATH_ETHERS "/etc/ethers"
#define _PATH_NETGROUP "/etc/netgroup"

static dispatch_once_t rootfs_once;
static si_item_t *rootfs = NULL;

#define CHUNK 256
#define FNG_MEM 0x00000010
#define FNG_GRP 0x00000020

#define forever for(;;)

#define VALIDATION_PASSWD 0
#define VALIDATION_MASTER_PASSWD 1
#define VALIDATION_GROUP 2
#define VALIDATION_NETGROUP 3
#define VALIDATION_ALIASES 4
#define VALIDATION_HOSTS 5
#define VALIDATION_NETWORKS 6
#define VALIDATION_SERVICES 7
#define VALIDATION_PROTOCOLS 8
#define VALIDATION_RPC 9
#define VALIDATION_FSTAB 10
#define VALIDATION_ETHERS 11
#define VALIDATION_COUNT 12

#define VALIDATION_MASK_PASSWD			0x00000001
#define VALIDATION_MASK_MASTER_PASSWD	0x00000002
#define VALIDATION_MASK_MASK_GROUP		0x00000004
#define VALIDATION_MASK_NETGROUP				0x00000008
#define VALIDATION_MASK_ALIASES			0x00000010
#define VALIDATION_MASK_HOSTS			0x00000020
#define VALIDATION_MASK_NETWORKS		0x00000040
#define VALIDATION_MASK_SERVICES		0x00000080
#define VALIDATION_MASK_PROTOCOLS		0x00000100
#define VALIDATION_MASK_RPC				0x00000200
#define VALIDATION_MASK_FSTAB			0x00000400
#define VALIDATION_MASK_ETHERS			0x00000800

typedef struct file_netgroup_member_s
{
	uint32_t flags;
	char *host;
	char *user;
	char *domain;
	struct file_netgroup_member_s *next;
} file_netgroup_member_t;

typedef struct file_netgroup_s
{
	char *name;
	uint32_t flags;
	file_netgroup_member_t *members;
	struct file_netgroup_s *next;
} file_netgroup_t;

typedef struct
{
	uint32_t validation_notify_mask;
	int notify_token[VALIDATION_COUNT];
	file_netgroup_t *file_netgroup_cache;
	uint64_t netgroup_validation_a;
	uint64_t netgroup_validation_b;
} file_si_private_t;

static pthread_mutex_t file_mutex = PTHREAD_MUTEX_INITIALIZER;

static char *
_fsi_copy_string(char *s)
{
	int len;
	char *t;

	if (s == NULL) return NULL;

	len = strlen(s) + 1;
	t = malloc(len);
	if (t == NULL) return NULL;

	bcopy(s, t, len);
	return t;
}

static char **
_fsi_append_string(char *s, char **l)
{
	int i, len;

	if (s == NULL) return l;
	if (l != NULL) {
		for (i = 0; l[i] != NULL; i++);
		len = i;
	} else {
		len = 0;
	}

	l = (char **) reallocf(l, (len + 2) * sizeof(char *));
	if (l == NULL) return NULL;

	l[len] = s;
	l[len + 1] = NULL;
	return l;
}

char **
_fsi_tokenize(char *data, const char *sep, int trailing_empty, int *ntokens)
{
	char **tokens;
	int p, i, start, end, more, len, end_on_sep;
	int scanning;

	tokens = NULL;
	end_on_sep = 0;

	if (data == NULL) return NULL;

	if (ntokens != NULL) *ntokens = 0;
	if (sep == NULL)
	{
		tokens = _fsi_append_string(data, tokens);
		if (ntokens != NULL) *ntokens = *ntokens + 1;
		return tokens;
	}

	len = strlen(sep);
	p = 0;

	while (data[p] != '\0')
	{
		end_on_sep = 1;
		/* skip leading white space */
		while ((data[p] == ' ') || (data[p] == '\t') || (data[p] == '\n')) p++;

		/* check for end of line */
		if (data[p] == '\0') break;

		/* scan for separator */
		start = p;
		end = p;
		scanning = 1;
		end_on_sep = 0;

		while (scanning == 1)
		{
			if (data[p] == '\0') break;

			for (i = 0; i < len; i++)
			{
				if (data[p] == sep[i])
				{
					scanning = 0;
					end_on_sep = 1;
					break;
				}
			}

			/* end is last non-whitespace character */
			if ((scanning == 1) && (data[p] != ' ') && (data[p] != '\t') && (data[p] != '\n')) end = p;

			p += scanning;
		}

		/* see if there's data left after p */
		more = 0;
		if (data[p] != '\0') more = 1;

		/* set the character following the token to nul */
		if (start == p) data[p] = '\0';
		else data[end + 1] = '\0';

		tokens = _fsi_append_string(data + start, tokens);
		if (ntokens != NULL) *ntokens = *ntokens + 1;
		p += more;
	}

	if ((end_on_sep == 1) && (trailing_empty != 0))
	{
		/* if the scan ended on an empty token, add a null string */
		tokens = _fsi_append_string(data + p, tokens);
		if (ntokens != NULL) *ntokens = *ntokens + 1;
	}

	return tokens;
}

char *
_fsi_get_line(FILE *fp)
{
	char s[4096];
	char *out;

	s[0] = '\0';

	fgets(s, sizeof(s), fp);
	if ((s == NULL) || (s[0] == '\0')) return NULL;

	if (s[0] != '#') s[strlen(s) - 1] = '\0';

	out = _fsi_copy_string(s);
	return out;
}

static const char *
_fsi_validation_path(int vtype)
{
	if (vtype == VALIDATION_PASSWD) return _PATH_PASSWD;
	else if (vtype == VALIDATION_MASTER_PASSWD) return _PATH_MASTERPASSWD;
	else if (vtype == VALIDATION_GROUP) return _PATH_GROUP;
	else if (vtype == VALIDATION_NETGROUP) return _PATH_NETGROUP;
	else if (vtype == VALIDATION_ALIASES) return _PATH_ALIASES;
	else if (vtype == VALIDATION_HOSTS) return _PATH_HOSTS;
	else if (vtype == VALIDATION_NETWORKS) return _PATH_NETWORKS;
	else if (vtype == VALIDATION_SERVICES) return _PATH_SERVICES;
	else if (vtype == VALIDATION_PROTOCOLS) return _PATH_PROTOCOLS;
	else if (vtype == VALIDATION_RPC) return _PATH_RPCS;
	else if (vtype == VALIDATION_FSTAB) return _PATH_FSTAB;
	else if (vtype == VALIDATION_ETHERS) return _PATH_ETHERS;

	return NULL;
}

static void
_fsi_get_validation(si_mod_t *si, int vtype, const char *path, FILE *f, uint64_t *a, uint64_t *b)
{
	struct stat sb;
	file_si_private_t *pp;
	uint32_t peek, bit;
	int status;

	if (a != NULL) *a = 0;
	if (b != NULL) *b = 0;

	if (si == NULL) return;
	if (path == NULL) return;
	if (gL1CacheEnabled == 0) return;

	pp = (file_si_private_t *)si->private;
	if (pp == NULL) return;

	if (vtype >= VALIDATION_COUNT) return;

	bit = 1 << vtype;
	if (bit & pp->validation_notify_mask)
	{
		/* use notify validation for this type */
		if (pp->notify_token[vtype] < 0)
		{
			char *str = NULL;
			asprintf(&str, "com.apple.system.info:%s", path);
			if (str == NULL) return;

			status = notify_register_check(str, &(pp->notify_token[vtype]));
			free(str);
		}

		if (a != NULL)
		{
			status = notify_peek(pp->notify_token[vtype], &peek);
			if (status == NOTIFY_STATUS_OK) *a = ntohl(peek);
		}

		if (b != NULL) *b = vtype;
	}
	else
	{
		/* use stat() and last mod time for this type */
		memset(&sb, 0, sizeof(struct stat));
		if (f != NULL)
		{
			if (fstat(fileno(f), &sb) == 0)
			{
				if (a != NULL) *a = sb.st_mtimespec.tv_sec;
				if (b != NULL) *b = sb.st_mtimespec.tv_nsec;
			}
		}
		else
		{
			path = _fsi_validation_path(vtype);
			if (path != NULL)
			{
				memset(&sb, 0, sizeof(struct stat));
				if (stat(path, &sb) == 0)
				{
					if (a != NULL) *a = sb.st_mtimespec.tv_sec;
					if (b != NULL) *b = sb.st_mtimespec.tv_nsec;
				}
			}
		}
	}
}

static int
_fsi_validate(si_mod_t *si, int cat, uint64_t va, uint64_t vb)
{
	struct stat sb;
	const char *path;
	uint32_t item_val, curr_val, vtype;
	file_si_private_t *pp;
	int status;

	if (si == NULL) return 0;

#if TARGET_OS_EMBEDDED
	/* /etc is on a read-only filesystem, so no validation is required */
	return 1;
#endif

	pp = (file_si_private_t *)si->private;
	if (pp == NULL) return 0;

	vtype = UINT32_MAX;
	switch (cat)
	{
		case CATEGORY_USER:
		{
			if (geteuid() == 0) vtype = VALIDATION_MASTER_PASSWD;
			else vtype = VALIDATION_PASSWD;
			break;
		}
		case CATEGORY_GROUP:
		{
			vtype = VALIDATION_GROUP;
			break;
		}
		case CATEGORY_GROUPLIST:
		{
			vtype = VALIDATION_GROUP;
			break;
		}
		case CATEGORY_NETGROUP:
		{
			vtype = VALIDATION_NETGROUP;
			break;
		}
		case CATEGORY_ALIAS:
		{
			vtype = VALIDATION_ALIASES;
			break;
		}
		case CATEGORY_HOST_IPV4:
		{
			vtype = VALIDATION_HOSTS;
			break;
		}
		case CATEGORY_HOST_IPV6:
		{
			vtype = VALIDATION_HOSTS;
			break;
		}
		case CATEGORY_NETWORK:
		{
			vtype = VALIDATION_NETWORKS;
			break;
		}
		case CATEGORY_SERVICE:
		{
			vtype = VALIDATION_SERVICES;
			break;
		}
		case CATEGORY_PROTOCOL:
		{
			vtype = VALIDATION_PROTOCOLS;
			break;
		}
		case CATEGORY_RPC:
		{
			vtype = VALIDATION_RPC;
			break;
		}
		case CATEGORY_FS:
		{
			vtype = VALIDATION_FSTAB;
			break;
		}
		case CATEGORY_MAC:
		{
			vtype = VALIDATION_ETHERS;
			break;
		}
		default: return 0;
	}

	if (pp->notify_token[vtype] < 0)
	{
		path = _fsi_validation_path(vtype);
		if (path == NULL) return 0;

		memset(&sb, 0, sizeof(struct stat));
		if (stat(path, &sb) != 0) return 0;
		if (va != sb.st_mtimespec.tv_sec) return 0;
		if (vb != sb.st_mtimespec.tv_nsec) return 0;
	}
	else
	{
		item_val = va;
		curr_val = -1;
		status = notify_peek(pp->notify_token[vtype], &curr_val);
		if (status != NOTIFY_STATUS_OK) return 0;

		curr_val = ntohl(curr_val);
		if (item_val != curr_val) return 0;
	}

	return 1;
}

/* netgroup support */
static char *
_fsi_append_char_to_line(char c, char *buf, size_t *x)
{
	if (x == NULL) return NULL;

	if (buf == NULL) *x = 0;

	if ((*x % CHUNK) == 0)
	{
		buf = reallocf(buf, *x + CHUNK);
		memset(buf + *x, 0, CHUNK);
	}

	buf[*x] = c;
	*x = *x + 1;

	return buf;
}

static char *
_fsi_read_netgroup_line(FILE *f)
{
	char *out = NULL;
	size_t x = 0;
	int white = 0;
	int paren = 0;

	if (f == NULL) return NULL;
	forever
	{
		int c = getc(f);

		if (c == EOF)
		{
			if (out == NULL) return NULL;
			return _fsi_append_char_to_line('\0', out, &x);
		}

		if (c == '\n') return _fsi_append_char_to_line('\0', out, &x);
		if (c == '(') paren = 1;
		else if (c == ')') paren = 0;

		if ((c == ' ') || (c == '\t'))
		{
			if ((white == 0) && (paren == 0)) out = _fsi_append_char_to_line(' ', out, &x);
			white = 1;
		}
		else if (c == '\\')
		{
			forever
			{
				c = getc(f);
				if (c == EOF) return _fsi_append_char_to_line('\0', out, &x);
				if (c == '\n') break;
			}
		}
		else
		{
			out = _fsi_append_char_to_line(c, out, &x);
			white = 0;
		}
	}
}

static file_netgroup_t *
_fsi_find_netgroup(file_netgroup_t **list, const char *name, int create)
{
	file_netgroup_t *n;

	if (list == NULL) return NULL;

	for (n = *list; n != NULL; n = n->next)
	{
		if (!strcmp(name, n->name)) return n;
	}

	if (create == 0) return NULL;

	n = (file_netgroup_t *)calloc(1, sizeof(file_netgroup_t));
	if (n == NULL) return NULL;

	n->name = strdup(name);

	n->next = *list;
	*list = n;
	return n;
}

void
_fsi_free_file_netgroup(file_netgroup_t *n)
{
	file_netgroup_member_t *m;

	if (n == NULL) return;
	free(n->name);

	m = n->members;
	while (m != NULL)
	{
		file_netgroup_member_t *x = m;
		m = m->next;
		free(x->host);
		free(x->user);
		free(x->domain);
		free(x);
	}

	free(n);
}

static void
_fsi_add_netgroup_group(file_netgroup_t *n, char *grp)
{
	file_netgroup_member_t *g;

	if (n == NULL) return;

	g = (file_netgroup_member_t *)calloc(1, sizeof(file_netgroup_member_t));
	if (g == NULL) return;

	g->flags = FNG_GRP;
	g->host = strdup(grp);

	g->next = n->members;
	n->members = g;

	return;
}

static void
_fsi_add_netgroup_member(file_netgroup_t *n, char *mem)
{
	char **tokens;
	file_netgroup_member_t *m;
	int ntokens;

	if (n == NULL) return;

	m = (file_netgroup_member_t *)calloc(1, sizeof(file_netgroup_member_t));
	if (m == NULL) return;

	tokens = _fsi_tokenize(mem + 1, ",)", 0, &ntokens);

	if (tokens == NULL)
	{
		free(m);
		return;
	}

	if ((ntokens > 0) && (tokens[0][0] != '\0')) m->host = strdup(tokens[0]);
	if ((ntokens > 1) && (tokens[1][0] != '\0')) m->user = strdup(tokens[1]);
	if ((ntokens > 2) && (tokens[2][0] != '\0')) m->domain = strdup(tokens[2]);

	free(tokens);

	m->flags = FNG_MEM;
	m->next = n->members;
	n->members = m;
}

static file_netgroup_t *
_fsi_process_netgroup_line(file_netgroup_t **pass1, char *line)
{
	file_netgroup_t *n;
	char **tokens;
	int i, ntokens = 0;

	tokens = _fsi_tokenize(line, " ", 0, &ntokens);
	if (tokens == NULL) return NULL;
	if (tokens[0] == NULL)
	{
		free(tokens);
		return NULL;
	}

	n = _fsi_find_netgroup(pass1, tokens[0], 1);

	for (i = 1; tokens[i] != NULL; i++)
	{
		if (tokens[i][0] == '(') _fsi_add_netgroup_member(n, tokens[i]);
		else if (tokens[i][0] != '\0') _fsi_add_netgroup_group(n, tokens[i]);
	}

	free(tokens);
	return n;
}

static void
_fsi_flatten_netgroup(file_netgroup_t *pass1, file_netgroup_t **top, file_netgroup_t *n, file_netgroup_member_t *m)
{
	if (n == NULL) return;

	if (n->flags == 1) return;
	n->flags = 1;

	if (*top == NULL)
	{
		*top = (file_netgroup_t *)calloc(1, sizeof(file_netgroup_t));
		if (*top == NULL) return;
		(*top)->name = strdup(n->name);
		if ((*top)->name == NULL)
		{
			free(*top);
			*top = NULL;
			return;
		}
	}

	while (m!= NULL)
	{
		if (m->flags & FNG_MEM)
		{
			file_netgroup_member_t *x = (file_netgroup_member_t *)calloc(1, sizeof(file_netgroup_member_t));
			if (x == NULL) return;

			x->flags = FNG_MEM;
			if (m->host != NULL) x->host = strdup(m->host);
			if (m->user != NULL) x->user = strdup(m->user);
			if (m->domain != NULL) x->domain = strdup(m->domain);

			x->next = (*top)->members;
			(*top)->members = x;
		}
		else
		{
			file_netgroup_t *g = _fsi_find_netgroup(&pass1, m->host, 0);
			if (g == NULL) continue;

			_fsi_flatten_netgroup(pass1, top, g, g->members);
		}

		m = m->next;
	}
}

static void
_fsi_check_netgroup_cache(si_mod_t *si)
{
	file_netgroup_t *p1, *n, *x, *a;
	char *line;
	FILE *f;
	file_si_private_t *pp;

	if (si == NULL) return;

	pp = (file_si_private_t *)si->private;
	if (pp == NULL) return;

	pthread_mutex_lock(&file_mutex);

	if (_fsi_validate(si, CATEGORY_NETGROUP, pp->netgroup_validation_a, pp->netgroup_validation_b))
	{
		pthread_mutex_unlock(&file_mutex);
		return;
	}

	n = pp->file_netgroup_cache;
	while (n != NULL)
	{
		x = n;
		n = n->next;
		_fsi_free_file_netgroup(x);
	}

	pp->file_netgroup_cache = NULL;

	f = fopen(_PATH_NETGROUP, "r");
	if (f == NULL)
	{
		pthread_mutex_unlock(&file_mutex);
		return;
	}

	_fsi_get_validation(si, VALIDATION_NETGROUP, _PATH_NETGROUP, f, &(pp->netgroup_validation_a), &(pp->netgroup_validation_b));

	p1 = NULL;

	line = _fsi_read_netgroup_line(f);
	while (line != NULL)
	{
		n = _fsi_process_netgroup_line(&p1, line);

		free(line);
		line = _fsi_read_netgroup_line(f);
	}

	fclose(f);

	for (n = p1; n != NULL; n = n->next)
	{
		a = NULL;
		_fsi_flatten_netgroup(p1, &a, n, n->members);
		for (x = p1; x != NULL; x = x->next) x->flags = 0;

		if (a != NULL)
		{
			a->next = pp->file_netgroup_cache;
			pp->file_netgroup_cache = a;
		}
	}

	n = p1;
	while (n != NULL)
	{
		x = n;
		n = n->next;
		_fsi_free_file_netgroup(x);
	}

	pthread_mutex_unlock(&file_mutex);
}

/* USERS */

static si_item_t *
_fsi_parse_user(si_mod_t *si, const char *name, uid_t uid, int which, char *data, int format, uint64_t va, uint64_t vb)
{
	char **tokens;
	int ntokens, match;
	time_t change, expire;
	si_item_t *item;
	uid_t xuid;

	if (data == NULL) return NULL;

	ntokens = 0;
	tokens = _fsi_tokenize(data, ":", 1, &ntokens);
	if (((format == 0) && (ntokens != 10)) || ((format == 1) && (ntokens !=  7)))
	{
		free(tokens);
		return NULL;
	}

	xuid = atoi(tokens[2]);
	match = 0;

	/* XXX MATCH GECOS? XXX*/
	if (which == SEL_ALL) match = 1;
	else if ((which == SEL_NAME) && (string_equal(name, tokens[0]))) match = 1;
	else if ((which == SEL_NUMBER) && (uid == xuid)) match = 1;

	if (match == 0)
	{
		free(tokens);
		return NULL;
	}

	if (format == 0)
	{
		/* master.passwd: name[0] passwd[1] uid[2] gid[3] class[4] change[5] expire[6] gecos[7] dir[8] shell[9] */
		/* struct pwd: name[0] passwd[1] uid[2] gid[3] change[5] class[4] gecos[7] dir[8] shell[9] expire[6] */
		change = atoi(tokens[5]);
		expire = atoi(tokens[6]);
		item = (si_item_t *)LI_ils_create("L4488ss44LssssL", (unsigned long)si, CATEGORY_USER, 1, va, vb, tokens[0], tokens[1], xuid, atoi(tokens[3]), change, tokens[4], tokens[7], tokens[8], tokens[9], expire);
	}
	else
	{
		/* passwd: name[0] passwd[1] uid[2] gid[3] gecos[4] dir[5] shell[6] */
		/* struct pwd: name[0] passwd[1] uid[2] gid[3] change[-] class[-] gecos[4] dir[5] shell[6] expire[-] */
		item = (si_item_t *)LI_ils_create("L4488ss44LssssL", (unsigned long)si, CATEGORY_USER, 1, va, vb, tokens[0], tokens[1], xuid, atoi(tokens[3]), 0, "", tokens[4], tokens[5], tokens[6], 0);
	}

	free(tokens); 
	return item;
}

static void *
_fsi_get_user(si_mod_t *si, const char *name, uid_t uid, int which)
{
	char *line;
	si_item_t *item;
	int fmt;
	FILE *f;
	si_list_t *all;
	uint64_t va, vb;

	if ((which == SEL_NAME) && (name == NULL)) return NULL;

	all = NULL;
	f = NULL;
	fmt = 0;
	va = 0;
	vb = 0;

	if (geteuid() == 0)
	{
		f = fopen(_PATH_MASTERPASSWD, "r");
		_fsi_get_validation(si, VALIDATION_MASTER_PASSWD, _PATH_MASTERPASSWD, f, &va, &vb);
	}
	else
	{
		f = fopen(_PATH_PASSWD, "r");
		_fsi_get_validation(si, VALIDATION_PASSWD, _PATH_PASSWD, f, &va, &vb);
		fmt = 1;
	}

	if (f == NULL) return NULL;


	forever
	{
		line = _fsi_get_line(f);
		if (line == NULL) break;

		if (line[0] == '#') 
		{
			free(line);
			line = NULL;
			continue;
		}

		item = _fsi_parse_user(si, name, uid, which, line, fmt, va, vb);
		free(line);
		line = NULL;

		if (item == NULL) continue;

		if (which == SEL_ALL)
		{
			all = si_list_add(all, item);
			si_item_release(item);
			continue;
		}

		fclose(f);
		return item;
	}
	fclose(f);
	return all;
}

/* GROUPS */

static si_item_t *
_fsi_parse_group(si_mod_t *si, const char *name, gid_t gid, int which, char *data, uint64_t va, uint64_t vb)
{
	char **tokens, **members;
	int ntokens, match;
	si_item_t *item;
	gid_t xgid;

	if (data == NULL) return NULL;

	ntokens = 0;
	tokens = _fsi_tokenize(data, ":", 1, &ntokens);
	if (ntokens != 4)
	{
		free(tokens);
		return NULL;
	}

	xgid = atoi(tokens[2]);
	match = 0;

	if (which == SEL_ALL) match = 1;
	else if ((which == SEL_NAME) && (string_equal(name, tokens[0]))) match = 1;
	else if ((which == SEL_NUMBER) && (gid == xgid)) match = 1;

	if (match == 0)
	{
		free(tokens);
		return NULL;
	}

	ntokens = 0;
	members = _fsi_tokenize(tokens[3], ",", 1, &ntokens);

	item = (si_item_t *)LI_ils_create("L4488ss4*", (unsigned long)si, CATEGORY_GROUP, 1, va, vb, tokens[0], tokens[1], xgid, members);

	free(tokens); 
	free(members);

	return item;
}

static void *
_fsi_get_group(si_mod_t *si, const char *name, gid_t gid, int which)
{
	char *line;
	si_item_t *item;
	FILE *f;
	si_list_t *all;
	uint64_t va, vb;

	if ((which == SEL_NAME) && (name == NULL)) return NULL;

	all = NULL;
	f = NULL;

	f = fopen(_PATH_GROUP, "r");
	if (f == NULL) return NULL;

	_fsi_get_validation(si, VALIDATION_GROUP, _PATH_GROUP, f, &va, &vb);

	forever
	{
		line = _fsi_get_line(f);
		if (line == NULL) break;

		if (line[0] == '#') 
		{
			free(line);
			line = NULL;
			continue;
		}

		item = _fsi_parse_group(si, name, gid, which, line, va, vb);
		free(line);
		line = NULL;

		if (item == NULL) continue;

		if (which == SEL_ALL)
		{
			all = si_list_add(all, item);
			si_item_release(item);
			continue;
		}

		fclose(f);
		return item;
	}

	fclose(f);
	return all;
}

static void *
_fsi_get_grouplist(si_mod_t *si, const char *user)
{
	char **tokens, **members;
	int ntokens, i, match, gidcount;
	char *line;
	si_item_t *item;
	FILE *f;
	uint64_t va, vb;
	gid_t gid, basegid;
	gid_t *gidlist;
	struct passwd *pw;

	if (user == NULL) return NULL;

	gidlist = NULL;
	gidcount = 0;
	f = NULL;
	basegid = -1;

	item = si->vtable->sim_user_byname(si, user);
	if (item != NULL)
	{
		pw = (struct passwd *)((uintptr_t)item + sizeof(si_item_t));
		basegid = pw->pw_gid;
		si_item_release(item);
		item = NULL;
	}

	f = fopen(_PATH_GROUP, "r");
	if (f == NULL) return NULL;

	_fsi_get_validation(si, VALIDATION_GROUP, _PATH_GROUP, f, &va, &vb);

	forever
	{
		line = _fsi_get_line(f);
		if (line == NULL) break;

		if (line[0] == '#') 
		{
			free(line);
			line = NULL;
			continue;
		}

		ntokens = 0;
		tokens = _fsi_tokenize(line, ":", 1, &ntokens);
		if (ntokens != 4)
		{
			free(tokens);
			continue;
		}

		ntokens = 0;
		members = _fsi_tokenize(tokens[3], ",", 1, &ntokens);

		match = 0;
		gid = -2;

		for (i = 0; i < ntokens; i++)
		{
			if (string_equal(user, members[i]))
			{
				gid = atoi(tokens[2]);
				match = 1;
				break;
			}
		}

		free(tokens); 
		free(members);
		free(line);
		line = NULL;

		if (match == 1)
		{
			gidlist = (gid_t *) reallocf(gidlist, (gidcount + 1) * sizeof(gid_t));
			if (gidlist == NULL)
			{
				gidcount = 0;
				break;
			}

			gidlist[gidcount++] = gid;
		}
	}

	fclose(f);

	if (gidcount != 0) {
		item = (si_item_t *)LI_ils_create("L4488s4@", (unsigned long)si, CATEGORY_GROUPLIST, 1, va, vb, user, gidcount, 
										  gidcount * sizeof(gid_t), gidlist);
	}

	free(gidlist);

	return item;
}

/* ALIASES */

static si_item_t *
_fsi_parse_alias(si_mod_t *si, const char *name, int which, char *data, uint64_t va, uint64_t vb)
{
	char **tokens, **members;
	int ntokens, match;
	si_item_t *item;

	if (data == NULL) return NULL;

	ntokens = 0;
	tokens = _fsi_tokenize(data, ":", 1, &ntokens);
	if (ntokens < 2)
	{
		free(tokens);
		return NULL;
	}

	match = 0;

	if (which == SEL_ALL) match = 1;
	else if (string_equal(name, tokens[0])) match = 1;

	if (match == 0)
	{
		free(tokens);
		return NULL;
	}

	ntokens = 0;
	members = _fsi_tokenize(tokens[1], ",", 1, &ntokens);

	item = (si_item_t *)LI_ils_create("L4488s4*4", (unsigned long)si, CATEGORY_ALIAS, 1, va, vb, tokens[0], ntokens, members, 1);

	free(tokens); 
	free(members);

	return item;
}

static void *
_fsi_get_alias(si_mod_t *si, const char *name, int which)
{
	char *line;
	si_item_t *item;
	FILE *f;
	si_list_t *all;
	uint64_t va, vb;

	if ((which == SEL_NAME) && (name == NULL)) return NULL;

	all = NULL;
	f = NULL;

	f = fopen(_PATH_ALIASES, "r");
	if (f == NULL) return NULL;

	_fsi_get_validation(si, VALIDATION_ALIASES, _PATH_ALIASES, f, &va, &vb);

	forever
	{
		line = _fsi_get_line(f);
		if (line == NULL) break;

		if (line[0] == '#') 
		{
			free(line);
			line = NULL;
			continue;
		}

		item = _fsi_parse_alias(si, name, which, line, va, vb);
		free(line);
		line = NULL;

		if (item == NULL) continue;

		if (which == SEL_ALL)
		{
			all = si_list_add(all, item);
			si_item_release(item);
			continue;
		}

		fclose(f);
		return item;
	}

	fclose(f);
	return all;
}

/* ETHERS */

static si_item_t *
_fsi_parse_ether(si_mod_t *si, const char *name, int which, char *data, uint64_t va, uint64_t vb)
{
	char **tokens;
	char *cmac;
	int ntokens, match;
	si_item_t *item;

	if (data == NULL) return NULL;

	ntokens = 0;
	tokens = _fsi_tokenize(data, " \t", 1, &ntokens);
	if (ntokens != 2)
	{
		free(tokens);
		return NULL;
	}

	cmac = si_standardize_mac_address(tokens[0]);
	if (cmac == NULL)
	{
		free(tokens);
		return NULL;
	}

	match = 0;
	if (which == SEL_ALL) match = 1;
	else if ((which == SEL_NAME) && (string_equal(name, tokens[1]))) match = 1;
	else if ((which == SEL_NUMBER) && (string_equal(name, cmac))) match = 1;

	if (match == 0)
	{
		free(tokens);
		free(cmac);
		return NULL;
	}

	item = (si_item_t *)LI_ils_create("L4488ss", (unsigned long)si, CATEGORY_MAC, 1, va, vb, tokens[1], cmac);

	free(tokens); 
	free(cmac);

	return item;
}

static void *
_fsi_get_ether(si_mod_t *si, const char *name, int which)
{
	char *line, *cmac;
	si_item_t *item;
	FILE *f;
	si_list_t *all;
	uint64_t va, vb;

	if ((which != SEL_ALL) && (name == NULL)) return NULL;

	cmac = NULL;
	if (which == SEL_NUMBER)
	{
		cmac = si_standardize_mac_address(name);
		if (cmac == NULL) return NULL;
	}

	all = NULL;
	f = NULL;

	f = fopen(_PATH_ETHERS, "r");
	if (f == NULL) return NULL;

	_fsi_get_validation(si, VALIDATION_ETHERS, _PATH_ETHERS, f, &va, &vb);

	forever
	{
		line = _fsi_get_line(f);
		if (line == NULL) break;

		if (line[0] == '#') 
		{
			free(line);
			line = NULL;
			continue;
		}

		item = NULL;
		if (which == SEL_NUMBER) item = _fsi_parse_ether(si, cmac, which, line, va, vb);
		else item = _fsi_parse_ether(si, name, which, line, va, vb);

		free(line);
		line = NULL;

		if (item == NULL) continue;

		if (which == SEL_ALL)
		{
			all = si_list_add(all, item);
			si_item_release(item);
			continue;
		}

		fclose(f);
		return item;
	}

	fclose(f);
	return all;
}

/* HOSTS */

static si_item_t *
_fsi_parse_host(si_mod_t *si, const char *name, const void *addr, int af, int which, char *data, uint64_t va, uint64_t vb)
{
	char **tokens, **h_aliases, *null_alias;
	int i, ntokens, match, h_addrtype, h_length;
	struct in_addr a4;
	struct in6_addr a6;
	si_item_t *item;
	char *h_addr_list[2];
	char h_addr_4[4], h_addr_6[16];

	if (data == NULL) return NULL;

	null_alias = NULL;

	ntokens = 0;
	tokens = _fsi_tokenize(data, " 	", 0, &ntokens);
	if (ntokens < 2)
	{
		free(tokens);
		return NULL;
	}

	h_addr_list[1] = NULL;

	h_addrtype = AF_UNSPEC;
	if (inet_pton(AF_INET, tokens[0], &a4) == 1)
	{
		h_addrtype = AF_INET;
		h_length = sizeof(struct in_addr);
		memcpy(h_addr_4, &a4, 4);
		h_addr_list[0] = h_addr_4;
	}
	else if (inet_pton(AF_INET6, tokens[0], &a6) == 1)
	{
		h_addrtype = AF_INET6;
		h_length = sizeof(struct in6_addr);
		memcpy(h_addr_6, &a6, 16);
		h_addr_list[0] = h_addr_6;
	}

	if (h_addrtype == AF_UNSPEC)
	{
		free(tokens);
		return NULL;
	}

	h_aliases = NULL;
	if (ntokens > 2) h_aliases = &(tokens[2]);

	match = 0;

	if (which == SEL_ALL) match = 1;
	else
	{
		if (h_addrtype == af)
		{
			if (which == SEL_NAME)
			{
				if (string_equal(name, tokens[1])) match = 1;
				else if (h_aliases != NULL)
				{
					for (i = 0; (h_aliases[i] != NULL) && (match == 0); i++)
						if (string_equal(name, h_aliases[i])) match = 1;
				}
			}
			else if (which == SEL_NUMBER)
			{
				if (memcmp(addr, h_addr_list[0], h_length) == 0) match = 1;
			}
		}
	}

	if (match == 0)
	{
		free(tokens);
		return NULL;
	}

	item = NULL;

	if (h_aliases == NULL) h_aliases = &null_alias;

	if (h_addrtype == AF_INET)
	{
		item = (si_item_t *)LI_ils_create("L4488s*44a", (unsigned long)si, CATEGORY_HOST_IPV4, 1, va, vb, tokens[1], h_aliases, h_addrtype, h_length, h_addr_list);
	}
	else
	{
		item = (si_item_t *)LI_ils_create("L4488s*44c", (unsigned long)si, CATEGORY_HOST_IPV6, 1, va, vb, tokens[1], h_aliases, h_addrtype, h_length, h_addr_list);
	}

	free(tokens);

	return item;
}

static void *
_fsi_get_host(si_mod_t *si, const char *name, const void *addr, int af, int which, uint32_t *err)
{
	char *line;
	si_item_t *item;
	FILE *f;
	si_list_t *all;
	uint64_t va, vb;

	if ((which == SEL_NAME) && (name == NULL))
	{
		if (err != NULL) *err = NO_RECOVERY;
		return NULL;
	}

	if ((which == SEL_NUMBER) && (addr == NULL))
	{
		if (err != NULL) *err = NO_RECOVERY;
		return NULL;
	}

	f = fopen(_PATH_HOSTS, "r");
	if (f == NULL)
	{
		if (err != NULL) *err = NO_RECOVERY;
		return NULL;
	}

	_fsi_get_validation(si, VALIDATION_HOSTS, _PATH_HOSTS, f, &va, &vb);

	all = NULL;

	forever
	{
		line = _fsi_get_line(f);
		if (line == NULL) break;

		if (line[0] == '#') 
		{
			free(line);
			line = NULL;
			continue;
		}

		item = _fsi_parse_host(si, name, addr, af, which, line, va, vb);
		free(line);
		line = NULL;

		if (item == NULL) continue;

		if (which == SEL_ALL)
		{
			all = si_list_add(all, item);
			si_item_release(item);
			continue;
		}

		fclose(f);
		return item;
	}

	fclose(f);
	return all;
}

/* SERVICE */

static si_item_t *
_fsi_parse_service(si_mod_t *si, const char *name, const char *proto, int port, int which, char *data, uint64_t va, uint64_t vb)
{
	char **tokens, **s_aliases, *xproto;
	int i, ntokens, match;
	si_item_t *item;
	int xport;

	if (data == NULL) return NULL;

	port = ntohs(port);

	ntokens = 0;
	tokens = _fsi_tokenize(data, " 	", 0, &ntokens);
	if (ntokens < 2)
	{
		free(tokens);
		return NULL;
	}

	s_aliases = NULL;
	if (ntokens > 2) s_aliases = &(tokens[2]);

	xport = atoi(tokens[1]);

	xproto = strchr(tokens[1], '/');

	if (xproto == NULL)
	{
		free(tokens);
		return NULL;
	}

	*xproto++ = '\0';
	if ((proto != NULL) && (string_not_equal(proto, xproto)))
	{
		free(tokens);
		return NULL;
	}

	match = 0;
	if (which == SEL_ALL) match = 1;
	else if (which == SEL_NAME)
	{
		if (string_equal(name, tokens[0])) match = 1;
		else if (s_aliases != NULL)
		{
			for (i = 0; (s_aliases[i] != NULL) && (match == 0); i++)
				if (string_equal(name, s_aliases[i])) match = 1;
		}
	}
	else if ((which == SEL_NUMBER) && (port == xport)) match = 1;

	if (match == 0)
	{
		free(tokens);
		return NULL;
	}

	/* strange but correct */
	xport = htons(xport);

	item = (si_item_t *)LI_ils_create("L4488s*4s", (unsigned long)si, CATEGORY_SERVICE, 1, va, vb, tokens[0], s_aliases, xport, xproto);

	free(tokens);

	return item;
}

static void *
_fsi_get_service(si_mod_t *si, const char *name, const char *proto, int port, int which)
{
	char *p, *line;
	si_item_t *item;
	FILE *f;
	si_list_t *all;
	uint64_t va, vb;

	if ((which == SEL_NAME) && (name == NULL)) return NULL;
	if ((which == SEL_NUMBER) && (port == 0)) return NULL;

	f = fopen(_PATH_SERVICES, "r");
	if (f == NULL) return NULL;

	_fsi_get_validation(si, VALIDATION_SERVICES, _PATH_SERVICES, f, &va, &vb);

	all = NULL;

	forever
	{
		line = _fsi_get_line(f);
		if (line == NULL) break;

		if (line[0] == '#') 
		{
			free(line);
			line = NULL;
			continue;
		}

		p = strchr(line, '#');
		if (p != NULL) *p = '\0';

		item = _fsi_parse_service(si, name, proto, port, which, line, va, vb);
		free(line);
		line = NULL;

		if (item == NULL) continue;

		if (which == SEL_ALL)
		{
			all = si_list_add(all, item);
			si_item_release(item);
			continue;
		}

		fclose(f);
		return item;
	}

	fclose(f);
	return all;
}

/*
 * Generic name/number/aliases lookup
 * Works for protocols, networks, and rpcs
 */

static si_item_t *
_fsi_parse_name_num_aliases(si_mod_t *si, const char *name, int num, int which, char *data, uint64_t va, uint64_t vb, int cat)
{
	char **tokens, **aliases;
	int i, ntokens, match, xnum;
	si_item_t *item;

	if (data == NULL) return NULL;

	ntokens = 0;
	tokens = _fsi_tokenize(data, " 	", 0, &ntokens);
	if (ntokens < 2)
	{
		free(tokens);
		return NULL;
	}

	xnum = atoi(tokens[1]);

	aliases = NULL;
	if (ntokens > 2) aliases = &(tokens[2]);

	match = 0;

	if (which == SEL_ALL) match = 1;
	else if (which == SEL_NAME)
	{
		if (string_equal(name, tokens[0])) match = 1;
		else if (aliases != NULL)
		{
			for (i = 0; (aliases[i] != NULL) && (match == 0); i++)
				if (string_equal(name, aliases[i])) match = 1;
		}
	}
	else if ((which == SEL_NUMBER) && (num == xnum)) match = 1;

	if (match == 0)
	{
		free(tokens);
		return NULL;
	}

	switch (cat) {
		case CATEGORY_NETWORK:
			// struct netent
			item = (si_item_t *)LI_ils_create("L4488s*44", (unsigned long)si, cat, 1, va, vb, tokens[0], aliases, AF_INET, xnum);
			break;
		case CATEGORY_PROTOCOL:
		case CATEGORY_RPC:
			// struct protoent
			// struct rpcent
			item = (si_item_t *)LI_ils_create("L4488s*4", (unsigned long)si, cat, 1, va, vb, tokens[0], aliases, xnum);
			break;
		default:
			abort();
	}

	free(tokens);

	return item;
}

static void *
_fsi_get_name_number_aliases(si_mod_t *si, const char *name, int num, int which, int cat)
{
	char *p, *line;
	si_item_t *item;
	FILE *f;
	si_list_t *all;
	uint64_t va, vb;
	const char *path;
	int vtype;

	switch (cat) {
		case CATEGORY_NETWORK:
			vtype = VALIDATION_NETWORKS;
			path = _PATH_NETWORKS;
			break;
		case CATEGORY_PROTOCOL:
			vtype = VALIDATION_PROTOCOLS;
			path = _PATH_PROTOCOLS;
			break;
		case CATEGORY_RPC:
			vtype = VALIDATION_RPC;
			path = _PATH_RPCS;
			break;
		default:
			abort();
	}

	f = fopen(path, "r");
	if (f == NULL) return NULL;

	_fsi_get_validation(si, vtype, path, f, &va, &vb);

	all = NULL;

	forever
	{
		line = _fsi_get_line(f);
		if (line == NULL) break;

		if (line[0] == '#') 
		{
			free(line);
			line = NULL;
			continue;
		}

		p = strchr(line, '#');
		if (p != NULL) *p = '\0';

		item = _fsi_parse_name_num_aliases(si, name, num, which, line, va, vb, cat);
		free(line);
		line = NULL;

		if (item == NULL) continue;

		if (which == SEL_ALL)
		{
			all = si_list_add(all, item);
			si_item_release(item);
			continue;
		}

		fclose(f);
		return item;
	}

	fclose(f);
	return all;
}

/* MOUNT */

static si_item_t *
_fsi_parse_fs(si_mod_t *si, const char *name, int which, char *data, uint64_t va, uint64_t vb)
{
	char **tokens, *tmp, **opts, *fstype;
	int ntokens, match, i, freq, passno;
	si_item_t *item;

	if (data == NULL) return NULL;

	freq = 0;
	passno = 0;
	fstype = NULL;

	ntokens = 0;
	tokens = _fsi_tokenize(data, " 	", 0, &ntokens);
	if ((ntokens < 4) || (ntokens > 6))
	{
		free(tokens);
		return NULL;
	}

	if (ntokens >= 5) freq = atoi(tokens[4]);
	if (ntokens == 6) passno = atoi(tokens[5]);

	tmp = strdup(tokens[3]);
	if (tmp == NULL)
	{
		free(tokens);
		return NULL;
	}

	ntokens = 0;
	opts = _fsi_tokenize(tmp, ",", 0, &ntokens);

	if (opts == NULL)
	{
		free(tokens); 
		free(tmp);
		return NULL;
	}

	for (i = 0; i < ntokens; i++)
	{
		if ((string_equal(opts[i], "rw")) || (string_equal(opts[i], "ro")) || (string_equal(opts[i], "sw")) || (string_equal(opts[i], "xx")))
		{
			fstype = opts[i];
			break;
		}
	}

	match = 0;

	if (which == SEL_ALL) match = 1;
	else if ((which == SEL_NAME) && (string_equal(name, tokens[0]))) match = 1;
	else if ((which == SEL_NUMBER) && (string_equal(name, tokens[1]))) match = 1;

	if (match == 0)
	{
		free(tokens);
		return NULL;
	}

	item = (si_item_t *)LI_ils_create("L4488sssss44", (unsigned long)si, CATEGORY_FS, 1, va, vb, tokens[0], tokens[1], tokens[2], tokens[3], (fstype == NULL) ? "rw" : fstype, freq, passno);

	free(tokens); 
	free(opts); 
	free(tmp);

	return item;
}

static char *
_fsi_get_device_path(dev_t target_dev)
{
	char *result;
    char dev[PATH_MAX];
    char *name;
	char namebuf[PATH_MAX];

	result = NULL;

    strlcpy(dev, _PATH_DEV, sizeof(dev));

    /* The root device in fstab should always be a block special device */
    name = devname_r(target_dev, S_IFBLK, namebuf, sizeof(namebuf));
    if (name == NULL)
	{
		DIR *dirp;
		struct stat devst;
		struct dirent *ent, entbuf;

       /* No _PATH_DEVDB. We have to search for it the slow way */
        dirp = opendir(_PATH_DEV);
        if (dirp == NULL) return NULL;

        while (readdir_r(dirp, &entbuf, &ent) == 0 && ent != NULL)
		{
            /* Look for a block special device */
            if (ent->d_type == DT_BLK)
			{
                strlcat(dev, ent->d_name, sizeof(dev));
                if (stat(dev, &devst) == 0)
				{
                    if (devst.st_rdev == target_dev) {
						result = strdup(dev);
						break;
					}
                }
            }

            /* reset dev to _PATH_DEV and try again */
            dev[sizeof(_PATH_DEV) - 1] = '\0';
        }

		if (dirp) closedir(dirp);
    }
	else
	{
        /* We found the _PATH_DEVDB entry */
		strlcat(dev, name, sizeof(dev));
		result = strdup(dev);
	}

    return result;
}

static si_item_t *
_fsi_fs_root(si_mod_t *si)
{
	dispatch_once(&rootfs_once, ^{
		struct stat rootstat;
		struct statfs rootfsinfo;
		char *root_spec;
		const char *root_path = "/";

		if (stat(root_path, &rootstat) < 0) return;
 		if (statfs(root_path, &rootfsinfo) < 0) return;

		// Check to make sure we're not looking at a synthetic root:
		if (string_equal(rootfsinfo.f_fstypename, "synthfs")) {
			root_path = "/root";
        		if (stat(root_path, &rootstat) < 0) return;
			if (statfs(root_path, &rootfsinfo) < 0) return;
		}

		root_spec = _fsi_get_device_path(rootstat.st_dev);

		rootfs = (si_item_t *)LI_ils_create("L4488sssss44", (unsigned long)si, CATEGORY_FS, 1, 0LL, 0LL, root_spec, root_path, rootfsinfo.f_fstypename, FSTAB_RW, FSTAB_RW, 0, 1);
	});

	return si_item_retain(rootfs);
}

static void *
_fsi_get_fs(si_mod_t *si, const char *name, int which)
{
	char *line;
	si_item_t *item;
	FILE *f;
	si_list_t *all;
	uint64_t va, vb;
	int synthesize_root;
	struct fstab *rfs;

	if ((which != SEL_ALL) && (name == NULL)) return NULL;

	all = NULL;
	f = NULL;
#ifdef SYNTH_ROOTFS
	synthesize_root = 1;
#else
	synthesize_root = 0;
#endif

	f = fopen(_PATH_FSTAB, "r");
	if ((f == NULL) || (synthesize_root == 1))
	{
		item = _fsi_fs_root(si);

		rfs = NULL;
		if (item != NULL) rfs = (struct fstab *)((uintptr_t)item + sizeof(si_item_t));

		switch (which)
		{
			case SEL_NAME:
			{
				if ((rfs != NULL) && (string_equal(name, rfs->fs_spec)))
				{
					if (f != NULL) fclose(f);
					return item;
				}

				break;
			}

			case SEL_NUMBER:
			{
				if ((rfs != NULL) && (string_equal(name, rfs->fs_file)))
				{
					if (f != NULL) fclose(f);
					return item;
				}

				break;
			}

			case SEL_ALL:
			{
				all = si_list_add(all, item);
				si_item_release(item);
				break;
			}
		}
	}

	if (f == NULL) return all;

	_fsi_get_validation(si, VALIDATION_FSTAB, _PATH_FSTAB, f, &va, &vb);

	forever
	{
		line = _fsi_get_line(f);
		if (line == NULL) break;

		if (line[0] == '#') 
		{
			free(line);
			line = NULL;
			continue;
		}

		item = _fsi_parse_fs(si, name, which, line, va, vb);
		free(line);
		line = NULL;

		if (item == NULL) continue;

		if (which == SEL_ALL)
		{
			all = si_list_add(all, item);
			si_item_release(item);
			continue;
		}

		fclose(f);
		return item;
	}

	fclose(f);
	return all;
}

static int
file_is_valid(si_mod_t *si, si_item_t *item)
{
	si_mod_t *src;

	if (si == NULL) return 0;
	if (item == NULL) return 0;
	if (si->name == NULL) return 0;
	if (item->src == NULL) return 0;

	src = (si_mod_t *)item->src;

	if (src->name == NULL) return 0;
	if (string_not_equal(si->name, src->name)) return 0;

	if (item == rootfs) return 1;

	return _fsi_validate(si, item->type, item->validation_a, item->validation_b);
}

static si_item_t *
file_user_byname(si_mod_t *si, const char *name)
{
	return _fsi_get_user(si, name, 0, SEL_NAME);
}

static si_item_t *
file_user_byuid(si_mod_t *si, uid_t uid)
{
	return _fsi_get_user(si, NULL, uid, SEL_NUMBER);
}

static si_list_t *
file_user_all(si_mod_t *si)
{
	return _fsi_get_user(si, NULL, 0, SEL_ALL);
}

static si_item_t *
file_group_byname(si_mod_t *si, const char *name)
{
	return _fsi_get_group(si, name, 0, SEL_NAME);
}

static si_item_t *
file_group_bygid(si_mod_t *si, gid_t gid)
{
	return _fsi_get_group(si, NULL, gid, SEL_NUMBER);
}

static si_list_t *
file_group_all(si_mod_t *si)
{
	return _fsi_get_group(si, NULL, 0, SEL_ALL);
}

static si_item_t *
file_grouplist(si_mod_t *si, const char *name, __unused uint32_t ignored)
{
	return _fsi_get_grouplist(si, name);
}

static si_list_t *
file_netgroup_byname(si_mod_t *si, const char *name)
{
	si_list_t *list = NULL;
	si_item_t *item;
	uint64_t va=0, vb=0;
	file_netgroup_t *n;
	file_si_private_t *pp;

	if (name == NULL) return NULL;

	pp = (file_si_private_t *)si->private;
	if (pp == NULL) return NULL;

	_fsi_check_netgroup_cache(si);

	pthread_mutex_lock(&file_mutex);

	n = _fsi_find_netgroup(&(pp->file_netgroup_cache), name, 0);
	if (n != NULL)
	{
		file_netgroup_member_t *m = n->members;
		while (m != NULL)
		{
			item = (si_item_t *)LI_ils_create("L4488sss", (unsigned long)si, CATEGORY_NETGROUP, 1, va, vb, m->host, m->user, m->domain);
			list = si_list_add(list, item);
			m = m->next;
		}
	}

	pthread_mutex_unlock(&file_mutex);

	return list;
}

static int
file_in_netgroup(si_mod_t *si, const char *group, const char *host, const char *user, const char *domain)
{
	file_netgroup_t *n;
	file_netgroup_member_t *m;
	file_si_private_t *pp;

	if (group == NULL) return 0;

	pp = (file_si_private_t *)si->private;
	if (pp == NULL) return 0;

	_fsi_check_netgroup_cache(si);

	pthread_mutex_lock(&file_mutex);

	n = _fsi_find_netgroup(&(pp->file_netgroup_cache), group, 0);
	if (n == NULL)
	{
		pthread_mutex_unlock(&file_mutex);
		return 0;
	}

	m = n->members;
	while (m != NULL)
	{
		file_netgroup_member_t *x = m;
		m = m->next;

		if (host != NULL)
		{
			if (x->host == NULL) continue;
			if (strcmp(host, x->host)) continue;
		}

		if (user != NULL)
		{
			if (x->user == NULL) continue;
			if (strcmp(user, x->user)) continue;
		}

		if (domain != NULL)
		{
			if (x->domain == NULL) continue;
			if (strcmp(domain, x->domain)) continue;
		}

		pthread_mutex_unlock(&file_mutex);
		return 1;
	}

	pthread_mutex_unlock(&file_mutex);
	return 0;
}

static si_item_t *
file_host_byname(si_mod_t *si, const char *name, int af, const char *ignored, uint32_t *err)
{
	si_item_t *item;

	if (err != NULL) *err = SI_STATUS_NO_ERROR;

	item = _fsi_get_host(si, name, NULL, af, SEL_NAME, err);
	if ((item == NULL) && (err != NULL) && (*err == 0)) *err = SI_STATUS_H_ERRNO_HOST_NOT_FOUND;

	return item;
}

static si_item_t *
file_host_byaddr(si_mod_t *si, const void *addr, int af, const char *ignored, uint32_t *err)
{
	si_item_t *item;

	if (err != NULL) *err = SI_STATUS_NO_ERROR;

	item = _fsi_get_host(si, NULL, addr, af, SEL_NUMBER, err);
	if ((item == NULL) && (err != NULL) && (*err == 0)) *err = SI_STATUS_H_ERRNO_HOST_NOT_FOUND;

	return item;
}

static si_list_t *
file_host_all(si_mod_t *si)
{
	return _fsi_get_host(si, NULL, NULL, 0, SEL_ALL, NULL);
}

static si_item_t *
file_network_byname(si_mod_t *si, const char *name)
{
	if (name == NULL) return NULL;
	return _fsi_get_name_number_aliases(si, name, 0, SEL_NAME, CATEGORY_NETWORK);
}

static si_item_t *
file_network_byaddr(si_mod_t *si, uint32_t addr)
{
	return _fsi_get_name_number_aliases(si, NULL, (int)addr, SEL_NUMBER, CATEGORY_NETWORK);
}

static si_list_t *
file_network_all(si_mod_t *si)
{
	return _fsi_get_name_number_aliases(si, NULL, 0, SEL_ALL, CATEGORY_NETWORK);
}

static si_item_t *
file_service_byname(si_mod_t *si, const char *name, const char *proto)
{
	return _fsi_get_service(si, name, proto, 0, SEL_NAME);
}

static si_item_t *
file_service_byport(si_mod_t *si, int port, const char *proto)
{
	return _fsi_get_service(si, NULL, proto, port, SEL_NUMBER);
}

static si_list_t *
file_service_all(si_mod_t *si)
{
	return _fsi_get_service(si, NULL, NULL, 0, SEL_ALL);
}

static si_item_t *
file_protocol_byname(si_mod_t *si, const char *name)
{
	if (name == NULL) return NULL;
	return _fsi_get_name_number_aliases(si, name, 0, SEL_NAME, CATEGORY_PROTOCOL);
}

static si_item_t *
file_protocol_bynumber(si_mod_t *si, int number)
{
	return _fsi_get_name_number_aliases(si, NULL, number, SEL_NUMBER, CATEGORY_PROTOCOL);
}

static si_list_t *
file_protocol_all(si_mod_t *si)
{
	return _fsi_get_name_number_aliases(si, NULL, 0, SEL_ALL, CATEGORY_PROTOCOL);
}

static si_item_t *
file_rpc_byname(si_mod_t *si, const char *name)
{
	if (name == NULL) return NULL;
	return _fsi_get_name_number_aliases(si, name, 0, SEL_NAME, CATEGORY_RPC);
}

static si_item_t *
file_rpc_bynumber(si_mod_t *si, int number)
{
	return _fsi_get_name_number_aliases(si, NULL, number, SEL_NUMBER, CATEGORY_RPC);
}

static si_list_t *
file_rpc_all(si_mod_t *si)
{
	return _fsi_get_name_number_aliases(si, NULL, 0, SEL_ALL, CATEGORY_RPC);
}

static si_item_t *
file_fs_byspec(si_mod_t *si, const char *spec)
{
	return _fsi_get_fs(si, spec, SEL_NAME);
}

static si_item_t *
file_fs_byfile(si_mod_t *si, const char *file)
{
	return _fsi_get_fs(si, file, SEL_NUMBER);
}

static si_list_t *
file_fs_all(si_mod_t *si)
{
	return _fsi_get_fs(si, NULL, SEL_ALL);
}

static si_item_t *
file_alias_byname(si_mod_t *si, const char *name)
{
	return _fsi_get_alias(si, name, SEL_NAME);
}

static si_list_t *
file_alias_all(si_mod_t *si)
{
	return _fsi_get_alias(si, NULL, SEL_ALL);
}

static si_item_t *
file_mac_byname(si_mod_t *si, const char *name)
{
	return _fsi_get_ether(si, name, SEL_NAME);
}

static si_item_t *
file_mac_bymac(si_mod_t *si, const char *mac)
{
	return _fsi_get_ether(si, mac, SEL_NUMBER);
}

static si_list_t *
file_mac_all(si_mod_t *si)
{
	return _fsi_get_ether(si, NULL, SEL_ALL);
}

static si_list_t *
file_addrinfo(si_mod_t *si, const void *node, const void *serv, uint32_t family, uint32_t socktype, uint32_t proto, uint32_t flags, const char *interface, uint32_t *err)
{
	if (err != NULL) *err = SI_STATUS_NO_ERROR;
	return _gai_simple(si, node, serv, family, socktype, proto, flags, interface, err);
}

si_mod_t *
si_module_static_file(void)
{
	static const struct si_mod_vtable_s file_vtable =
	{
		.sim_is_valid = &file_is_valid,

		.sim_user_byname = &file_user_byname,
		.sim_user_byuid = &file_user_byuid,
		.sim_user_byuuid = NULL,
		.sim_user_all = &file_user_all,

		.sim_group_byname = &file_group_byname,
		.sim_group_bygid = &file_group_bygid,
		.sim_group_byuuid = NULL,
		.sim_group_all = &file_group_all,

		.sim_grouplist = &file_grouplist,

		.sim_netgroup_byname = &file_netgroup_byname,
		.sim_in_netgroup = &file_in_netgroup,

		.sim_alias_byname = &file_alias_byname,
		.sim_alias_all = &file_alias_all,

		.sim_host_byname = &file_host_byname,
		.sim_host_byaddr = &file_host_byaddr,
		.sim_host_all = &file_host_all,

		.sim_network_byname = &file_network_byname,
		.sim_network_byaddr = &file_network_byaddr,
		.sim_network_all = &file_network_all,

		.sim_service_byname = &file_service_byname,
		.sim_service_byport = &file_service_byport,
		.sim_service_all = &file_service_all,

		.sim_protocol_byname = &file_protocol_byname,
		.sim_protocol_bynumber = &file_protocol_bynumber,
		.sim_protocol_all = &file_protocol_all,

		.sim_rpc_byname = &file_rpc_byname,
		.sim_rpc_bynumber = &file_rpc_bynumber,
		.sim_rpc_all = &file_rpc_all,

		.sim_fs_byspec = &file_fs_byspec,
		.sim_fs_byfile = &file_fs_byfile,
		.sim_fs_all = &file_fs_all,

		.sim_mac_byname = &file_mac_byname,
		.sim_mac_bymac = &file_mac_bymac,
		.sim_mac_all = &file_mac_all,

		.sim_wants_addrinfo = NULL,
		.sim_addrinfo = &file_addrinfo,

		/* no nameinfo support */
		.sim_nameinfo = NULL,
	};

	static si_mod_t si =
	{
		.vers = 1,
		.refcount = 1,
		.flags = SI_MOD_FLAG_STATIC,

		.private = NULL,
		.vtable = &file_vtable,
	};

	static dispatch_once_t once;

	dispatch_once(&once, ^{
		si.name = strdup("file");
		file_si_private_t *pp = calloc(1, sizeof(file_si_private_t));
		if (pp != NULL)
		{
			int i;
			for (i = 0; i < VALIDATION_COUNT; i++) pp->notify_token[i] = -1;

			/* hardwired for now, but we may want to make this configurable someday */
			pp->validation_notify_mask = VALIDATION_MASK_HOSTS | VALIDATION_MASK_SERVICES | VALIDATION_MASK_PROTOCOLS;
		}

		si.private = pp;
	});

	return (si_mod_t *)&si;
}
