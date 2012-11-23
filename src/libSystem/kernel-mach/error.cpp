#include "error.h"
#include <cstdio>

const char* mach_error_string(kern_return_t code)
{
	switch (code)
	{
		case KERN_SUCCESS:
			return "Success";
		case KERN_FAILURE:
			return "Generic failure";
		case KERN_INVALID_ARGUMENT:
			return "Invalid argument";
		case KERN_ALREADY_WAITING:
			return "Another thread is already waiting for the operation";
		case KERN_RESOURCE_SHORTAGE:
			return "Out of memory";
		case KERN_LOCK_OWNED_SELF:
			return "The calling thread already owns the lock";
		case KERN_LOCK_OWNED:
			return "The lock is already owned by another thread";
		case KERN_INVALID_ADDRESS:
			return "Invalid address";
		case KERN_NO_SPACE:
			return "No virtual memory space";
		case KERN_PROTECTION_FAILURE:
			return "Access denied";
		default:
		{
			static __thread char buf[20];
			sprintf(buf, "Error code %d", code);
			return buf;
		}
	}
}

void mach_error(const char* str, kern_return_t code)
{
	const char* msg = mach_error_string(code);
	fprintf(stderr, "%s %s\n", str, msg);
}

const char* mach_error_type(kern_return_t code)
{
	return "error type dummy";
}

