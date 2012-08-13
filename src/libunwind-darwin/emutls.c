/* TLS emulation.
   Copyright (C) 2006, 2008, 2009 Free Software Foundation, Inc.
   Contributed by Jakub Jelinek <jakub@redhat.com>.

This file is part of GCC.

GCC is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free
Software Foundation; either version 3, or (at your option) any later
version.

GCC is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or
FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
for more details.

Under Section 7 of GPL version 3, you are granted additional
permissions described in the GCC Runtime Library Exception, version
3.1, as published by the Free Software Foundation.

You should have received a copy of the GNU General Public License and
a copy of the GCC Runtime Library Exception along with this program;
see the files COPYING3 and COPYING.RUNTIME respectively.  If not, see
<http://www.gnu.org/licenses/>.  */

#include "tconfig.h"
#include "tsystem.h"
#include "coretypes.h"
#include "tm.h"
#include "gthr.h"

typedef unsigned int word __attribute__((mode(word)));
typedef unsigned int pointer __attribute__((mode(pointer)));

struct __emutls_object
{
  word size;
  word align;
  union {
    pointer offset;
    void *ptr;
  } loc;
  void *templ;
};

struct __emutls_array
{
  pointer size;
  void **data[];
};

void *__emutls_get_address (struct __emutls_object *);
void __emutls_register_common (struct __emutls_object *, word, word, void *);

#ifdef __GTHREADS
#ifdef __GTHREAD_MUTEX_INIT
static __gthread_mutex_t emutls_mutex = __GTHREAD_MUTEX_INIT;
#else
static __gthread_mutex_t emutls_mutex;
#endif
static __gthread_key_t emutls_key;
static pointer emutls_size;

static void
emutls_destroy (void *ptr)
{
  struct __emutls_array *arr = ptr;
  pointer size = arr->size;
  pointer i;

  for (i = 0; i < size; ++i)
    {
      if (arr->data[i])
	free (arr->data[i][-1]);
    }

  free (ptr);
}

static void
emutls_init (void)
{
#ifndef __GTHREAD_MUTEX_INIT
  __GTHREAD_MUTEX_INIT_FUNCTION (&emutls_mutex);
#endif
  if (__gthread_key_create (&emutls_key, emutls_destroy) != 0)
    abort ();
}
#endif

static void *
emutls_alloc (struct __emutls_object *obj)
{
  void *ptr;
  void *ret;

  /* We could use here posix_memalign if available and adjust
     emutls_destroy accordingly.  */
  if (obj->align <= sizeof (void *))
    {
      ptr = malloc (obj->size + sizeof (void *));
      if (ptr == NULL)
	abort ();
      ((void **) ptr)[0] = ptr;
      ret = ptr + sizeof (void *);
    }
  else
    {
      ptr = malloc (obj->size + sizeof (void *) + obj->align - 1);
      if (ptr == NULL)
	abort ();
      ret = (void *) (((pointer) (ptr + sizeof (void *) + obj->align - 1))
		      & ~(pointer)(obj->align - 1));
      ((void **) ret)[-1] = ptr;
    }

  if (obj->templ)
    memcpy (ret, obj->templ, obj->size);
  else
    memset (ret, 0, obj->size);

  return ret;
}

void *
__emutls_get_address (struct __emutls_object *obj)
{
  if (! __gthread_active_p ())
    {
      if (__builtin_expect (obj->loc.ptr == NULL, 0))
	obj->loc.ptr = emutls_alloc (obj);
      return obj->loc.ptr;
    }

#ifndef __GTHREADS
  abort ();
#else
  pointer offset = obj->loc.offset;

  if (__builtin_expect (offset == 0, 0))
    {
      static __gthread_once_t once = __GTHREAD_ONCE_INIT;
      __gthread_once (&once, emutls_init);
      __gthread_mutex_lock (&emutls_mutex);
      offset = obj->loc.offset;
      if (offset == 0)
	{
	  offset = ++emutls_size;
	  obj->loc.offset = offset;
	}
      __gthread_mutex_unlock (&emutls_mutex);
    }

  struct __emutls_array *arr = __gthread_getspecific (emutls_key);
  if (__builtin_expect (arr == NULL, 0))
    {
      pointer size = offset + 32;
      arr = calloc (size + 1, sizeof (void *));
      if (arr == NULL)
	abort ();
      arr->size = size;
      __gthread_setspecific (emutls_key, (void *) arr);
    }
  else if (__builtin_expect (offset > arr->size, 0))
    {
      pointer orig_size = arr->size;
      pointer size = orig_size * 2;
      if (offset > size)
	size = offset + 32;
      arr = realloc (arr, (size + 1) * sizeof (void *));
      if (arr == NULL)
	abort ();
      arr->size = size;
      memset (arr->data + orig_size, 0,
	      (size - orig_size) * sizeof (void *));
      __gthread_setspecific (emutls_key, (void *) arr);
    }

  void *ret = arr->data[offset - 1];
  if (__builtin_expect (ret == NULL, 0))
    {
      ret = emutls_alloc (obj);
      arr->data[offset - 1] = ret;
    }
  return ret;
#endif
}

void
__emutls_register_common (struct __emutls_object *obj,
			  word size, word align, void *templ)
{
  if (obj->size < size)
    {
      obj->size = size;
      obj->templ = NULL;
    }
  if (obj->align < align)
    obj->align = align;
  if (templ && size == obj->size)
    obj->templ = templ;
}
