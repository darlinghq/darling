/*
 * Copyright (c) 1987, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#if defined(LIBC_SCCS) && !defined(lint)
static char sccsid[] = "@(#)setenv.c	8.1 (Berkeley) 6/4/93";
#endif /* LIBC_SCCS and not lint */
#include <sys/cdefs.h>
__FBSDID("$FreeBSD: src/lib/libc/stdlib/setenv.c,v 1.14 2007/05/01 16:02:41 ache Exp $");

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <crt_externs.h>
#include <errno.h>
#include <sys/types.h>
#include <fcntl.h>

struct owned_ptr;
__private_extern__ char *__findenv(const char *, int *, char **);
__private_extern__ int __setenv(const char *, const char *, int, int, char ***, struct owned_ptr *);
__private_extern__ void __unsetenv(const char *, char **, struct owned_ptr *);

__private_extern__ struct owned_ptr *__env_owned;
__private_extern__ int __init__env_owned(int);

/*
 * _cthread_init_routine used to be called from crt1.o to initialize threads.
 * This is no longer needed, as initialization happens in dylib initializers,
 * but is provided to maintain backwards compatibility.  Normally, for 10.5
 * or greater, _cthread_init_routine does nothing.
 *
 * Before 10.5, the _start routine in crt1.o clobbers environ with the original
 * stack value, which effectively undoes any environment changes made in
 * initializers.  When LEGACY_CRT1_ENVIRON is defined, we replace the
 * do-nothing routine with one that attempts to restore the environ value.
 * But this only works if the setenv (and family) routines were used
 * exclusively, (no direct manipulation of environ).  Note that according to
 * SUSv3, direct manipulation of environ may result in undefined behavior in
 * setenv and family, so we don't support that (on less than 10.5).
 */
#ifdef BUILDING_VARIANT
#  ifdef LEGACY_CRT1_ENVIRON
extern char **_saved_environ;
#  endif /* LEGACY_CRT1_ENVIRON */
#else /* !BUILDING_VARIANT */
#  ifdef LEGACY_CRT1_ENVIRON
__private_extern__ char **_saved_environ = NULL;

static int
_legacy_crt1_environ(void)
{
	if (_saved_environ) *_NSGetEnviron() = _saved_environ;
	return 0;
}
int (*_cthread_init_routine)(void) = _legacy_crt1_environ;

#  else /* !LEGACY_CRT1_ENVIRON */
static int _do_nothing(void) { return 0; }
int (*_cthread_init_routine)(void) = _do_nothing;
#  endif /* !LEGACY_CRT1_ENVIRON */

__private_extern__ struct owned_ptr *__env_owned = NULL;

/*
 * The owned_ptr structure is a table of pointers that we own, and can
 * realloc/free as we choose.  table[0] is always NULL, so it is always
 * less that any real pointer.  "used" is the number of table entries in use
 * (including the initial NULL), while "size" is the allocated size of the
 * table (used to enlarge the size of the table when needed).
 */
struct owned_ptr {
    const void **table;
    int used;
    int size;
};

#define OWNED_PTR_INITIAL_SIZE	8

__private_extern__ int _owned_ptr_search(struct owned_ptr * __restrict, const void * __restrict, int * __restrict);

__private_extern__ void
_owned_ptr_add(struct owned_ptr * __restrict owned, const void * __restrict ptr)
{
	int index;

	if (_owned_ptr_search(owned, ptr, &index) == 0) return; /* already there */
	if (owned->used >= owned->size) {
		int new_size = 2 * owned->size;
		const void **new_table = (const void **)realloc(owned->table,
					 new_size * sizeof(const void *));
		if (!new_table) {
			/* no memory to enlarge the table, so just drop */
			return;
		}
		owned->table = new_table;
		owned->size = new_size;
	}
	memmove(owned->table + index + 2, owned->table + index + 1,
		sizeof(void *) * (owned->used - index - 1));
	owned->table[index + 1] = ptr;
	owned->used++;
}

__private_extern__ struct owned_ptr *
_owned_ptr_alloc(void)
{
	struct owned_ptr *owned;

	owned = (struct owned_ptr *)malloc(sizeof(struct owned_ptr));
	if (!owned) return NULL;
	owned->table = (const void **)malloc(OWNED_PTR_INITIAL_SIZE *
					     sizeof(const void *));
	if (!owned->table) {
		int save = errno;
		free(owned);
		errno = save;
		return NULL;
	}
	owned->table[0] = NULL;
	owned->used = 1;
	owned->size = OWNED_PTR_INITIAL_SIZE;
	return owned;
}

__private_extern__ void
_owned_ptr_delete(struct owned_ptr *owned, int index)
{
	if (!index || index >= owned->used) return;
	memmove(owned->table + index, owned->table + index + 1,
		sizeof(void *) * (owned->used - index - 1));
	owned->used--;
}

__private_extern__ void
_owned_ptr_free(struct owned_ptr *owned)
{
	free(owned->table);
	free(owned);
}

/*
 * Search owned->table for "ptr".  Zero is returned if found, non-zero means
 * not found.  If "result" is non-NULL, the found index is returned in it, or
 * if not found, the index of the immediate lower value (ptr can be inserted
 * after this index).
 */
__private_extern__ int
_owned_ptr_search(struct owned_ptr * __restrict owned, const void * __restrict ptr, int * __restrict result)
{
	int low = 0;
	int high = owned->used - 1;
	int cur;

	if (owned->table[high] < ptr) {
		if (result) *result = high;
		return -1;
	} else if (owned->table[high] == ptr) {
		if (result) *result = high;
		return 0;
	}
	while (high - low > 1) {
		cur = (low + high) / 2;
		if (ptr > owned->table[cur]) {
			low = cur;
		} else if (ptr < owned->table[cur]) {
			high = cur;
		} else {
			/* match found */
			if (result) *result = cur;
			return 0;
		}
	}
	/* no match found; *result will be the insert-after position */
	if (result) *result = low;
	return -1;
}

/*
 * Initialize the process's __env_owned structure
 */
__private_extern__ int
__init__env_owned(int should_set_errno)
{
	int save;

	if (__env_owned) return 0;

	if (!should_set_errno)
		save = errno;
	__env_owned = _owned_ptr_alloc();
	if (!__env_owned) {
		if (!should_set_errno)
			errno = save;
		return -1;
	}
	return 0;
}

/*
 * The copy flag may have 3 values:
 *  1 - make a copy of the name/value pair
 *  0 - take the name as a user-supplied name=value string
 * -1 - like 0, except we copy of the name=value string in name
 */
__private_extern__ int
__setenv(name, value, rewrite, copy, environp, owned)
	const char *name;
	const char *value;
	int rewrite, copy;
	char ***environp;
	struct owned_ptr *owned;
{
	char *c;
	int offset;
	int oindex;

	if ((c = __findenv(name, &offset, *environp))) { /* find if already exists */
		char *e;
		if (!rewrite)
			return (0);
		/*
		 * In UNIX03, we can overwrite only if we allocated the
		 * string.  Then we can realloc it if it is too small.
		 */
		e = (*environp)[offset];
		if (_owned_ptr_search(owned, e, &oindex) == 0) {
			if (copy > 0) {
				size_t l_value = strlen(value);
				if (strlen(c) < l_value) {	/* old smaller; resize*/
					char *r;
					size_t len = c - e;
					if ((r = realloc(e, l_value + len + 1)) == NULL)
						return (-1);
					if (r != e) {
						(*environp)[offset] = r;
						c = r + len;
						_owned_ptr_delete(owned, oindex);
						_owned_ptr_add(owned, r);
					}
				}
				while ( (*c++ = *value++) );
				return (0);
			}
			/* Free up the slot for later use (putenv) */
			_owned_ptr_delete(owned, oindex);
			free(e);
		}
	} else {					/* create new slot */
		int cnt;
		char **p;

		for (p = *environp, cnt = 0; *p; ++p, ++cnt);
		if (_owned_ptr_search(owned, *environp, &oindex) == 0) {	/* just increase size */
			p = (char **)realloc((char *)*environp,
			    (size_t)(sizeof(char *) * (cnt + 2)));
			if (!p)
				return (-1);
			if (*environp != p) {
				_owned_ptr_delete(owned, oindex);
				_owned_ptr_add(owned, p);
				*environp = p;
			}
		}
		else {				/* get new space */
						/* copy old entries into it */
			p = malloc((size_t)(sizeof(char *) * (cnt + 2)));
			if (!p)
				return (-1);
			_owned_ptr_add(owned, p);
			bcopy(*environp, p, cnt * sizeof(char *));
			*environp = p;
		}
		(*environp)[cnt + 1] = NULL;
		offset = cnt;
	}
	/* For non Unix03, or UnixO3 setenv(), we make a copy of the user's
	 * strings.  For Unix03 putenv(), we put the string directly in
	 * the environment. */
	if (copy > 0) {
		for (c = (char *)name; *c && *c != '='; ++c);	/* no `=' in name */
		if (!((*environp)[offset] =			/* name + `=' + value */
		    malloc((size_t)((int)(c - name) + strlen(value) + 2))))
			return (-1);
		_owned_ptr_add(owned, (*environp)[offset]);
		for (c = (*environp)[offset]; (*c = *name++) && *c != '='; ++c);
		for (*c++ = '='; (*c++ = *value++); );
	} else {
		/* the legacy behavior copies the string */
		if (copy < 0) {
			size_t len = strlen(name);
			if((c = malloc(len + 1)) == NULL)
				return (-1);
			_owned_ptr_add(owned, c);
			memcpy(c, name, len + 1);
			name = c;
		}
		(*environp)[offset] = (char *)name;
	}
	return (0);
}

__private_extern__ void
__unsetenv(const char *name, char **environ, struct owned_ptr *owned)
{
	char **p;
	int offset;
	int oindex;

	while (__findenv(name, &offset, environ)) { /* if set multiple times */
		/* if we malloc-ed it, free it first */
		if (_owned_ptr_search(owned, environ[offset], &oindex) == 0) {
			_owned_ptr_delete(owned, oindex);
			free(environ[offset]);
		}
		for (p = &environ[offset];; ++p)
			if (!(*p = *(p + 1)))
				break;
	}
}

/****************************************************************************/
/*
 * _allocenvstate -- SPI that creates a new state (opaque)
 */
void *
_allocenvstate(void)
{
	return _owned_ptr_alloc();
}

/*
 * _copyenv -- SPI that copies a NULL-tereminated char * array in a newly
 * allocated buffer, compatible with the other SPI env routines.  If env
 * is NULL, a char * array composed of a single NULL is returned.  NULL
 * is returned on error.  (This isn't needed anymore, as __setenv will
 * automatically make a copy.)
 */
char **
_copyenv(char **env)
{
	char **p;
	int cnt = 1;

	if (env)
		for (p = env; *p; ++p, ++cnt);
	p = (char **)malloc((size_t)(sizeof(char *) * cnt));
	if (!p)
		return (NULL);
	if (env)
		bcopy(env, p, cnt * sizeof(char *));
	else
		*p = NULL;
	return p;
}

/*
 * _deallocenvstate -- SPI that frees all the memory associated with the state
 * and all allocated strings, including the environment array itself if it
 * was copied.
 */
int
_deallocenvstate(void *state)
{
	struct owned_ptr *owned;

	if (!(owned = (struct owned_ptr *)state) || owned == __env_owned) {
		errno = EINVAL;
		return -1;
	}
	_owned_ptr_free(owned);
	return 0;
}

/*
 * setenvp -- SPI using an arbitrary pointer to string array and an env state,
 * created by _allocenvstate().  Initial checking is not done.
 *
 *	Set the value of the environmental variable "name" to be
 *	"value".  If rewrite is set, replace any current value.
 */
int
_setenvp(const char *name, const char *value, int rewrite, char ***envp, void *state)
{
	if (__init__env_owned(1)) return (-1);
	return (__setenv(name, value, rewrite, 1, envp, (state ? (struct owned_ptr *)state : __env_owned)));
}

/*
 * unsetenv(name) -- SPI using an arbitrary pointer to string array and an env
 * state, created by _allocenvstate().  Initial checking is not done.
 *
 *	Delete environmental variable "name".
 */
int
_unsetenvp(const char *name, char ***envp, void *state)
{
	if (__init__env_owned(1)) return (-1);
	__unsetenv(name, *envp, (state ? (struct owned_ptr *)state : __env_owned));
	return 0;
}

#endif /* !BUILD_VARIANT */

/*
 * setenv --
 *	Set the value of the environmental variable "name" to be
 *	"value".  If rewrite is set, replace any current value.
 */
int
setenv(name, value, rewrite)
	const char *name;
	const char *value;
	int rewrite;
{
#ifdef LEGACY_CRT1_ENVIRON
	int ret;
#endif /* LEGACY_CRT1_ENVIRON */

	/* no null ptr or empty str */
	if(name == NULL || *name == 0) {
		errno = EINVAL;
		return (-1);
	}

#if __DARWIN_UNIX03
	/* no '=' in name */
	if (strchr(name, '=')) {
		errno = EINVAL;
		return (-1);
	}
#endif /* __DARWIN_UNIX03 */

	if (*value == '=')			/* no `=' in value */
		++value;
	if (__init__env_owned(1)) return (-1);
#ifdef LEGACY_CRT1_ENVIRON
	ret = __setenv(name, value, rewrite, 1, _NSGetEnviron(), __env_owned);
	_saved_environ = *_NSGetEnviron();
	return ret;
#else /* !LEGACY_CRT1_ENVIRON */
	return (__setenv(name, value, rewrite, 1, _NSGetEnviron(), __env_owned));
#endif /* !LEGACY_CRT1_ENVIRON */
}

/*
 * unsetenv(name) --
 *	Delete environmental variable "name".
 */
#if __DARWIN_UNIX03
int
#else /* !__DARWIN_UNIX03 */
void
#endif /* __DARWIN_UNIX03 */
unsetenv(name)
	const char *name;
{
#if __DARWIN_UNIX03
	/* no null ptr or empty str */
	if(name == NULL || *name == 0) {
		errno = EINVAL;
		return (-1);
	}

	/* no '=' in name */
	if (strchr(name, '=')) {
		errno = EINVAL;
		return (-1);
	}
	if (__init__env_owned(1)) return (-1);
#else /* !__DARWIN_UNIX03 */
	/* no null ptr or empty str */
	if(name == NULL || *name == 0)
		return;
	if (__init__env_owned(0)) return;
#endif /* __DARWIN_UNIX03 */
	__unsetenv(name, *_NSGetEnviron(), __env_owned);
#if __DARWIN_UNIX03
	return 0;
#endif /* __DARWIN_UNIX03 */
}
