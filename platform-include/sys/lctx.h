/*
 * $Id: lctx.h,v 1.1.6.2 2006/03/03 23:20:46 msteil Exp $
 */

#ifndef _SYS_LCTX_H_
#define _SYS_LCTX_H_

#include <sys/syscall.h>

static __inline pid_t
getlcid(pid_t pid)
{
        return (syscall(SYS_getlcid, pid));
}

static __inline int
setlcid(pid_t pid, pid_t lcid)
{
        return (syscall(SYS_setlcid, pid, lcid));
}

#define LCID_PROC_SELF  (0)
#define LCID_REMOVE     (-1)
#define LCID_CREATE     (0)

#endif	/* !_SYS_LCTX_H_ */
