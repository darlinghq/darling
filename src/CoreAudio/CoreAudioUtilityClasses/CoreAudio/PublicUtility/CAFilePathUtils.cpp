/*
     File: CAFilePathUtils.cpp
 Abstract: CAFilePathUtils.h
  Version: 1.1
 
 Disclaimer: IMPORTANT:  This Apple software is supplied to you by Apple
 Inc. ("Apple") in consideration of your agreement to the following
 terms, and your use, installation, modification or redistribution of
 this Apple software constitutes acceptance of these terms.  If you do
 not agree with these terms, please do not use, install, modify or
 redistribute this Apple software.
 
 In consideration of your agreement to abide by the following terms, and
 subject to these terms, Apple grants you a personal, non-exclusive
 license, under Apple's copyrights in this original Apple software (the
 "Apple Software"), to use, reproduce, modify and redistribute the Apple
 Software, with or without modifications, in source and/or binary forms;
 provided that if you redistribute the Apple Software in its entirety and
 without modifications, you must retain this notice and the following
 text and disclaimers in all such redistributions of the Apple Software.
 Neither the name, trademarks, service marks or logos of Apple Inc. may
 be used to endorse or promote products derived from the Apple Software
 without specific prior written permission from Apple.  Except as
 expressly stated in this notice, no other rights or licenses, express or
 implied, are granted by Apple herein, including but not limited to any
 patent rights that may be infringed by your derivative works or by other
 works in which the Apple Software may be incorporated.
 
 The Apple Software is provided by Apple on an "AS IS" basis.  APPLE
 MAKES NO WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION
 THE IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS
 FOR A PARTICULAR PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND
 OPERATION ALONE OR IN COMBINATION WITH YOUR PRODUCTS.
 
 IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL
 OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION,
 MODIFICATION AND/OR DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER CAUSED
 AND WHETHER UNDER THEORY OF CONTRACT, TORT (INCLUDING NEGLIGENCE),
 STRICT LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN ADVISED OF THE
 POSSIBILITY OF SUCH DAMAGE.
 
 Copyright (C) 2014 Apple Inc. All Rights Reserved.
 
*/
#include "CAFilePathUtils.h"
#include <string.h>

#if !CA_NO_CORE_SERVICES
#if !defined(__COREAUDIO_USE_FLAT_INCLUDES__)
	#include <CoreServices/CoreServices.h> // FSRef
    #include <CoreAudio/CoreAudioTypes.h>
#else
	#include <CoreServices.h>
	#include <CoreAudioTypes.h>
#endif

OSStatus	PosixPathToParentFSRefAndName(const char *path, FSRef &outParentDir, CFStringRef &outFileName)
{
	// convert C string to CFString
#if !TARGET_OS_WIN32
	CFStringRef cfFullPath = CFStringCreateWithCString(NULL, path, kCFStringEncodingUTF8);
#else
	CFStringRef cfFullPath = CFStringCreateWithCString(NULL, path, kCFStringEncodingWindowsLatin1);
#endif
	// convert CF string to URL
	CFURLRef fullurl = CFURLCreateWithFileSystemPath(NULL, cfFullPath, TARGET_OS_WIN32 ? kCFURLWindowsPathStyle : kCFURLPOSIXPathStyle, false);
	CFRelease(cfFullPath);
	// get the directory portion of the URL
	CFURLRef dirurl = CFURLCreateCopyDeletingLastPathComponent(NULL, fullurl);
	// get the directory's FSRef
	OSStatus err = CFURLGetFSRef(dirurl, &outParentDir) ? OSStatus(noErr) : OSStatus(kAudio_FileNotFoundError);
	CFRelease(dirurl);
	
	CFStringRef lastPathComponent = CFURLCopyLastPathComponent(fullurl);
	CFRelease(fullurl);
	CFMutableStringRef filename = CFStringCreateMutableCopy(NULL, 0, lastPathComponent);
	CFRelease(lastPathComponent);
	// convert colons (legal in POSIX paths, illegal in File Manager) to slashes
	CFStringFindAndReplace(filename, CFSTR(":"), CFSTR("/"), CFRangeMake(0, CFStringGetLength(filename)), 0);
	
	outFileName = filename;
	
	return err;
}
#endif // !CA_NO_CORE_SERVICES


#if TARGET_OS_WIN32

char*	dirname(const char* inPath)
{
	static char sAnswer[1024];
	
	char* theAnswer = NULL;
	SInt32 thePathLength = strlen(inPath);
	if(thePathLength < 1023)
	{
		//	make a working copy
		strlcpy(sAnswer, inPath, sizeof(sAnswer));
		
		//	start at the end of the string
		SInt32 theIndex = thePathLength - 1;
		
		//	walk back over the '\' characters
		while((theIndex > 0) && (sAnswer[theIndex] == '\\'))
		{
			--theIndex;
		}
		
		//	now keep walking back until we get to a '\'
		while((theIndex > 0) && (sAnswer[theIndex] != '\\'))
		{
			--theIndex;
		}
		
		//	where we are now is either the first character of the path or the '\' that marks the end of the directory name
		if(theIndex > 0)
		{
			//	we have a name so put a '\0' in place of the '\'
			sAnswer[theIndex] = 0;
		}
		else
		{
			//	no name, so the answer is "."
			sAnswer[0] = '.';
			sAnswer[1] = 0;
		}
		
		//	set the return value
		theAnswer = sAnswer;
	}
	
	return theAnswer;
}

char*	basename(const char* inPath)
{
	static char sAnswer[1024];
	
	char* theAnswer = NULL;
	SInt32 thePathLength = strlen(inPath);
	if(thePathLength < 1023)
	{
		//	make a working copy
		strlcpy(sAnswer, inPath, sizeof(sAnswer));
		
		//	start at the end of the string
		SInt32 theLastIndex = thePathLength - 1;
		
		//	walk back over the '\' characters
		while((theLastIndex > 0) && (sAnswer[theLastIndex] == '\\'))
		{
			--theLastIndex;
		}
		
		//	check to see if we're at the beginning now
		if(theLastIndex > 0)
		{
			//	there's a name in there now, so start where we are and go back to the next '\'
			UInt32 theFirstIndex = theLastIndex;
			while((theFirstIndex > 0) && (sAnswer[theFirstIndex] != '\\'))
			{
				--theFirstIndex;
			}
			
			//	we now have a string, so put a '\0' after the last character
			sAnswer[theLastIndex + 1] = 0;
			
			//	and set the return value
			theAnswer = &sAnswer[theFirstIndex];
		}
		else
		{
			//	the path was entirely '\' characters, so the return value is "\"
			sAnswer[0] = '\\';
			sAnswer[1] = 0;
		
			//	set the return value
			theAnswer = sAnswer;
		}
	}
	
	return theAnswer;
}

#endif
