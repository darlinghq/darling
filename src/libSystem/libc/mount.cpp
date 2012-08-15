#include "mount.h"
#include <mntent.h>
#include "libc/darwin_errno_codes.h"
#include <errno.h>

template<typename Statfs, typename Fsstatfun> int __darwin_getmntinfoGen(Fsstatfun f, Statfs** mntbufp, int flags)
{
	static __thread Statfs* lastMntbufp = 0;
	
	*mntbufp = 0;
	
	if (lastMntbufp)
	{
		delete [] lastMntbufp;
		lastMntbufp = 0;
	}
	
	int count = f(0, 0, 0);
	
	if (count < 0)
		return -1;
	
	lastMntbufp = new Statfs[count];
	if (!lastMntbufp)
	{
		errno = DARWIN_ENOMEM;
		return -1;
	}
	
	count = f(lastMntbufp, count*sizeof(Statfs), 0);
	
	if (count < 0)
	{
		delete [] lastMntbufp;
		lastMntbufp = 0;
		return -1;
	}
	
	*mntbufp = lastMntbufp;
	return count;
}

int __darwin_getmntinfo(struct __darwin_statfs** mntbufp, int flags)
{
	return __darwin_getmntinfoGen(__darwin_getfsstat, mntbufp, flags);
}

int __darwin_getmntinfo64(struct __darwin_statfs64** mntbufp, int flags)
{
	return __darwin_getmntinfoGen(__darwin_getfsstat64, mntbufp, flags);
}

