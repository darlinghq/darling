/*
 * Copyright (c) 2000-2007 Apple Inc. All rights reserved.
 */
#ifndef _MACHINE_DISKLABEL_H_
#define _MACHINE_DISKLABEL_H_

#include <sys/appleapiopts.h>

#ifdef __APPLE_API_OBSOLETE
#define LABELSECTOR     (1024 / DEV_BSIZE)      /* sector containing label */
#define LABELOFFSET     0                       /* offset of label in sector */
#define MAXPARTITIONS   8                       /* number of partitions */
#define RAW_PART        2                       /* raw partition: xx?c */

/* Just a dummy */
struct cpu_disklabel {
	int     cd_dummy;                       /* must have one element. */
};
#endif /* __APPLE_API_OBSOLETE */

#endif /* _MACHINE_DISKLABEL_H_ */
