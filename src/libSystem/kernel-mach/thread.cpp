#include "thread.h"
#include <fstream>
#include <sstream>
#include <unistd.h>
#include <mach/kern_return.h>
#include "../libc/pthread.h"

kern_return_t thread_info(thread_act_t target_act, thread_flavor_t flavor, void* infoOut, mach_msg_type_number_t* count)
{
	if (!infoOut || !count)
		return KERN_INVALID_ARGUMENT;

	switch (flavor)
	{
		case THREAD_IDENTIFIER_INFO:
		{
			std::stringstream ss;
			std::ifstream fs;
			std::string procpath;
			thread_identifier_info* info = static_cast<thread_identifier_info*>(infoOut);

			if (*count < THREAD_IDENTIFIER_INFO_COUNT)
				return KERN_NO_SPACE;
			*count = THREAD_IDENTIFIER_INFO_COUNT;

			//info->thread_handle = Darling::tidForPthread(pthread_t(target_act));
			info->thread_handle = target_act;
			info->dispatch_qaddr = 0;
			info->thread_id = info->thread_handle;

			ss << "/proc/" << getpid() << "/task/" << info->thread_handle << "/stat";
			procpath = ss.str();
			fs.open(procpath.c_str(), std::ios_base::in);

			if (fs.is_open())
			{
				uint64_t dummy;
				for (int i = 0; i < 22; i++)
					fs >> dummy;
				info->thread_id = info->thread_handle << 32;
				info->thread_id |= dummy & 0xffffffff; // OR thread start time into lower 32 bits
			}

			return KERN_SUCCESS;
		}
		case THREAD_BASIC_INFO:
			return KERN_FAILURE;
		default:
			return KERN_INVALID_ARGUMENT;
	}
}

