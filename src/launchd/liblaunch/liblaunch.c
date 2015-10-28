/*
 * Copyright (c) 2005-2012 Apple Inc. All rights reserved.
 *
 * @APPLE_APACHE_LICENSE_HEADER_START@
 * 
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 *     http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 * 
 * @APPLE_APACHE_LICENSE_HEADER_END@
 */

#include "config.h"
#include "launch.h"
#include "launch_priv.h"
#include "launch_internal.h"
#include "ktrace.h"

#include <mach/mach.h>
#include <libkern/OSByteOrder.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/fcntl.h>
#include <sys/un.h>
#include <sys/uio.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <errno.h>
#include <pwd.h>
#include <assert.h>
#include <uuid/uuid.h>
#include <sys/syscall.h>
#include <dlfcn.h>

#ifdef __LP64__
/* workaround: 5723161 */
#ifndef __DARWIN_ALIGN32
#define	__DARWIN_ALIGN32(x)	(((size_t)(x) + 3) & ~3)
#endif
#undef	CMSG_DATA
#define	CMSG_DATA(cmsg)	\
	((uint8_t *)(cmsg) + __DARWIN_ALIGN32(sizeof(struct cmsghdr)))
#undef	CMSG_SPACE
#define	CMSG_SPACE(l)	\
	(__DARWIN_ALIGN32(sizeof(struct cmsghdr)) + __DARWIN_ALIGN32(l))
#undef	CMSG_LEN
#define	CMSG_LEN(l)	\
	(__DARWIN_ALIGN32(sizeof(struct cmsghdr)) + (l))
#endif

struct _launch_data {
	uint64_t type;
	union {
		struct {
			union {
				launch_data_t *_array;
				char *string;
				void *opaque;
				int64_t __junk;
			};
			union {
				uint64_t _array_cnt;
				uint64_t string_len;
				uint64_t opaque_size;
			};
		};
		int64_t fd;
		uint64_t  mp;
		uint64_t err;
		int64_t number;
		uint64_t boolean; /* We'd use 'bool' but this struct needs to be used under Rosetta, and sizeof(bool) is different between PowerPC and Intel */
		double float_num;
	};
};

#include "bootstrap.h"
#include "vproc.h"
#include "vproc_priv.h"
#include "vproc_internal.h"

/* __OSBogusByteSwap__() must not really exist in the symbol namespace
 * in order for the following to generate an error at build time.
 */
extern void __OSBogusByteSwap__(void);

#define host2wire(x)				\
	({ typeof (x) _X, _x = (x);		\
	 switch (sizeof(_x)) {			\
	 case 8:				\
	 	_X = OSSwapHostToLittleInt64(_x);	\
	 	break;				\
	 case 4:				\
	 	_X = OSSwapHostToLittleInt32(_x);	\
	 	break;				\
	 case 2:				\
	 	_X = OSSwapHostToLittleInt16(_x);	\
	 	break;				\
	 case 1:				\
	 	_X = _x;			\
		break;				\
	 default:				\
	 	__OSBogusByteSwap__();		\
		break;				\
	 }					\
	 _X;					\
	 })


#define big2wire(x)				\
	({ typeof (x) _X, _x = (x);		\
	 switch (sizeof(_x)) {			\
	 case 8:				\
	 	_X = OSSwapLittleToHostInt64(_x);	\
	 	break;				\
	 case 4:				\
	 	_X = OSSwapLittleToHostInt32(_x);	\
	 	break;				\
	 case 2:				\
	 	_X = OSSwapLittleToHostInt16(_x);	\
	 	break;				\
	 case 1:				\
	 	_X = _x;			\
		break;				\
	 default:				\
	 	__OSBogusByteSwap__();		\
		break;				\
	 }					\
	 _X;					\
	 })

union _launch_double_u {
	uint64_t iv;
	double dv;
};

#define host2wire_f(x) ({ \
	typeof(x) _F, _f = (x); \
	union _launch_double_u s; \
	s.dv = _f; \
	s.iv = host2wire(s.iv); \
	_F = s.dv; \
	_F; \
})

#define big2wire_f(x) ({ \
	typeof(x) _F, _f = (x); \
	union _launch_double_u s; \
	s.dv = _f; \
	s.iv = big2wire(s.iv); \
	_F = s.dv; \
	_F; \
})


struct launch_msg_header {
	uint64_t magic;
	uint64_t len;
};

#define LAUNCH_MSG_HEADER_MAGIC 0xD2FEA02366B39A41ull

enum {
	LAUNCHD_USE_CHECKIN_FD,
	LAUNCHD_USE_OTHER_FD,
};
struct _launch {
	void	*sendbuf;
	int	*sendfds;
	void	*recvbuf;
	int	*recvfds;
	size_t	sendlen;
	size_t	sendfdcnt;
	size_t	recvlen;
	size_t	recvfdcnt;
	int which;
	int cifd;
	int	fd;
};

static launch_data_t launch_data_array_pop_first(launch_data_t where);
static int _fd(int fd);
static void launch_client_init(void);
static void launch_msg_getmsgs(launch_data_t m, void *context);
static launch_data_t launch_msg_internal(launch_data_t d);
static void launch_mach_checkin_service(launch_data_t obj, const char *key, void *context);

void
_launch_init_globals(launch_globals_t globals)
{
	pthread_once_t once = PTHREAD_ONCE_INIT;
	globals->lc_once = once;
	pthread_mutex_init(&globals->lc_mtx, NULL);
}

#if !_LIBLAUNCH_HAS_ALLOC_ONCE
launch_globals_t __launch_globals;

void
_launch_globals_init(void)
{
	__launch_globals = calloc(1, sizeof(struct launch_globals_s));
	_launch_init_globals(__launch_globals);
}

launch_globals_t
_launch_globals_impl(void)
{
	static pthread_once_t once = PTHREAD_ONCE_INIT;
	pthread_once(&once, &_launch_globals_init);
	return __launch_globals;
}
#endif

void
launch_client_init(void)
{
	struct sockaddr_un sun;
	char *where = getenv(LAUNCHD_SOCKET_ENV);
	char *_launchd_fd = getenv(LAUNCHD_TRUSTED_FD_ENV);
	int dfd, lfd = -1, cifd = -1;
	name_t spath;

	if (_launchd_fd) {
		cifd = strtol(_launchd_fd, NULL, 10);
		if ((dfd = dup(cifd)) >= 0) {
			close(dfd);
			_fd(cifd);
		} else {
			cifd = -1;
		}
		unsetenv(LAUNCHD_TRUSTED_FD_ENV);
	}

	memset(&sun, 0, sizeof(sun));
	sun.sun_family = AF_UNIX;

	/* The rules are as follows. 
	 * - All users (including root) talk to their per-user launchd's by default.
	 * - If we have been invoked under sudo, talk to the system launchd.
	 * - If we're the root user and the __USE_SYSTEM_LAUNCHD environment variable is set, then
	 *   talk to the system launchd.
	 */
	if (where && where[0] != '\0') {
		strncpy(sun.sun_path, where, sizeof(sun.sun_path));
	} else {
		if (_vprocmgr_getsocket(spath) == 0) {
			if ((getenv("SUDO_COMMAND") || getenv("__USE_SYSTEM_LAUNCHD")) && geteuid() == 0) {
				/* Talk to the system launchd. */
				strncpy(sun.sun_path, LAUNCHD_SOCK_PREFIX "/sock", sizeof(sun.sun_path));
			} else {
				/* Talk to our per-user launchd. */
				size_t min_len;

				min_len = sizeof(sun.sun_path) < sizeof(spath) ? sizeof(sun.sun_path) : sizeof(spath);

				strncpy(sun.sun_path, spath, min_len);
			}
		}
	}

	launch_globals_t globals = _launch_globals();
	if ((lfd = _fd(socket(AF_UNIX, SOCK_STREAM, 0))) == -1) {
		goto out_bad;
	}

#if TARGET_OS_EMBEDDED
	(void)vproc_swap_integer(NULL, VPROC_GSK_EMBEDDEDROOTEQUIVALENT, NULL, &globals->s_am_embedded_god);
#endif
	if (-1 == connect(lfd, (struct sockaddr *)&sun, sizeof(sun))) {
		if (cifd != -1 || globals->s_am_embedded_god) {
			/* There is NO security enforced by this check. This is just a hint to our
			 * library that we shouldn't error out due to failing to open this socket. If
			 * we inherited a trusted file descriptor, we shouldn't fail. This should be
			 * adequate for clients' expectations.
			 */
			close(lfd);
			lfd = -1;
		} else {
			goto out_bad;
		}
	}
	
	if (!(globals->l = launchd_fdopen(lfd, cifd))) {
		goto out_bad;
	}

	if (!(globals->async_resp = launch_data_alloc(LAUNCH_DATA_ARRAY))) {
		goto out_bad;
	}

	return;
out_bad:
	if (globals->l) {
		launchd_close(globals->l, close);
		globals->l = NULL;
	} else if (lfd != -1) {
		close(lfd);
	}
	if (cifd != -1) {
		close(cifd);
	}
}

launch_data_t
launch_data_alloc(launch_data_type_t t)
{
	launch_data_t d = calloc(1, sizeof(struct _launch_data));

	if (d) {
		d->type = t;
		switch (t) {
		case LAUNCH_DATA_DICTIONARY:
		case LAUNCH_DATA_ARRAY:
			d->_array = malloc(0);
			break;
		case LAUNCH_DATA_OPAQUE:
			d->opaque = malloc(0);
		default:
			break;
		}
	}

	return d;
}

launch_data_type_t
launch_data_get_type(launch_data_t d)
{
	return d->type;
}

void
launch_data_free(launch_data_t d)
{
	size_t i;

	switch (d->type) {
	case LAUNCH_DATA_DICTIONARY:
	case LAUNCH_DATA_ARRAY:
		for (i = 0; i < d->_array_cnt; i++) {
			if (d->_array[i]) {
				launch_data_free(d->_array[i]);
			}
		}
		free(d->_array);
		break;
	case LAUNCH_DATA_STRING:
		if (d->string)
			free(d->string);
		break;
	case LAUNCH_DATA_OPAQUE:
		if (d->opaque)
			free(d->opaque);
		break;
	default:
		break;
	}
	free(d);
}

size_t
launch_data_dict_get_count(launch_data_t dict)
{
	return dict->_array_cnt / 2;
}

bool
launch_data_dict_insert(launch_data_t dict, launch_data_t what, const char *key)
{
	size_t i;
	launch_data_t thekey = launch_data_alloc(LAUNCH_DATA_STRING);

	launch_data_set_string(thekey, key);

	for (i = 0; i < dict->_array_cnt; i += 2) {
		if (!strcasecmp(key, dict->_array[i]->string)) {
			launch_data_array_set_index(dict, thekey, i);
			launch_data_array_set_index(dict, what, i + 1);
			return true;
		}
	}
	launch_data_array_set_index(dict, thekey, i);
	launch_data_array_set_index(dict, what, i + 1);
	return true;
}

launch_data_t
launch_data_dict_lookup(launch_data_t dict, const char *key)
{
	size_t i;

	if (LAUNCH_DATA_DICTIONARY != dict->type)
		return NULL;

	for (i = 0; i < dict->_array_cnt; i += 2) {
		if (!strcasecmp(key, dict->_array[i]->string))
			return dict->_array[i + 1];
	}

	return NULL;
}

bool
launch_data_dict_remove(launch_data_t dict, const char *key)
{
	size_t i;

	for (i = 0; i < dict->_array_cnt; i += 2) {
		if (!strcasecmp(key, dict->_array[i]->string))
			break;
	}
	if (i == dict->_array_cnt)
		return false;
	launch_data_free(dict->_array[i]);
	launch_data_free(dict->_array[i + 1]);
	memmove(dict->_array + i, dict->_array + i + 2, (dict->_array_cnt - (i + 2)) * sizeof(launch_data_t));
	dict->_array_cnt -= 2;
	return true;
}

void
launch_data_dict_iterate(launch_data_t dict, void (*cb)(launch_data_t, const char *, void *), void *context)
{
	size_t i;

	if (LAUNCH_DATA_DICTIONARY != dict->type) {
		return;
	}

	for (i = 0; i < dict->_array_cnt; i += 2) {
		cb(dict->_array[i + 1], dict->_array[i]->string, context);
	}
}

bool
launch_data_array_set_index(launch_data_t where, launch_data_t what, size_t ind)
{
	if ((ind + 1) >= where->_array_cnt) {
		where->_array = reallocf(where->_array, (ind + 1) * sizeof(launch_data_t));
		memset(where->_array + where->_array_cnt, 0, (ind + 1 - where->_array_cnt) * sizeof(launch_data_t));
		where->_array_cnt = ind + 1;
	}

	if (where->_array[ind]) {
		launch_data_free(where->_array[ind]);
	}

	where->_array[ind] = what;
	return true;
}

launch_data_t
launch_data_array_get_index(launch_data_t where, size_t ind)
{
	if (LAUNCH_DATA_ARRAY != where->type || ind >= where->_array_cnt) {
		return NULL;
	} else {
		return where->_array[ind];
	}
}

launch_data_t
launch_data_array_pop_first(launch_data_t where)
{
	launch_data_t r = NULL;

	if (where->_array_cnt > 0) {
		r = where->_array[0];
		memmove(where->_array, where->_array + 1, (where->_array_cnt - 1) * sizeof(launch_data_t));
		where->_array_cnt--;
	}
	return r;
}

size_t
launch_data_array_get_count(launch_data_t where)
{
	if (LAUNCH_DATA_ARRAY != where->type)
		return 0;
	return where->_array_cnt;
}

bool
launch_data_set_errno(launch_data_t d, int e)
{
	d->err = e;
	return true;
}

bool
launch_data_set_fd(launch_data_t d, int fd)
{
	d->fd = fd;
	return true;
}

bool
launch_data_set_machport(launch_data_t d, mach_port_t p)
{
	d->mp = p;
	return true;
}

bool
launch_data_set_integer(launch_data_t d, long long n)
{
	d->number = n;
	return true;
}

bool
launch_data_set_bool(launch_data_t d, bool b)
{
	d->boolean = b;
	return true;
}

bool
launch_data_set_real(launch_data_t d, double n)
{
	d->float_num = n;
	return true;
}

bool
launch_data_set_string(launch_data_t d, const char *s)
{
	if (d->string)
		free(d->string);
	d->string = strdup(s);
	if (d->string) {
		d->string_len = strlen(d->string);
		return true;
	}
	return false;
}

bool
launch_data_set_opaque(launch_data_t d, const void *o, size_t os)
{
	d->opaque_size = os;
	if (d->opaque)
		free(d->opaque);
	d->opaque = malloc(os);
	if (d->opaque) {
		memcpy(d->opaque, o, os);
		return true;
	}
	return false;
}

int
launch_data_get_errno(launch_data_t d)
{
	return d->err;
}

int
launch_data_get_fd(launch_data_t d)
{
	return d->fd;
}

mach_port_t
launch_data_get_machport(launch_data_t d)
{
	return d->mp;
}

long long
launch_data_get_integer(launch_data_t d)
{
	return d->number;
}

bool
launch_data_get_bool(launch_data_t d)
{
	return d->boolean;
}

double
launch_data_get_real(launch_data_t d)
{
	return d->float_num;
}

const char *
launch_data_get_string(launch_data_t d)
{
	if (LAUNCH_DATA_STRING != d->type)
		return NULL;
	return d->string;
}

void *
launch_data_get_opaque(launch_data_t d)
{
	if (LAUNCH_DATA_OPAQUE != d->type)
		return NULL;
	return d->opaque;
}

size_t
launch_data_get_opaque_size(launch_data_t d)
{
	return d->opaque_size;
}

int
launchd_getfd(launch_t l)
{
	return (l->which == LAUNCHD_USE_CHECKIN_FD) ? l->cifd : l->fd;
}

launch_t
launchd_fdopen(int fd, int cifd)
{
	launch_t c;

	c = calloc(1, sizeof(struct _launch));
	if (!c)
		return NULL;

	c->fd = fd;
	c->cifd = cifd;

	if (c->fd == -1 || (c->fd != -1 && c->cifd != -1)) {
		c->which = LAUNCHD_USE_CHECKIN_FD;
	} else if (c->cifd == -1) {
		c->which = LAUNCHD_USE_OTHER_FD;
	}

	fcntl(fd, F_SETFL, O_NONBLOCK);
	fcntl(cifd, F_SETFL, O_NONBLOCK);

	if ((c->sendbuf = malloc(0)) == NULL)
		goto out_bad;
	if ((c->sendfds = malloc(0)) == NULL)
		goto out_bad;
	if ((c->recvbuf = malloc(0)) == NULL)
		goto out_bad;
	if ((c->recvfds = malloc(0)) == NULL)
		goto out_bad;

	return c;

out_bad:
	if (c->sendbuf)
		free(c->sendbuf);
	if (c->sendfds)
		free(c->sendfds);
	if (c->recvbuf)
		free(c->recvbuf);
	if (c->recvfds)
		free(c->recvfds);
	free(c);
	return NULL;
}

void
launchd_close(launch_t lh, typeof(close) closefunc)
{
	launch_globals_t globals = _launch_globals();

	if (globals->in_flight_msg_recv_client == lh) {
		globals->in_flight_msg_recv_client = NULL;
	}

	if (lh->sendbuf)
		free(lh->sendbuf);
	if (lh->sendfds)
		free(lh->sendfds);
	if (lh->recvbuf)
		free(lh->recvbuf);
	if (lh->recvfds)
		free(lh->recvfds);
	closefunc(lh->fd);
	closefunc(lh->cifd);
	free(lh);
}

#define ROUND_TO_64BIT_WORD_SIZE(x)	((x + 7) & ~7)

size_t
launch_data_pack(launch_data_t d, void *where, size_t len, int *fd_where, size_t *fd_cnt)
{
	launch_data_t o_in_w = where;
	size_t i, rsz, node_data_len = sizeof(struct _launch_data);

	if (node_data_len > len) {
		return 0;
	}

	where += node_data_len;

	o_in_w->type = host2wire(d->type);

	size_t pad_len = 0;
	switch (d->type) {
	case LAUNCH_DATA_INTEGER:
		o_in_w->number = host2wire(d->number);
		break;
	case LAUNCH_DATA_REAL:
		o_in_w->float_num = host2wire_f(d->float_num);
		break;
	case LAUNCH_DATA_BOOL:
		o_in_w->boolean = host2wire(d->boolean);
		break;
	case LAUNCH_DATA_ERRNO:
		o_in_w->err = host2wire(d->err);
		break;
	case LAUNCH_DATA_FD:
		o_in_w->fd = host2wire(d->fd);
		if (fd_where && d->fd != -1) {
			fd_where[*fd_cnt] = d->fd;
			(*fd_cnt)++;
		}
		break;
	case LAUNCH_DATA_STRING:
		o_in_w->string_len = host2wire(d->string_len);
		node_data_len += ROUND_TO_64BIT_WORD_SIZE(d->string_len + 1);

		if (node_data_len > len) {
			return 0;
		}
		memcpy(where, d->string, d->string_len + 1);

		/* Zero padded data. */
		pad_len = ROUND_TO_64BIT_WORD_SIZE(d->string_len + 1) - (d->string_len + 1);
		bzero(where + d->string_len + 1, pad_len);

		break;
	case LAUNCH_DATA_OPAQUE:
		o_in_w->opaque_size = host2wire(d->opaque_size);
		node_data_len += ROUND_TO_64BIT_WORD_SIZE(d->opaque_size);
		if (node_data_len > len) {
			return 0;
		}
		memcpy(where, d->opaque, d->opaque_size);

		/* Zero padded data. */
		pad_len = ROUND_TO_64BIT_WORD_SIZE(d->opaque_size) - d->opaque_size;
		bzero(where + d->opaque_size, pad_len);

		break;
	case LAUNCH_DATA_DICTIONARY:
	case LAUNCH_DATA_ARRAY:
		o_in_w->_array_cnt = host2wire(d->_array_cnt);
		node_data_len += d->_array_cnt * sizeof(uint64_t);
		if (node_data_len > len) {
			return 0;
		}

		where += d->_array_cnt * sizeof(uint64_t);

		for (i = 0; i < d->_array_cnt; i++) {
			rsz = launch_data_pack(d->_array[i], where, len - node_data_len, fd_where, fd_cnt);
			if (rsz == 0) {
				return 0;
			}
			where += rsz;
			node_data_len += rsz;
		}
		break;
	default:
		break;
	}

	return node_data_len;
}

launch_data_t
launch_data_unpack(void *data, size_t data_size, int *fds, size_t fd_cnt, size_t *data_offset, size_t *fdoffset)
{
	launch_data_t r = data + *data_offset;
	size_t i, tmpcnt;

	//Check for integer underflow
	if (data_size < *data_offset)
		return NULL;

	if ((data_size - *data_offset) < sizeof(struct _launch_data))
		return NULL;
	*data_offset += sizeof(struct _launch_data);

	switch (big2wire(r->type)) {
	case LAUNCH_DATA_DICTIONARY:
	case LAUNCH_DATA_ARRAY:
		tmpcnt = big2wire(r->_array_cnt);

		//Check for integer overflows
		if (tmpcnt > SIZE_MAX / sizeof(uint64_t)) {
			errno = EAGAIN;
			return NULL;
		}

		if ((data_size - *data_offset) < (tmpcnt * sizeof(uint64_t))) {
			errno = EAGAIN;
			return NULL;
		}
		r->_array = data + *data_offset;
		*data_offset += tmpcnt * sizeof(uint64_t);
		for (i = 0; i < tmpcnt; i++) {
			r->_array[i] = launch_data_unpack(data, data_size, fds, fd_cnt, data_offset, fdoffset);
			if (r->_array[i] == NULL)
				return NULL;
		}
		r->_array_cnt = tmpcnt;
		break;
	case LAUNCH_DATA_STRING:
		tmpcnt = big2wire(r->string_len);
		if ((data_size - *data_offset) < (tmpcnt + 1)) {
			errno = EAGAIN;
			return NULL;
		}
		r->string = data + *data_offset;
		r->string_len = tmpcnt;
		*data_offset += ROUND_TO_64BIT_WORD_SIZE(tmpcnt + 1);
		break;
	case LAUNCH_DATA_OPAQUE:
		tmpcnt = big2wire(r->opaque_size);
		if ((data_size - *data_offset) < tmpcnt) {
			errno = EAGAIN;
			return NULL;
		}
		r->opaque = data + *data_offset;
		r->opaque_size = tmpcnt;
		*data_offset += ROUND_TO_64BIT_WORD_SIZE(tmpcnt);
		break;
	case LAUNCH_DATA_FD:
		if (r->fd != -1 && fd_cnt > *fdoffset) {
			r->fd = _fd(fds[*fdoffset]);
			*fdoffset += 1;
		}
		break;
	case LAUNCH_DATA_INTEGER:
		r->number = big2wire(r->number);
		break;
	case LAUNCH_DATA_REAL:
		r->float_num = big2wire_f(r->float_num);
		break;
	case LAUNCH_DATA_BOOL:
		r->boolean = big2wire(r->boolean);
		break;
	case LAUNCH_DATA_ERRNO:
		r->err = big2wire(r->err);
	case LAUNCH_DATA_MACHPORT:
		break;
	default:
		errno = EINVAL;
		return NULL;
		break;
	}

	r->type = big2wire(r->type);

	return r;
}

int
launchd_msg_send(launch_t lh, launch_data_t d)
{
	struct launch_msg_header lmh;
	struct cmsghdr *cm = NULL;
	struct msghdr mh;
	struct iovec iov[2];
	size_t sentctrllen = 0;
	int r;

	int fd2use = launchd_getfd(lh);
	if (fd2use == -1) {
		errno = EPERM;
		return -1;
	}

	memset(&mh, 0, sizeof(mh));

	/* confirm that the next hack works */
	assert((d && lh->sendlen == 0) || (!d && lh->sendlen));

	if (d) {
		size_t fd_slots_used = 0;
		size_t good_enough_size = 10 * 1024 * 1024;
		uint64_t msglen;

		/* hack, see the above assert to verify "correctness" */
		free(lh->sendbuf);
		lh->sendbuf = malloc(good_enough_size);
		if (!lh->sendbuf) {
			errno = ENOMEM;
			return -1;
		}

		free(lh->sendfds);
		lh->sendfds = malloc(4 * 1024);
		if (!lh->sendfds) {
			free(lh->sendbuf);
			lh->sendbuf = NULL;
			errno = ENOMEM;
			return -1;
		}

		lh->sendlen = launch_data_pack(d, lh->sendbuf, good_enough_size, lh->sendfds, &fd_slots_used);

		if (lh->sendlen == 0) {
			errno = ENOMEM;
			return -1;
		}

		lh->sendfdcnt = fd_slots_used;

		msglen = lh->sendlen + sizeof(struct launch_msg_header); /* type promotion to make the host2wire() macro work right */
		lmh.len = host2wire(msglen);
		lmh.magic = host2wire(LAUNCH_MSG_HEADER_MAGIC);

		iov[0].iov_base = &lmh;
		iov[0].iov_len = sizeof(lmh);
		mh.msg_iov = iov;
		mh.msg_iovlen = 2;
	} else {
		mh.msg_iov = iov + 1;
		mh.msg_iovlen = 1;
	}

	iov[1].iov_base = lh->sendbuf;
	iov[1].iov_len = lh->sendlen;


	if (lh->sendfdcnt > 0) {
		sentctrllen = mh.msg_controllen = CMSG_SPACE(lh->sendfdcnt * sizeof(int));
		cm = alloca(mh.msg_controllen);
		mh.msg_control = cm;

		memset(cm, 0, mh.msg_controllen);

		cm->cmsg_len = CMSG_LEN(lh->sendfdcnt * sizeof(int));
		cm->cmsg_level = SOL_SOCKET;
		cm->cmsg_type = SCM_RIGHTS;

		memcpy(CMSG_DATA(cm), lh->sendfds, lh->sendfdcnt * sizeof(int));
	}

	if ((r = sendmsg(fd2use, &mh, 0)) == -1) {
		return -1;
	} else if (r == 0) {
		errno = ECONNRESET;
		return -1;
	} else if (sentctrllen != mh.msg_controllen) {
		errno = ECONNRESET;
		return -1;
	}

	if (d) {
		r -= sizeof(struct launch_msg_header);
	}

	lh->sendlen -= r;
	if (lh->sendlen > 0) {
		memmove(lh->sendbuf, lh->sendbuf + r, lh->sendlen);
	} else {
		free(lh->sendbuf);
		lh->sendbuf = malloc(0);
	}

	lh->sendfdcnt = 0;
	free(lh->sendfds);
	lh->sendfds = malloc(0);

	if (lh->sendlen > 0) {
		errno = EAGAIN;
		return -1;
	}

	return 0;
}

int
launch_get_fd(void)
{
	launch_globals_t globals = _launch_globals();
	pthread_once(&globals->lc_once, launch_client_init);

	if (!globals->l) {
		errno = ENOTCONN;
		return -1;
	}

	return globals->l->fd;
}

void
launch_msg_getmsgs(launch_data_t m, void *context)
{
	launch_data_t async_resp, *sync_resp = context;

	launch_globals_t globals = _launch_globals();

	if ((LAUNCH_DATA_DICTIONARY == launch_data_get_type(m)) && (async_resp = launch_data_dict_lookup(m, LAUNCHD_ASYNC_MSG_KEY))) {
		launch_data_array_set_index(globals->async_resp, launch_data_copy(async_resp), launch_data_array_get_count(globals->async_resp));
	} else {
		*sync_resp = launch_data_copy(m);
	}
}

void
launch_mach_checkin_service(launch_data_t obj, const char *key, void *context __attribute__((unused)))
{
	kern_return_t result;
	mach_port_t p;
	name_t srvnm;

	strlcpy(srvnm, key, sizeof(srvnm));

	result = bootstrap_check_in(bootstrap_port, srvnm, &p);

	if (result == BOOTSTRAP_SUCCESS)
		launch_data_set_machport(obj, p);
}

launch_data_t
launch_msg(launch_data_t d)
{
	launch_data_t mps, r = launch_msg_internal(d);

	if (launch_data_get_type(d) == LAUNCH_DATA_STRING) {
		if (strcmp(launch_data_get_string(d), LAUNCH_KEY_CHECKIN) != 0)
			return r;
		if (r == NULL)
			return r;
		if (launch_data_get_type(r) != LAUNCH_DATA_DICTIONARY)
			return r;
		mps = launch_data_dict_lookup(r, LAUNCH_JOBKEY_MACHSERVICES);
		if (mps == NULL)
			return r;
		launch_data_dict_iterate(mps, launch_mach_checkin_service, NULL);
	}

	return r;
}

extern kern_return_t vproc_mig_set_security_session(mach_port_t, uuid_t, mach_port_t);

static inline bool
uuid_data_is_null(launch_data_t d)
{
	bool result = false;
	if (launch_data_get_type(d) == LAUNCH_DATA_OPAQUE && launch_data_get_opaque_size(d) == sizeof(uuid_t)) {
		uuid_t existing_uuid;
		memcpy(existing_uuid, launch_data_get_opaque(d), sizeof(uuid_t));

		/* A NULL UUID tells us to keep the session inherited from the parent. */
		result = (bool)uuid_is_null(existing_uuid);
	}

	return result;
}

launch_data_t
launch_msg_internal(launch_data_t d)
{
	launch_data_t resp = NULL;

	if (d && (launch_data_get_type(d) == LAUNCH_DATA_STRING)
			&& (strcmp(launch_data_get_string(d), LAUNCH_KEY_GETJOBS) == 0)
			&& vproc_swap_complex(NULL, VPROC_GSK_ALLJOBS, NULL, &resp) == NULL) {
		return resp;
	}

	launch_globals_t globals = _launch_globals();
	pthread_once(&globals->lc_once, launch_client_init);
	if (!globals->l) {
		errno = ENOTCONN;
		return NULL;
	}

	int fd2use = -1;
	if ((launch_data_get_type(d) == LAUNCH_DATA_STRING && strcmp(launch_data_get_string(d), LAUNCH_KEY_CHECKIN) == 0) || globals->s_am_embedded_god) {
		globals->l->which = LAUNCHD_USE_CHECKIN_FD;
	} else {
		globals->l->which = LAUNCHD_USE_OTHER_FD;
	}

	fd2use = launchd_getfd(globals->l);

	if (fd2use == -1) {
		errno = EPERM;
		return NULL;
	}

#if !TARGET_OS_EMBEDDED
	uuid_t uuid;
	launch_data_t uuid_d = NULL;
	size_t jobs_that_need_sessions = 0;
	if (d && launch_data_get_type(d) == LAUNCH_DATA_DICTIONARY) {
		launch_data_t v = launch_data_dict_lookup(d, LAUNCH_KEY_SUBMITJOB);

		if (v && launch_data_get_type(v) == LAUNCH_DATA_ARRAY) {
			size_t cnt = launch_data_array_get_count(v);
			size_t i = 0;

			uuid_generate(uuid);
			for (i = 0; i < cnt; i++) {
				launch_data_t ji = launch_data_array_get_index(v, i);
				if (launch_data_get_type(ji) == LAUNCH_DATA_DICTIONARY) {
					launch_data_t existing_v = launch_data_dict_lookup(ji, LAUNCH_JOBKEY_SECURITYSESSIONUUID);
					if (!existing_v) {
						/* I really wish these were reference-counted. Sigh... */
						uuid_d = launch_data_new_opaque(uuid, sizeof(uuid));
						launch_data_dict_insert(ji, uuid_d, LAUNCH_JOBKEY_SECURITYSESSIONUUID);
						jobs_that_need_sessions++;
					} else if (launch_data_get_type(existing_v) == LAUNCH_DATA_OPAQUE) {
						jobs_that_need_sessions += uuid_data_is_null(existing_v) ? 0 : 1;
					}
				}
			}
		} else if (v && launch_data_get_type(v) == LAUNCH_DATA_DICTIONARY) {
			launch_data_t existing_v = launch_data_dict_lookup(v, LAUNCH_JOBKEY_SECURITYSESSIONUUID);
			if (!existing_v) {
				uuid_generate(uuid);
				uuid_d = launch_data_new_opaque(uuid, sizeof(uuid));
				launch_data_dict_insert(v, uuid_d, LAUNCH_JOBKEY_SECURITYSESSIONUUID);
				jobs_that_need_sessions++;
			} else {
				jobs_that_need_sessions += uuid_data_is_null(existing_v) ? 0 : 1;
			}
		}
	}
#endif

	pthread_mutex_lock(&globals->lc_mtx);

	if (d && launchd_msg_send(globals->l, d) == -1) {
		do {
			if (errno != EAGAIN)
				goto out;
		} while (launchd_msg_send(globals->l, NULL) == -1);
	}

	while (resp == NULL) {
		if (d == NULL && launch_data_array_get_count(globals->async_resp) > 0) {
			resp = launch_data_array_pop_first(globals->async_resp);
			goto out;
		}
		if (launchd_msg_recv(globals->l, launch_msg_getmsgs, &resp) == -1) {
			if (errno != EAGAIN) {
				goto out;
			} else if (d == NULL) {
				errno = 0;
				goto out;
			} else {
				fd_set rfds;

				FD_ZERO(&rfds);
				FD_SET(fd2use, &rfds);

				select(fd2use + 1, &rfds, NULL, NULL, NULL);
			}
		}
	}

out:
#if !TARGET_OS_EMBEDDED
	if (!uuid_is_null(uuid) && resp && jobs_that_need_sessions > 0) {
		mach_port_t session_port = _audit_session_self();
		launch_data_type_t resp_type = launch_data_get_type(resp);

		bool set_session = false;
		if (resp_type == LAUNCH_DATA_ERRNO) {
			set_session = (launch_data_get_errno(resp) == ENEEDAUTH);
		} else if (resp_type == LAUNCH_DATA_ARRAY) {
			set_session = true;
		}

		kern_return_t kr = KERN_FAILURE;
		if (set_session) {
			kr = vproc_mig_set_security_session(bootstrap_port, uuid, session_port);
		}

		if (kr == KERN_SUCCESS) {
			if (resp_type == LAUNCH_DATA_ERRNO) {
				launch_data_set_errno(resp, 0);
			} else {
				size_t i = 0;
				for (i = 0; i < launch_data_array_get_count(resp); i++) {
					launch_data_t ri = launch_data_array_get_index(resp, i);

					int recvd_err = 0;
					if (launch_data_get_type(ri) == LAUNCH_DATA_ERRNO && (recvd_err = launch_data_get_errno(ri))) {
						launch_data_set_errno(ri, recvd_err == ENEEDAUTH ? 0 : recvd_err);
					}
				}
			}
		}

		mach_port_deallocate(mach_task_self(), session_port);
	}
#endif

	pthread_mutex_unlock(&globals->lc_mtx);

	return resp;
}

int
launchd_msg_recv(launch_t lh, void (*cb)(launch_data_t, void *), void *context)
{
	struct cmsghdr *cm = alloca(4096); 
	launch_data_t rmsg = NULL;
	size_t data_offset, fd_offset;
	struct msghdr mh;
	struct iovec iov;
	int r;

	int fd2use = launchd_getfd(lh);
	if (fd2use == -1) {
		errno = EPERM;
		return -1;
	}

	memset(&mh, 0, sizeof(mh));
	mh.msg_iov = &iov;
	mh.msg_iovlen = 1;

	lh->recvbuf = reallocf(lh->recvbuf, lh->recvlen + 8*1024);

	iov.iov_base = lh->recvbuf + lh->recvlen;
	iov.iov_len = 8*1024;
	mh.msg_control = cm;
	mh.msg_controllen = 4096;

	if ((r = recvmsg(fd2use, &mh, 0)) == -1)
		return -1;
	if (r == 0) {
		errno = ECONNRESET;
		return -1;
	}
	if (mh.msg_flags & MSG_CTRUNC) {
		errno = ECONNABORTED;
		return -1;
	}
	lh->recvlen += r;
	if (mh.msg_controllen > 0) {
		lh->recvfds = reallocf(lh->recvfds, lh->recvfdcnt * sizeof(int) + mh.msg_controllen - sizeof(struct cmsghdr));
		memcpy(lh->recvfds + lh->recvfdcnt, CMSG_DATA(cm), mh.msg_controllen - sizeof(struct cmsghdr));
		lh->recvfdcnt += (mh.msg_controllen - sizeof(struct cmsghdr)) / sizeof(int);
	}

	r = 0;

	while (lh->recvlen > 0) {
		struct launch_msg_header *lmhp = lh->recvbuf;
		uint64_t tmplen;
		data_offset = sizeof(struct launch_msg_header);
		fd_offset = 0;

		if (lh->recvlen < sizeof(struct launch_msg_header))
			goto need_more_data;

		tmplen = big2wire(lmhp->len);

		if (big2wire(lmhp->magic) != LAUNCH_MSG_HEADER_MAGIC || tmplen <= sizeof(struct launch_msg_header)) {
			errno = EBADRPC;
			goto out_bad;
		}

		if (lh->recvlen < tmplen) {
			goto need_more_data;
		}

		if ((rmsg = launch_data_unpack(lh->recvbuf, lh->recvlen, lh->recvfds, lh->recvfdcnt, &data_offset, &fd_offset)) == NULL) {
			errno = EBADRPC;
			goto out_bad;
		}

		launch_globals_t globals = _launch_globals();

		globals->in_flight_msg_recv_client = lh;

		cb(rmsg, context);

		/* launchd and only launchd can call launchd_close() as a part of the callback */
		if (globals->in_flight_msg_recv_client == NULL) {
			r = 0;
			break;
		}

		lh->recvlen -= data_offset;
		if (lh->recvlen > 0) {
			memmove(lh->recvbuf, lh->recvbuf + data_offset, lh->recvlen);
		} else {
			free(lh->recvbuf);
			lh->recvbuf = malloc(0);
		}

		lh->recvfdcnt -= fd_offset;
		if (lh->recvfdcnt > 0) {
			memmove(lh->recvfds, lh->recvfds + fd_offset, lh->recvfdcnt * sizeof(int));
		} else {
			free(lh->recvfds);
			lh->recvfds = malloc(0);
		}
	}

	return r;

need_more_data:
	errno = EAGAIN;
out_bad:
	return -1;
}

launch_data_t
launch_data_copy(launch_data_t o)
{
	launch_data_t r = launch_data_alloc(o->type);
	size_t i;

	free(r->_array);
	memcpy(r, o, sizeof(struct _launch_data));

	switch (o->type) {
	case LAUNCH_DATA_DICTIONARY:
	case LAUNCH_DATA_ARRAY:
		r->_array = calloc(1, o->_array_cnt * sizeof(launch_data_t));
		for (i = 0; i < o->_array_cnt; i++) {
			if (o->_array[i])
				r->_array[i] = launch_data_copy(o->_array[i]);
		}
		break;
	case LAUNCH_DATA_STRING:
		r->string = strdup(o->string);
		break;
	case LAUNCH_DATA_OPAQUE:
		r->opaque = malloc(o->opaque_size);
		memcpy(r->opaque, o->opaque, o->opaque_size);
		break;
	default:
		break;
	}

	return r;
}

int
_fd(int fd)
{
	if (fd >= 0)
		fcntl(fd, F_SETFD, 1);
	return fd;
}

launch_data_t
launch_data_new_errno(int e)
{
	launch_data_t r = launch_data_alloc(LAUNCH_DATA_ERRNO);

	if (r)
		launch_data_set_errno(r, e);

	return r;
}

launch_data_t
launch_data_new_fd(int fd)
{
	launch_data_t r = launch_data_alloc(LAUNCH_DATA_FD);

	if (r)
		launch_data_set_fd(r, fd);

	return r;
}

launch_data_t
launch_data_new_machport(mach_port_t p)
{
	launch_data_t r = launch_data_alloc(LAUNCH_DATA_MACHPORT);

	if (r)
		launch_data_set_machport(r, p);

	return r;
}

launch_data_t
launch_data_new_integer(long long n)
{
	launch_data_t r = launch_data_alloc(LAUNCH_DATA_INTEGER);

	if (r)
		launch_data_set_integer(r, n);

	return r;
}

launch_data_t
launch_data_new_bool(bool b)
{
	launch_data_t r = launch_data_alloc(LAUNCH_DATA_BOOL);

	if (r)
		launch_data_set_bool(r, b);

	return r;
}

launch_data_t
launch_data_new_real(double d)
{
	launch_data_t r = launch_data_alloc(LAUNCH_DATA_REAL);

	if (r)
		launch_data_set_real(r, d);

	return r;
}

launch_data_t
launch_data_new_string(const char *s)
{
	launch_data_t r = launch_data_alloc(LAUNCH_DATA_STRING);

	if (r == NULL)
		return NULL;

	if (!launch_data_set_string(r, s)) {
		launch_data_free(r);
		return NULL;
	}

	return r;
}

launch_data_t
launch_data_new_opaque(const void *o, size_t os)
{
	launch_data_t r = launch_data_alloc(LAUNCH_DATA_OPAQUE);

	if (r == NULL)
		return NULL;

	if (!launch_data_set_opaque(r, o, os)) {
		launch_data_free(r);
		return NULL;
	}

	return r;
}

void
load_launchd_jobs_at_loginwindow_prompt(int flags __attribute__((unused)), ...)
{
	_vprocmgr_init(VPROCMGR_SESSION_LOGINWINDOW);
}

pid_t
create_and_switch_to_per_session_launchd(const char *login __attribute__((unused)), int flags, ...)
{
	uid_t target_user = geteuid() ? geteuid() : getuid();
	if (_vprocmgr_move_subset_to_user(target_user, VPROCMGR_SESSION_AQUA, flags)) {
		return -1;
	}

	return 1;
}
