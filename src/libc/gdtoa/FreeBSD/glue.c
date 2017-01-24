/*
 * Machine-independent glue to integrate David Gay's gdtoa
 * package into libc.
 *
 * $FreeBSD: src/lib/libc/gdtoa/glue.c,v 1.1 2003/03/12 20:29:58 das Exp $
 */

#include "spinlock.h"

spinlock_t __gdtoa_locks[2];
