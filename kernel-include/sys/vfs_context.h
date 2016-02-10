#ifndef _BSD_SYS_VFS_CONTEXT_H_
#define _BSD_SYS_VFS_CONTEXT_H_

#include <sys/cdefs.h>
#include <sys/types.h>
#include <sys/kernel_types.h>
#include <kern/thread.h>
#ifdef BSD_KERNEL_PRIVATE
#include <sys/user.h>
#endif
#include <stdint.h>

/*
 * XXX should go away; pulls in vfs_context structure definition from
 * XXX <sys/user.h>
 */

#endif /* !_BSD_SYS_VFS_CONTEXT_H_ */
