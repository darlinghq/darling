/*
Copyright (C) 2016 Apple Inc. All Rights Reserved.
See LICENSE.txt for this sample’s licensing information

Abstract:
Part of Core Audio Public Utility Classes
*/

//==================================================================================================
//	Includes
//==================================================================================================

//	Self Include
#include "CADebugPrintf.h"

#if	DEBUG || CoreAudio_Debug

	#if	TARGET_OS_WIN32
		#include <stdarg.h>
		#include <stdio.h>
		#include <Windows.h>
		extern "C"
		int	CAWin32DebugPrintf(char* inFormat, ...)
		{
			char theMessage[1024];
			va_list theArguments;
			va_start(theArguments, inFormat);
			_vsnprintf(theMessage, 1024, inFormat, theArguments);
			va_end(theArguments);
			OutputDebugString(theMessage);
			return 0;
		}
	#endif
	
	#if defined(CoreAudio_UseSideFile)
		#include <unistd.h>
		FILE* sDebugPrintfSideFile = NULL;
		extern "C"
		void OpenDebugPrintfSideFile()
		{
			if(sDebugPrintfSideFile == NULL)
			{
				char theFileName[1024];
				snprintf(theFileName, sizeof(theFileName), CoreAudio_UseSideFile, getpid());
				sDebugPrintfSideFile = fopen(theFileName, "a+");
				DebugPrintfRtn(DebugPrintfFileComma "\n------------------------------\n");
			}
		}
	#endif

#endif
