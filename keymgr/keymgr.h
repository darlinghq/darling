/* Copyright (C) 1999, 2000, 2001, 2002, 2003 Apple Computer, Inc.

This file is part of KeyMgr.

KeyMgr is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free
Software Foundation; either version 2, or (at your option) any later
version.

In addition to the permissions in the GNU General Public License,
Apple Computer gives you unlimited permission to link the compiled
version of this file into combinations with other programs, and to
distribute those combinations without any restriction coming from the
use of this file.  (The General Public License restrictions do apply
in other respects; for example, they cover modification of the file,
and distribution when not linked into a combine executable.)

KeyMgr is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or
FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
for more details.

You should have received a copy of the GNU General Public License
along with KeyMgr; see the file COPYING.  If not, write to the Free
Software Foundation, 59 Temple Place - Suite 330, Boston, MA
02111-1307, USA.  */

#ifndef __KEYMGR_H
#define __KEYMGR_H

#ifdef __cplusplus
extern "C" {
#endif



/*
 * keymgr - Create and maintain process-wide global data known to 
 *	    all threads across all dynamic libraries. 
 *
 */
 
enum {
  NM_ALLOW_RECURSION = 1,
  NM_RECURSION_ILLEGAL = 2
};
	
extern void * _keymgr_get_per_thread_data (unsigned int key);
extern int _keymgr_set_per_thread_data (unsigned int key, void *keydata);
extern void *_keymgr_get_and_lock_processwide_ptr (unsigned int key);
extern int _keymgr_get_and_lock_processwide_ptr_2 (unsigned int key, void **);
extern int _keymgr_set_and_unlock_processwide_ptr (unsigned int key, 
						   void *ptr);
extern int _keymgr_unlock_processwide_ptr (unsigned int key);
extern int _keymgr_set_lockmode_processwide_ptr (unsigned int key, 
						 unsigned int mode);
extern unsigned int _keymgr_get_lockmode_processwide_ptr (unsigned int key);
extern int _keymgr_get_lock_count_processwide_ptr (unsigned int key);

/*
 * Keys currently in use:
 */

/* Head pointer of exception context node.  */
#define KEYMGR_EH_CONTEXT_KEY		1

/* New handler.  */
#define KEYMGR_NEW_HANDLER_KEY		2

/* Unexpected exception handler.  */
#define KEYMGR_UNEXPECTED_HANDLER_KEY	3

/* Terminate handler.  */
#define KEYMGR_TERMINATE_HANDLER_KEY	4

/* Runtime mode bits.  */
#define KEYMGR_MODE_BITS		5

/* Head pointer of the list of open streams.  */
#define KEYMGR_IO_LIST			6

/* libstdc++ for GCC 2.95 stdin.  */
#define KEYMGR_IO_STDIN			7

/* libstdc++ for GCC 2.95 stdout.  */
#define KEYMGR_IO_STDOUT		8

/* libstdc++ for GCC 2.95 stdout.  */
#define KEYMGR_IO_STDERR		9

/* Number of plugins/main program currently using streams in GCC 2.95.  */
#define KEYMGR_IO_REFCNT		10

/* Flags controlling the behavior of C++ I/O.  */
#define KEYMGR_IO_MODE_BITS		11

/* Head pointer for list of per image dwarf2 unwind sections.  */
#define KEYMGR_ZOE_IMAGE_LIST		12

/* C++ runtime EH global data.  */
#define KEYMGR_EH_GLOBALS_KEY		13

/* atexit() and __cxa_atexit routine list.  */
#define KEYMGR_ATEXIT_LIST		14

/* KeyMgr 3.x is the first one supporting GCC3 stuff natively.  */
#define KEYMGR_API_MAJOR_GCC3		3
/* ... with these keys.  */
#define KEYMGR_GCC3_LIVE_IMAGE_LIST	301	/* loaded images  */
#define KEYMGR_GCC3_DW2_OBJ_LIST	302	/* Dwarf2 object list  */

/*
 * Other important data.
 */
 
/* Major revision number of the keymgr API.  */
#define KEYMGR_API_REV_MAJOR		5
/* Minor revision number of the keymgr API.  */
#define KEYMGR_API_REV_MINOR		0

#ifdef __cplusplus
}
#endif

#endif /* __KEYMGR_H */
