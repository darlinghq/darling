/*
 * Copyright (c) 2000-2007 Apple Inc. All rights reserved.
 */
/*
 * Copyright (c) 1997, Apple Computer, Inc. All rights reserved.
 *
 */

#ifndef _BSD_ARM_PROFILE_H_
#define _BSD_ARM_PROFILE_H_

#include <sys/appleapiopts.h>

#ifdef KERNEL
#ifdef __APPLE_API_UNSTABLE

/*
 * Block interrupts during mcount so that those interrupts can also be
 * counted (as soon as we get done with the current counting).  On the
 * arm platfom, can't do splhigh/splx as those are C routines and can
 * recursively invoke mcount.
 */
#warning MCOUNT_* not implemented yet.

#define MCOUNT_INIT
#define MCOUNT_ENTER    /* s = splhigh(); */ /* XXX TODO */
#define MCOUNT_EXIT     /* (void) splx(s); */ /* XXX TODO */

#endif /* __APPLE_API_UNSTABLE */
#endif /* KERNEL */

#endif /* _BSD_ARM_PROFILE_H_ */
