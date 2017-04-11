#include "__pthread_canceled.h"
#include "../base.h"
#include "../errno.h"
#include "../signal/duct_signals.h"
#include "../../../../../platform-include/sys/errno.h"
#include <stddef.h>
#include "../simple.h"

long sys_pthread_canceled(int action)
{
	switch (action)
	{
		case 0: // has the thread been canceled with __pthread_markcancel?
			// return 0 if yes
			return -EAGAIN;
		case 1: // enable cancellation
			break;
		case 2: // disable cancellation
			break;
	}
}

