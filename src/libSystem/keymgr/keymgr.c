/* Copyright (C) 1999, 2000, 2001, 2002, 2003, 2004, 2005 Apple Computer, Inc.
   Copyright (C) 1997, 2001 Free Software Foundation, Inc.

This file is part of KeyMgr.

KeyMgr is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free
Software Foundation; either version 2, or (at your option) any later
version.

In addition to the permissions in the GNU General Public License,
Apple Computer and the Free Software Foundation gives you unlimited
permission to link the compiled version of this file into combinations
with other programs, and to distribute those combinations without any
restriction coming from the use of this file.  (The General Public
License restrictions do apply in other respects; for example, they
cover modification of the file, and distribution when not linked into
a combine executable.)

KeyMgr is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or
FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
for more details.

You should have received a copy of the GNU General Public License
along with KeyMgr; see the file COPYING.  If not, write to the Free
Software Foundation, 59 Temple Place - Suite 330, Boston, MA
02111-1307, USA.  */

/* The FSF-copyright part is the definition of 'struct
   old_object'.  It was taken from unwind-dw2-fde.h from GCC 3.1.  */

/*
 * keymgr - Create and maintain process-wide global data known to
 *	    all threads across all dynamic libraries.
 *
 */

#include <mach-o/dyld.h>
#include <dlfcn.h>
#include <pthread.h>
#include <errno.h>
#include <stdlib.h>
#include <libkern/OSAtomic.h>
#include <stdint.h>
#include "keymgr.h"

/* On x86 or later platforms, there's no need to support the TLS SPIs.
   The only reason to support them on ppc64 is because the tools
   shipped with Tiger included a libsupc++.a which used these
   functions.  (To do: investigate whether anyone ever actually used
   that library for ppc64.)  */
#if defined(__ppc__) || defined(__ppc64__)
#define WANT_TLS_SPIS 1
#endif

#ifndef ESUCCESS
#define ESUCCESS 0
#endif

/* Types of node, used for error checking.  */
typedef enum node_kinds {
  NODE_PROCESSWIDE_PTR=1,
#ifdef WANT_TLS_SPIS
  NODE_THREAD_SPECIFIC_DATA
#endif
} TnodeKind;

/* These values are internal; the subflags of NM_ENHANCED_LOCKING are
   defined in keymgr.h and are not internal.  */
enum {
  NM_ENHANCED_LOCKING=3
};

/* Base node kind for keymgr node list; also used to represent per
   thread variables (NODE_THREAD_SPECIFIC_DATA).  */

typedef struct Skey_data {
  struct Skey_data * next;
  unsigned int handle;		/* Key id of variable.  */
  unsigned char node_kind;	/* What kind of variable is this?  */
  unsigned char flags;		/* Flags controlling behavior of variable.  */
  unsigned short refcount;	/* If recursion has been enabled, reference
				   count.  */
  void *ptr;			/* Data associated with variable.  */
  pthread_mutex_t thread_lock;	/* Semaphore for this specific variable.  */
} Tkey_Data;

typedef struct Sinfo_Node {
  unsigned int size; 		/* Size of this node.  */
  unsigned short major_version; /* API major version.  */
  unsigned short minor_version;	/* API minor version.  */
} Tinfo_Node;

/* Static variables for initial __keymgr_global values.  */

static const Tinfo_Node keymgr_info = {
  sizeof (Tinfo_Node),
  KEYMGR_API_REV_MAJOR,
  KEYMGR_API_REV_MINOR
};

/* __keymgr_global - contains pointers to keymgr data that needs to be
   global and known across all dlls/plugins.  */
struct {
  /* Formerly, a pthreads semaphore for accessing the keymgr global data.  
     Now unused.  */
  void *unused;

  /* Pointer to keymgr global list.  This pointer was previously
     contained in __eh_global_dataptr.  */
  Tkey_Data * volatile keymgr_globals;

  /* Pointer to keymgr information node.  This is part of the semi-public
     ABI of keymgr.  */
  const Tinfo_Node *keymgr_info;
} __attribute__((aligned (32))) __keymgr_global = {
  NULL,
  NULL,
  &keymgr_info
};

#if defined(__ppc__)
/* Initialize keymgr.  */

void _init_keymgr (void)
{
  /* This routine is now empty, but is part of keymgr's ABI on ppc and
     so can't be completely removed.  */
}
#endif

/* Find any Tkey_Data associated with KEY.  If none exists, or KIND
   does not match, return NULL.  */

static Tkey_Data *
get_key_element (unsigned int key, TnodeKind kind)
{
  Tkey_Data  *keyArray;

  for (keyArray = __keymgr_global.keymgr_globals;
       keyArray != NULL;
       keyArray = keyArray->next)
    if (keyArray->handle == key)
      {
	if (keyArray->node_kind == kind)
	  return keyArray;
	else
	  return NULL;
      }

  return NULL;
}

/* Find any Tkey_Data associated with KEY.  If none exists, create one
   and add it to the list.  Put it in *RESULT.

   On success, return 0, otherwise:
   [EINVAL]  The node existed but was not of type KIND.
   [ENOMEM]  Out of memory, couldn't create node.
   [EAGAIN]  The mutex associated with the new node couldn't be created.
	     This can only happen when KIND == NODE_PROCESSWIDE_PTR.

   The aims of this routine are:
	     
   - After execution, there should be exactly one element in the list
     with HANDLE set to KEY.
   - The pointer to that element should be returned in *RESULT.

   In addition, we know that:

   - The list only has elements added, never removed, and they are
     always added to the head of the list.
*/

static int
get_or_create_key_element (unsigned int key, TnodeKind kind, 
			   Tkey_Data **result)
{
  Tkey_Data *searchEnd = NULL;
  Tkey_Data *newEntry = NULL;
  
  for (;;)
    {
      Tkey_Data *keyArrayStart = __keymgr_global.keymgr_globals;
      Tkey_Data *keyArray;

      /* At this point, we know that we have not searched the elements
	 between keyArrayStart and searchEnd, but we have previously searched
	 searchEnd (if not NULL) and all the elements after it.  */

      for (keyArray = keyArrayStart;
	   keyArray != searchEnd;
	   keyArray = keyArray->next)
	if (keyArray->handle == key)
	  {
	    /* Found an existing entry for KEY.  Free any new entry we
	       created in a previous pass through the loop.  */
	    if (newEntry)
	      {
		if (kind == NODE_PROCESSWIDE_PTR)
		  /* This call should never fail.  */
		  if (pthread_mutex_destroy (&newEntry->thread_lock)
		      != ESUCCESS)
		    abort ();
		free (newEntry);
	      }

	    if (keyArray->node_kind != kind)
	      return EINVAL;	  
	    *result = keyArray;
	    return ESUCCESS;
	  }

      /* At this point, we know that there is no entry after keyArrayStart
	 which matches KEY.  We will try to add one.  */

      if (! newEntry)
	{
	  /* Create the new entry.  */
	  newEntry = (Tkey_Data *) malloc (sizeof (Tkey_Data));
	  if (newEntry == NULL)
	    return ENOMEM;
      
	  if (kind == NODE_PROCESSWIDE_PTR)
	    {
	      pthread_mutexattr_t attr;
	      int errnum;
	      
	      newEntry->refcount = 0;
	      newEntry->flags = 0;
	      
	      errnum = pthread_mutexattr_init (&attr);
	      if (errnum == ESUCCESS)
		{
		  pthread_mutexattr_settype (&attr, PTHREAD_MUTEX_ERRORCHECK);
		  errnum = pthread_mutex_init (&newEntry->thread_lock, &attr);
		  pthread_mutexattr_destroy (&attr);
		}
	      if (errnum != ESUCCESS)
		{
		  free (newEntry);
		  return errnum;
		}
	    }
	  
	  newEntry->handle = key;
	  newEntry->ptr = NULL;
	  newEntry->node_kind = kind;
	}

      /* The compare-and-swap will succeed only if the list head is
	 keyArrayStart, and we are prepending newEntry onto the list,
	 so the next element after newEntry should be keyArrayStart.  */

      newEntry->next = keyArrayStart;

      /* The barrier is necessary to ensure that newEntry->next is seen
	 to be set by any thread that sees newEntry in the list.  */

      /* If the compare-and-swap succeeds, it means that the head of
	 the list did not change between the search above and the
	 store of newEntry.  Therefore, since we searched all the
	 elements from the head at that time, we can be sure that the
	 entry we're adding is not in the list.  */
#ifdef __LP64__
      if (OSAtomicCompareAndSwap64Barrier (
			   (int64_t) keyArrayStart,
			   (int64_t) newEntry,
			   (int64_t *) &__keymgr_global.keymgr_globals))
#else
      if (OSAtomicCompareAndSwap32Barrier (
			   (int32_t) keyArrayStart,
			   (int32_t) newEntry,
			   (int32_t *) &__keymgr_global.keymgr_globals))
#endif
	{
	  *result = newEntry;
	  return ESUCCESS;
	}
      /* Another thread changed the list.  The entries between
	 keyArrayStart and searchEnd were searched above, so now we
	 can stop searching at keyArrayStart.  We know we will make
	 progress in the next loop iteration because the new searchEnd
	 will not be equal to __keymgr_global.keymgr_globals and so we
	 will search at least one new element.  Since the list can
	 have at most one element for each valid key, eventually there
	 will be no more elements to search and so we will stop reaching
	 this point and looping.  */
      searchEnd = keyArrayStart;
    }
}

#ifdef WANT_TLS_SPIS
/* Return the data associated with KEY for the current thread.
   Return NULL if there is no such data.  */

void *
_keymgr_get_per_thread_data (unsigned int key)
{
  Tkey_Data * keyArray;
  void * key_data;

  keyArray = get_key_element (key, NODE_THREAD_SPECIFIC_DATA);

  /* If the element hasn't been set, the answer is always NULL.  */
  if (keyArray == NULL)
    return NULL;

  /* On all platforms we run on, a pointer-sized load from an aligned
     address is atomic.  */
  key_data = keyArray->ptr;

  /* key_data might be NULL if this entry is in the process of being
     created.  In that case, return NULL.  */
  if (key_data == NULL)
    return NULL;

  return pthread_getspecific ((pthread_key_t) key_data);
}

/* Set the data for KEY for this thread to be KEYDATA.  Return 0 on success,
   or:

   [ENOMEM]  Out of memory.
   [EINVAL]  The KEY was previously used to store process-wide data.
   [EAGAIN]  The system lacked the necessary resources to create
             another thread-specific data key, or the system- imposed
             limit on the total number of keys per process
             [PTHREAD_KEYS_MAX] would be exceeded.  This error can't
             happen after the first successful call to
             _keymgr_set_per_thread_data from a particular thread with
             a particular KEY.
*/

int
_keymgr_set_per_thread_data (unsigned int key, void *keydata)
{
  volatile Tkey_Data * keyArray;
  pthread_key_t pthread_key;
  void *ptr;
  int errnum;

  errnum = get_or_create_key_element (key, NODE_THREAD_SPECIFIC_DATA,
				      (Tkey_Data **)&keyArray);
  if (errnum != ESUCCESS)
    return errnum;

  /* This routine doesn't lock *keyArray.  Instead, it relies on the
     fact that keyArray->ptr is either NULL or a valid pthread key value,
     and it only changes once, from NULL to a valid value.  */

  ptr = keyArray->ptr;
  if (ptr == NULL)
    {
      void (*destructor)(void *);
      bool neededInit;

      switch (key)
	{
	case KEYMGR_EH_CONTEXT_KEY:
	case KEYMGR_EH_GLOBALS_KEY:
	  destructor = free;
	  break;
	default:
	  destructor = NULL;
	  break;
	}
      if ((errnum = pthread_key_create (&pthread_key, destructor)) != 0)
	return errnum;

#ifdef __LP64__
      neededInit = OSAtomicCompareAndSwap64 ((int64_t) NULL,
					     (int64_t) pthread_key,
					     (int64_t *) &keyArray->ptr);
#else
      neededInit = OSAtomicCompareAndSwap32 ((int32_t) NULL,
					     (int32_t) pthread_key,
					     (int32_t *) &keyArray->ptr);
#endif
      if (!neededInit)
	pthread_key_delete (pthread_key);
      ptr = keyArray->ptr;
    }
  pthread_key = (pthread_key_t) ptr;

  return pthread_setspecific (pthread_key, keydata);
}
#endif /* WANT_TLS_SPIS */


/*
 * These routines provide management of a process wide, thread safe,
 * persistent pointer. If a pointer is created by a bundle/plug-in
 * and placed in here, it will persist for the life of the process,
 * even after the bundle has been unloaded. This is especially useful
 * for data shared across plugins and across repeated plugin loads and
 * unloads.
 */

/* Get the processwide data associated with KEY into *RESULT,
   and lock a lock associated with KEY.  

   On success, return 0, otherwise:

   [EINVAL]  KEY was previously used to store thread-specific data.
   [ENOMEM]  Out of memory, couldn't create node.
   [EAGAIN]  The mutex associated with the new node couldn't be created.
   [EDEADLK] A deadlock would occur if the thread blocked waiting
   	     for the lock.
*/

int
_keymgr_get_and_lock_processwide_ptr_2 (unsigned int key, void ** result)
{
  Tkey_Data *keyArray;
  int errnum;

  errnum = get_or_create_key_element (key, NODE_PROCESSWIDE_PTR, &keyArray);
  if (errnum != ESUCCESS)
    return errnum;

  if ((errnum = pthread_mutex_lock (&keyArray->thread_lock)) != ESUCCESS)
    return errnum;

  keyArray->refcount++;
  *result = keyArray->ptr;
  return ESUCCESS;
}

/* For backwards compatibility; like _keymgr_get_and_lock_processwide_ptr_2
   but returns the pointer or NULL on error.  */

void *
_keymgr_get_and_lock_processwide_ptr (unsigned int key)
{
  void *result = NULL;
  _keymgr_get_and_lock_processwide_ptr_2 (key, &result);
  return result;
}

/* Unlock the lock associated with NODE.  Return 0 on success.
   Returns EPERM if the lock is not locked by the current thread.  */

static int
unlock_node (Tkey_Data *node)
{
  int result;

  node->refcount--;
  result = pthread_mutex_unlock (&node->thread_lock);
  if (result != ESUCCESS)
    node->refcount++;
  return result;
}

/* Set the processwide data associated with KEY to PTR, and unlock
   the lock associated with KEY.

   On success, returns 0.  On failure, returns:

   [EINVAL]  KEY was previously used to store thread-specific data.
   [ENOMEM]  Out of memory, couldn't create node.
   [EAGAIN]  The mutex associated with the new node couldn't be created.
   [EPERM]   The lock was not locked by the current thread.
*/

int
_keymgr_set_and_unlock_processwide_ptr (unsigned int key, void *ptr)
{
  Tkey_Data *keyArray;
  int result;

  result = get_or_create_key_element (key, NODE_PROCESSWIDE_PTR, &keyArray);
  if (result != ESUCCESS)
    return result;

  keyArray->ptr = ptr;
  return unlock_node (keyArray);
}

/* Unlock the lock associated with KEY.

   On success, returns 0.  On failure, returns:

   [EINVAL]  KEY was not previously used to store process-specific data.
   [EPERM]   The lock was not locked by the current thread.
*/

int
_keymgr_unlock_processwide_ptr (unsigned int key)
{
  Tkey_Data *keyArray;

  keyArray = get_key_element (key, NODE_PROCESSWIDE_PTR);
  if (keyArray == NULL)
    return EINVAL;

  return unlock_node (keyArray);
}

/* Set the locking mode of KEY to MODE.  This should not be done while
   KEY is locked or another thread might be using KEY.

   On success, returns 0.  On failure, returns:
   [EINVAL]  KEY was previously used to store thread-specific data.
   [ENOMEM]  Out of memory.
   [EBUSY]   The mutex associated with the node was locked.
*/

int
_keymgr_set_lockmode_processwide_ptr (unsigned int key, unsigned int mode)
{
  Tkey_Data *keyArray;
  pthread_mutexattr_t attr;
  int type;
  int result;

  result = get_or_create_key_element (key, NODE_PROCESSWIDE_PTR, &keyArray);
  if (result != ESUCCESS)
    return result;

  if (mode == keyArray->flags)
    return ESUCCESS;
  
  result = pthread_mutexattr_init (&attr);
  if (result != ESUCCESS)
    return result;
  
  if (mode == NM_ALLOW_RECURSION)
    type = PTHREAD_MUTEX_RECURSIVE;
  else
    type = PTHREAD_MUTEX_ERRORCHECK;
  pthread_mutexattr_settype (&attr, type);
  
  /* Delete the old mutex and create a new one.  */
  result = pthread_mutex_destroy (&keyArray->thread_lock);
  /* Apple's implementation of pthread_mutex_init can't fail in this
     situation.  */
  if (result == ESUCCESS)
    result = pthread_mutex_init (&keyArray->thread_lock, &attr);

  pthread_mutexattr_destroy (&attr);

  if (result == ESUCCESS)
    keyArray->flags = mode;
  return result;
}
	
/* Returns the locking mode of the lock associated with KEY, if any,
   or 0 if there is no such lock.  */

unsigned int
_keymgr_get_lockmode_processwide_ptr (unsigned int key)
{
  Tkey_Data *keyArray;

  keyArray = get_key_element (key, NODE_PROCESSWIDE_PTR);
  if (keyArray == NULL)
    return 0;
  return keyArray->flags;
}

/* Return the number of locks on KEY.  To call this routine safely,
   you should be holding one of them, thus 0 is an error return.  */

int
_keymgr_get_lock_count_processwide_ptr (unsigned int key)
{
  Tkey_Data *keyArray;

  keyArray = get_key_element (key, NODE_PROCESSWIDE_PTR);
  if (keyArray == NULL)
    return 0;
  return keyArray->refcount;
}

/*********************************************/

#include <mach-o/getsect.h>

/* Beware, this is an API.  */

struct __live_images {
  unsigned long this_size;			/* sizeof (__live_images)  */
  const struct mach_header *mh;			/* the image info  */
  intptr_t vm_slide;
  void (*destructor)(struct __live_images *);	/* destructor for this  */
  struct __live_images *next;
  unsigned long examined_p;
  void *fde;
  void *object_info;
  unsigned long info[2];			/* GCC3 use  */
};

/* Bits in the examined_p field of struct live_images.  */
enum {
  EXAMINED_IMAGE_MASK = 1,	/* We've seen this one.  */
  ALLOCED_IMAGE_MASK = 2,	/* The FDE entries were allocated by
				   malloc, and must be freed.  This isn't
				   used by newer libgcc versions.  */
  IMAGE_IS_TEXT_MASK = 4,	/* This image is in the TEXT segment.  */
  DESTRUCTOR_MAY_BE_CALLED_LIVE = 8  /* The destructor may be called on an
					object that's part of the live
					image list.  */
};

#ifdef __ppc__
struct old_object
{
  void *pc_begin;
  void *pc_end;
  struct dwarf_fde *fde_begin;
  struct dwarf_fde **fde_array;
  size_t count;
  struct old_object *next;
  long section_size;
};

static const char __DWARF2_UNWIND_SECTION_TYPE[] = "__TEXT";
static const char __DWARF2_UNWIND_SECTION_NAME[] = "__dwarf2_unwind";
#endif

#if !__x86_64__
/* Called by dyld when an image is added to the executable.
   If it has a dwarf2_unwind section, register it so the C++ runtime
   can get at it.  All of this is protected by dyld thread locks.  */

static void dwarf2_unwind_dyld_add_image_hook (const struct mach_header *mh,
                                               intptr_t vm_slide)
{
#ifdef __ppc__
  uint32_t sz;
  char *fde;

  /* See if the image has a __TEXT __dwarf2_unwind section.  This
     is for backwards compatibility with old GCCs.  */

  fde = getsectdatafromheader (mh, __DWARF2_UNWIND_SECTION_TYPE,
				   __DWARF2_UNWIND_SECTION_NAME, &sz);

  if (fde != 0)
    {
      struct old_object *obp;

      obp = (struct old_object *) calloc (1, sizeof (struct old_object) + 8);
      if (obp == NULL)
	return;

      obp->section_size = sz;
      obp->fde_begin = (struct dwarf_fde *) (fde + vm_slide);

      if (_keymgr_get_and_lock_processwide_ptr_2 (KEYMGR_ZOE_IMAGE_LIST,
						  (void **) &obp->next) == 0)
	_keymgr_set_and_unlock_processwide_ptr (KEYMGR_ZOE_IMAGE_LIST, obp);
    }
#endif

  {
    struct __live_images *l = (struct __live_images *) calloc (1, sizeof (*l));
    if (l == NULL)
      return;

    l->mh = mh;
    l->vm_slide = vm_slide;
    l->this_size = sizeof (*l);
    if (_keymgr_get_and_lock_processwide_ptr_2 (KEYMGR_GCC3_LIVE_IMAGE_LIST,
						(void **) &l->next) == 0)
      _keymgr_set_and_unlock_processwide_ptr (KEYMGR_GCC3_LIVE_IMAGE_LIST, l);
  }
}



static void
dwarf2_unwind_dyld_remove_image_hook (const struct mach_header *mh,
				      intptr_t vm_slide)
{
#ifdef __ppc__
  uint32_t sz;
  char *fde;

  /* See if the image has a __TEXT __dwarf2_unwind section.  */

  fde = getsectdatafromheader (mh, __DWARF2_UNWIND_SECTION_TYPE,
				   __DWARF2_UNWIND_SECTION_NAME, &sz);
  if (fde != 0)
    {
      struct old_object *objlist, **obp;
 
      if (_keymgr_get_and_lock_processwide_ptr_2 (KEYMGR_ZOE_IMAGE_LIST,
						  (void **) &objlist) != 0)
	goto get_zoe_failed;

      for (obp = &objlist; *obp; obp = &(*obp)->next)
        if ((char *)(*obp)->fde_begin == fde + vm_slide)
          {
            struct old_object *p = *obp;
            *obp = p->next;
            if (p->pc_begin)
              free (p->fde_array);
            free (p);
            break;
          }

      _keymgr_set_and_unlock_processwide_ptr (KEYMGR_ZOE_IMAGE_LIST, objlist);
    get_zoe_failed:
      ;
    }
#endif

  {
    struct __live_images *top, **lip, *destroy = NULL;

    /* This is a bit of cache.  _dyld_get_image_header_containing_address
       can be expensive, but most of the time the destructors come from
       one or two objects and are consecutive in the list.  */
    void (*prev_destructor)(struct __live_images *) = NULL;
    int was_in_object = 0;

    /* Look for it in the list of live images and delete it.
       Also, call any destructors in case they are in this image and about
       to be unloaded.  The test with DESTRUCTOR_MAY_BE_CALLED_LIVE is
       because in GCC 3.1, the destructor would (uselessly) try to acquire
       the LIVE_IMAGE_LIST lock, and it's not practical to call it in
       that case (deadlock ensues, unless we release the lock, in which
       case the concurrency issues become impossible).  */

    if (_keymgr_get_and_lock_processwide_ptr_2 (KEYMGR_GCC3_LIVE_IMAGE_LIST,
						(void **) &top) != 0)
      goto get_live_image_failed;
      
    lip = &top;

    while (*lip != NULL)
      {
	if ((*lip)->destructor
	    && ((*lip)->examined_p & DESTRUCTOR_MAY_BE_CALLED_LIVE))
	  {
	    if (! was_in_object && (*lip)->destructor != prev_destructor)
	      {
		Dl_info info;
		prev_destructor = (*lip)->destructor;
		was_in_object = (dladdr (prev_destructor, &info) != 0
				 && info.dli_fbase == mh);
	      }
	    if ((*lip)->destructor == prev_destructor && was_in_object)
	      (*lip)->destructor (*lip);
	  }

	if ((*lip)->mh == mh && (*lip)->vm_slide == vm_slide)
	  {
	    destroy = *lip;
	    *lip = destroy->next;                 /* unlink DESTROY  */
	
	    if (destroy->this_size != sizeof (*destroy))  /* sanity check  */
	      abort ();
	
	    continue;
	  }
	lip = &(*lip)->next;
      }
    _keymgr_set_and_unlock_processwide_ptr (KEYMGR_GCC3_LIVE_IMAGE_LIST, top);

    /* Now that we have unlinked this from the image list, toss it.
       The destructor gets called here only to handle the GCC 3.1 case.  */
    if (destroy != NULL)
      {
	if (destroy->destructor != NULL)
	  (*destroy->destructor) (destroy);
	free (destroy);
      }
  get_live_image_failed:
    ;
  }
}
#endif // !x86_64

void __keymgr_dwarf2_register_sections (void)
{
  /* This function needs to remain for binary compatibiliity */
  /* with old programs which were linked with a crt1.o that */
  /* explicitly called __keymgr_dwarf2_register_sections() */
}


/* Used to statically allocate the head of the object list. */
/* The actual sruct is 4 pointers long and must be initially */
/* zero filled. */
static void* km_object_list_head[4];

/* call by libSystem's initializer */
void __keymgr_initializer (void)
{
  /* On Mac OS X 10.6, unwinding is down by libunwind in libSystem.B.dylib */
  /* Normally, the object list is need used.  But some applications that */
  /* dynamically generate code directly use keymgr to find the KEYMGR_GCC3_DW2_OBJ_LIST */
  /* list and insert frame information.  The problem is those apps don't check */
  /* if KEYMGR_GCC3_DW2_OBJ_LIST is currently NULL.  The fix is to statically */
  /* allocate the object list head struct and set KEYMGR_GCC3_DW2_OBJ_LIST to point*/
  /* to it <rdar://problem/6599778>. */
  _keymgr_get_and_lock_processwide_ptr(KEYMGR_GCC3_DW2_OBJ_LIST);
  _keymgr_set_and_unlock_processwide_ptr(KEYMGR_GCC3_DW2_OBJ_LIST, &km_object_list_head);

#if __x86_64__
  /* On Mac OS X 10.6, libunwind in libSystem.dylib implements all unwinding functionality. */
  /* libunwind does not use keymgr, so there is no need to maintain KEYMGR_GCC3_LIVE_IMAGE_LIST */
  /* in sync with dyld's list of images.  But there might be some i386 or ppc applications that */
  /* carry around there own copy of the unwinder (from libgcc.a) and need KEYMGR_GCC3_LIVE_IMAGE_LIST. */
#else
  /* register with dyld so that we are notified about all loaded mach-o images */
  _dyld_register_func_for_add_image (dwarf2_unwind_dyld_add_image_hook);
  _dyld_register_func_for_remove_image (dwarf2_unwind_dyld_remove_image_hook);
#endif /* __x86_64__ */
}

