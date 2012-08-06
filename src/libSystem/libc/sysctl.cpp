#include "sysctl.h"
#include "log.h"
#include <unistd.h>

int __darwin_sysctl(int* name, u_int namelen,
                    void* oldp, size_t* oldlenp,
                    void* newp, size_t newlen) {
	int i;
	
	LOGF("sysctl: namelen=%u", namelen);
	for (i = 0; i < namelen; i++)
		LOGF(" name[%d]=%d", i, name[i]);
	LOGF(" newp=%p\n", newp);

	if (newp)
	{
		fprintf(stderr, "sysctl with newp isn't supported yet.\n");
		errno = DARWIN_EINVAL;
		return -1;
	}

	if (namelen != 2)
	{
		fprintf(stderr, "sysctl with namelen=%u isn't supported yet.\n", namelen);
		errno = DARWIN_EINVAL;
		return -1;
	}

	switch (name[0])
	{
	case CTL_HW:
	{
		if (*oldlenp != 4 && *oldlenp != 8)
		{
			fprintf(stderr, "sysctl(HW) with oldlenp=%lu isn't supported yet.\n", (unsigned long)*oldlenp);
			errno = DARWIN_EINVAL;
			return -1;
		}

		uint64_t val = 0;
		switch (name[1])
		{
		case HW_NCPU:
			val = sysconf(_SC_NPROCESSORS_ONLN);
			break;
		case HW_AVAILCPU:
			val = ::sysconf(_SC_NPROCESSORS_CONF);
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
			fprintf(stderr, "sysctl(HW) with name[1]=%d isn't supported yet.\n", name[1]);
        
			errno = DARWIN_EINVAL;
			return -1;
		}

		if (*oldlenp == 4)
			*reinterpret_cast<uint32_t*>(oldp) = val;
		else if (*oldlenp == 8)
			*reinterpret_cast<uint64_t*>(oldp) = val;
		return 0;
    }

	case CTL_KERN:
	{
		switch (name[1])
		{
			case KERN_OSRELEASE:
				
				// TODO: use real uname?
				strcpy((char*)oldp, "10.6.0");
				*oldlenp = 7;
				break;

			default:
				fprintf(stderr, "sysctl(KERN) with oldp=%u isn't supported yet.\n", *oldp);
				errno = DARWIN_EINVAL;
				return -1;
		}
		return 0;
	}

	default:
		fprintf(stderr, "sysctl with name[0]=%d isn't supported yet.\n", name[0]);
		errno = DARWIN_EINVAL;
		return -1;
	}
}
