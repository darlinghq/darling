/*
 * Copyright (c) 2000-2003, 2007 Apple Inc. All rights reserved.
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
/*
 * Copyright 1996 1995 by Open Software Foundation, Inc. 1997 1996 1995 1994 1993 1992 1991  
 *              All Rights Reserved 
 *  
 * Permission to use, copy, modify, and distribute this software and 
 * its documentation for any purpose and without fee is hereby granted, 
 * provided that the above copyright notice appears in all copies and 
 * that both the copyright notice and this permission notice appear in 
 * supporting documentation. 
 *  
 * OSF DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE 
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS 
 * FOR A PARTICULAR PURPOSE. 
 *  
 * IN NO EVENT SHALL OSF BE LIABLE FOR ANY SPECIAL, INDIRECT, OR 
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM 
 * LOSS OF USE, DATA OR PROFITS, WHETHER IN ACTION OF CONTRACT, 
 * NEGLIGENCE, OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION 
 * WITH THE USE OR PERFORMANCE OF THIS SOFTWARE. 
 * 
 */
/*
 * MkLinux
 */

/*
 * POSIX Pthread Library
 *   Thread Specific Data support
 *   NB: pthread_getspecific() is in a separate assembly file
 */

#include "pthread_internals.h"

static struct
{
	int  created;    /* Set TRUE if 'create_key' used this slot */
	void (*destructor)(void *);
} _pthread_keys[_INTERNAL_POSIX_THREAD_KEYS_END];
static pthread_lock_t tds_lock = LOCK_INITIALIZER;

/*
 * Partition _pthread_keys in a lower part that dyld can use, and an upper
 * part for libSystem.  The libSystem part starts at __pthread_tsd_first = 4.
 * dyld will set this value to 1.
 * LEFT OVER TILL DYLD changes to using static (1-3) numbers
 */

#ifdef DYLD_STATIC_LIBC_BUILD
__private_extern__ int __pthread_tsd_first = 1;
__private_extern__ int __pthread_tsd_start = 5;
__private_extern__ int __pthread_tsd_end = 10;
__private_extern__ int __pthread_tsd_max = 10;
#else
__private_extern__ int __pthread_tsd_first = 10;
__private_extern__ int __pthread_tsd_start = _INTERNAL_POSIX_THREAD_KEYS_MAX;
__private_extern__ int __pthread_tsd_end = _INTERNAL_POSIX_THREAD_KEYS_END;
__private_extern__ int __pthread_tsd_max = 10;
#endif
/*
 * Create a new key for thread specific data
 */
int       
pthread_key_create(pthread_key_t *key,
		   void (*destructor)(void *))
{
	int res, i;
	LOCK(tds_lock);
	res = EAGAIN;  /* No 'free' keys, return EAGAIN (as per standard) */
	/* The first slot is reserved for pthread_self() */
	for (i = __pthread_tsd_start;  i < __pthread_tsd_end;  i++)
	{
		if (_pthread_keys[i].created == FALSE)
		{
			_pthread_keys[i].created = TRUE;
			_pthread_keys[i].destructor = destructor;
			*key = i;
			res = 0;
			break;
		}
	}
	UNLOCK(tds_lock);
        return (res);
}

/*
 * Destroy a thread specific data key
 */
int       
pthread_key_delete(pthread_key_t key)
{
	int res;
	LOCK(tds_lock);
	/* The first slot is reserved for pthread_self() */
	if ((key >= __pthread_tsd_start) && (key < __pthread_tsd_end))
	{
		if (_pthread_keys[key].created)
		{
			struct _pthread * p;

			_pthread_keys[key].created = FALSE;
			LOCK(_pthread_list_lock);
			TAILQ_FOREACH(p, &__pthread_head, plist) {
				/* It is  an 32bit value no lock needed */
				p->tsd[key] = 0;
			}
			UNLOCK(_pthread_list_lock);	
			res = 0;
		} else
		{
			res = EINVAL;
		}
	} else
	{ /* Invalid key */
		res = EINVAL;
	}
	UNLOCK(tds_lock);
	return (res);
}

/*
 * Set the thread private value for a given key.
 * We do not take the spinlock for this or pthread_getspecific.
 * The assignment to self->tsd[] is thread-safe because we never
 * refer to the tsd[] of a thread other than pthread_self().
 * The reference to _pthread_keys[...].created could race with a
 * pthread_key_delete() but in this case the behaviour is allowed
 * to be undefined.
 */
int       
pthread_setspecific(pthread_key_t key,
		    const void *value)
{
	int res;
	pthread_t self;
	/* The first slot is reserved for pthread_self() */
	if ((key >= __pthread_tsd_first) && (key < __pthread_tsd_end))
	{
		if ((key < __pthread_tsd_start) || _pthread_keys[key].created)
		{
			self = pthread_self();
			self->tsd[key] = (void *) value;
			res = 0;
			if ((key < __pthread_tsd_start) && (_pthread_keys[key].created == FALSE))
				_pthread_keys[key].created = TRUE;

			if (key > self->max_tsd_key)
			        self->max_tsd_key = key;
		} else
		{
			res = EINVAL;
		}
	} else
	{ /* Invalid key */
		res = EINVAL;
	}
        return (res);
}

/*
 * Clean up thread specific data as thread 'dies'
 */
void
_pthread_tsd_cleanup(pthread_t self)
{
	int i, j;
	void *param;

	/* destroy dynamic keys first */
	for (j = 0;  j < PTHREAD_DESTRUCTOR_ITERATIONS;  j++)
	{
		for (i = __pthread_tsd_start;  i <= self->max_tsd_key;  i++)
		{
			if (_pthread_keys[i].created && (param = self->tsd[i]))
			{
				self->tsd[i] = (void *)NULL;
				if (_pthread_keys[i].destructor)
				{
					(_pthread_keys[i].destructor)(param);
				}
			}
		}
	}

	self->max_tsd_key = 0;

	/* 
	 * The first slot is reserved for pthread_self() and there is no cleanup on it.
	 * Destroy rest of the static keys next only if any destructors registered.
	 */
	for (j = 0;  j < PTHREAD_DESTRUCTOR_ITERATIONS;  j++) {
		for (i = __pthread_tsd_first;  i <= __pthread_tsd_max;  i++)
		{
			if (_pthread_keys[i].created && (param = self->tsd[i]))
			{
				self->tsd[i] = (void *)NULL;
				if (_pthread_keys[i].destructor)
				{
					(_pthread_keys[i].destructor)(param);
				}
			}
		}
	}
}

int
pthread_key_init_np(int key, void (*destructor)(void *))
{
	if ((key >= __pthread_tsd_first) && (key < __pthread_tsd_start)) {
		LOCK(tds_lock);
		_pthread_keys[key].created = TRUE;
		_pthread_keys[key].destructor = destructor;

		if (key > __pthread_tsd_max)
		        __pthread_tsd_max = key;
		UNLOCK(tds_lock);
        	return (0);
	} else 
		return(EINVAL);
}

/* we need this till the switchover happens for the dyld. It get 1- 10 slot */
void
_pthread_keys_init()
{
	if (__pthread_tsd_first == 1)  {
		__pthread_tsd_start = 5;		
		__pthread_tsd_end = 10;
		__pthread_tsd_max = 10;
	}

}

