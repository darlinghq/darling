/*
Copyright (C) 2016 Apple Inc. All Rights Reserved.
See LICENSE.txt for this sample’s licensing information

Abstract:
Part of Core Audio Public Utility Classes
*/

//=============================================================================
//	Includes
//=============================================================================

#include "CADebugger.h"

//=============================================================================
//	CADebugger
//=============================================================================

#if TARGET_API_MAC_OSX

#include <sys/sysctl.h>
#include <stdlib.h>
#include <unistd.h>

bool CAIsDebuggerAttached(void)
{
	int					mib[4];
	struct kinfo_proc	info;
	size_t				size;

	mib[0] = CTL_KERN;
	mib[1] = KERN_PROC;
	mib[2] = KERN_PROC_PID;
	mib[3] = getpid();
	size = sizeof(info);
	info.kp_proc.p_flag = 0;

	sysctl(mib, 4, &info, &size, NULL, 0);

	return (info.kp_proc.p_flag & P_TRACED) == P_TRACED;
}

#endif

void	CADebuggerStop(void)
{
	#if	CoreAudio_Debug
		#if	TARGET_API_MAC_OSX
			if(CAIsDebuggerAttached())
			{
				#if defined(__i386__) || defined(__x86_64__)
					asm("int3");
				#else
					__builtin_trap();
				#endif
			}
			else
			{
				abort();
			}
		#else
			__debugbreak();
		#endif
	#endif
}
