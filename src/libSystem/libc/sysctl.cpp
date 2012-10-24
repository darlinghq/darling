#include "config.h"
#include "sysctl.h"
#include "log.h"
#include "trace.h"
#include "darwin_errno_codes.h"
#include <errno.h>
#include <unistd.h>

#define CTL_KERN 1
#define CTL_HW 6

#define HW_NCPU 3
#define HW_PHYSMEM 5
#define HW_USERMEM 6
#define HW_AVAILCPU 25

int __darwin_sysctl(int* name, unsigned int namelen,
                    void* oldp, size_t* oldlenp,
                    void* newp, size_t newlen) {
	TRACE6(name, namelen, oldp, oldlenp, newp, newlen);
	
	LOG << "sysctl: namelen=" << namelen;
	for (int i = 0; i < namelen; i++)
		LOG << " name[" << i << "]=" << name[i];
	LOG << " newp=" << newp << std::endl;

	if (newp)
	{
		LOG << "sysctl with newp isn't supported yet.\n";
		errno = DARWIN_EINVAL;
		return -1;
	}

	if (namelen != 2)
	{
		LOG << "sysctl with namelen=" << namelen << " isn't supported yet.\n";
		errno = DARWIN_EINVAL;
		return -1;
	}

	switch (name[0])
	{
	case CTL_HW:
	{
		if (*oldlenp != 4 && *oldlenp != 8)
		{
			LOG << "sysctl(HW) with oldlenp=" << *oldlenp << " isn't supported yet.\n";
			errno = DARWIN_EINVAL;
			return -1;
		}

		uint64_t val = 0;
		switch (name[1])
		{
		case HW_NCPU:
			val = ::sysconf(_SC_NPROCESSORS_CONF);
			break;
		case HW_AVAILCPU:
			val = ::sysconf(_SC_NPROCESSORS_ONLN);
			break;
		case HW_PHYSMEM:
		case HW_USERMEM:
		{
			long pages = ::sysconf(_SC_PHYS_PAGES);
			long page_size = ::sysconf(_SC_PAGE_SIZE);
			val = pages * page_size;
			break;
		}

		default:
			LOG << "sysctl(HW) with name[1]=" << name[1] << " isn't supported yet.\n";
        
			errno = DARWIN_EINVAL;
			return -1;
		}

		if (oldp)
		{
			if (*oldlenp == 4)
				*reinterpret_cast<uint32_t*>(oldp) = val;
			else if (*oldlenp == 8)
				*reinterpret_cast<uint64_t*>(oldp) = val;
		}
		else
			*oldlenp = sizeof(long);
		return 0;
    }

	case CTL_KERN:
	{
		switch (name[1])
		{
			case KERN_OSRELEASE:
				
				// TODO: use real uname?
				if (oldp)
					strcpy((char*)oldp, "10.7.0");
				*oldlenp = 7;
				break;
				
			case KERN_OSVERSION:
				if (oldp)
					strcpy((char*)oldp, "10J869");
				*oldlenp = 7;
				break;

			default:
				LOG << "sysctl(KERN) with name[1]=" << name[1] << " isn't supported yet.\n";
				errno = DARWIN_EINVAL;
				return -1;
		}
		return 0;
	}

	default:
		LOG << "sysctl with name[0]=" << name[0] << " isn't supported yet.\n";
		errno = DARWIN_EINVAL;
		return -1;
	}
}
