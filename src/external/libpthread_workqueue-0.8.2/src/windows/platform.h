#ifndef _PTWQ_WINDOWS_PLATFORM_H
#define _PTWQ_WINDOWS_PLATFORM_H 1

#define PROVIDE_LEGACY_XP_SUPPORT 1

#ifdef PROVIDE_LEGACY_XP_SUPPORT
# define _WIN32_WINNT 0x0500
#else
# define _WIN32_WINNT 0x0610
#endif
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <string.h>
#include "winpthreads.h"

/* Instead of __attribute__ ((constructor)), use DllMain() */
#define CONSTRUCTOR	  /* */

#define VISIBLE __declspec(dllexport)

# define __func__ __FUNCTION__

#undef LIST_HEAD
#include "queue.h"

#define sleep(sec) 	Sleep(1000*sec)
#define strdup(p)	_strdup(p)
#define random()	rand()

#ifdef PROVIDE_LEGACY_XP_SUPPORT
# define WORKQUEUE_PLATFORM_SPECIFIC \
	LIST_ENTRY(_pthread_workqueue) wqlist_entry
#else
/* Specific workqueue items */
# define WORKQUEUE_PLATFORM_SPECIFIC \
	PTP_POOL win_thread_pool; \
	TP_CALLBACK_ENVIRON win_callback_env
#endif


#endif  /* _PTWQ_WINDOWS_PLATFORM_H */
