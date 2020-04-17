/*
     File: CACFPreferences.cpp
 Abstract: CACFPreferences.h
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
//==================================================================================================
//	Includes
//==================================================================================================

//	Self Include
#include "CACFPreferences.h"

//	PublicUtility Includes
#include "CACFDistributedNotification.h"
#include "CADebugMacros.h"

//==================================================================================================
//	CACFPreferences
//==================================================================================================

CFPropertyListRef	CACFPreferences::CopyValue(CFStringRef inKey, bool inCurrentUser, bool inCurrentHost)
{
	//	synchronize to make sure that what's in memory matches what's on disk
	Synchronize(inCurrentUser, inCurrentHost, false);
	
	CFPropertyListRef theAnswer = NULL;
	CFStringRef theUser = inCurrentUser ? kCFPreferencesCurrentUser : kCFPreferencesAnyUser;
	CFStringRef theHost = inCurrentHost ? kCFPreferencesCurrentHost : kCFPreferencesAnyHost;
	
	theAnswer = CFPreferencesCopyValue(inKey, kCFPreferencesAnyApplication, theUser, theHost);
	
	return theAnswer;
}

CFStringRef	CACFPreferences::CopyStringValue(CFStringRef inKey, bool inCurrentUser, bool inCurrentHost)
{
	CFStringRef theAnswer = NULL;
	
	//	get the raw value
	CFPropertyListRef theRawValue = CopyValue(inKey, inCurrentUser, inCurrentHost);
	
	if(theRawValue != NULL)
	{
		//	get it's type ID and make sure it's a CFString
		CFTypeID theTypeID = CFGetTypeID(theRawValue);
		if(theTypeID == CFStringGetTypeID())
		{
			//	cast the value
			theAnswer = static_cast<CFStringRef>(theRawValue);
		}
		else
		{
			CFRelease(theRawValue);
			DebugMessage("CACFPreferences::CopyStringValue: not a CFString");
		}
	}
	
	return theAnswer;
}

CFNumberRef	CACFPreferences::CopyNumberValue(CFStringRef inKey, bool inCurrentUser, bool inCurrentHost)
{
	CFNumberRef theAnswer = NULL;
	
	//	get the raw value
	CFPropertyListRef theRawValue = CopyValue(inKey, inCurrentUser, inCurrentHost);
	
	if(theRawValue != NULL)
	{
		//	get it's type ID and make sure it's a CFNumber
		CFTypeID theTypeID = CFGetTypeID(theRawValue);
		if(theTypeID == CFNumberGetTypeID())
		{
			//	cast the value
			theAnswer = static_cast<CFNumberRef>(theRawValue);
		}
		else
		{
			CFRelease(theRawValue);
			DebugMessage("CACFPreferences::CopyNumberValue: not a CFNumber");
		}
	}
	
	return theAnswer;
}

CFArrayRef	CACFPreferences::CopyArrayValue(CFStringRef inKey, bool inCurrentUser, bool inCurrentHost)
{
	CFArrayRef theAnswer = NULL;
	
	//	get the raw value
	CFPropertyListRef theRawValue = CopyValue(inKey, inCurrentUser, inCurrentHost);
	
	if(theRawValue != NULL)
	{
		//	get it's type ID and make sure it's a CFArray
		CFTypeID theTypeID = CFGetTypeID(theRawValue);
		if(theTypeID == CFArrayGetTypeID())
		{
			//	cast the value
			theAnswer = static_cast<CFArrayRef>(theRawValue);
		}
		else
		{
			CFRelease(theRawValue);
			DebugMessage("CACFPreferences::CopyArrayValue: not a CFArray");
		}
	}
	
	return theAnswer;
}

CFDictionaryRef	CACFPreferences::CopyDictionaryValue(CFStringRef inKey, bool inCurrentUser, bool inCurrentHost)
{
	CFDictionaryRef theAnswer = NULL;
	
	//	get the raw value
	CFPropertyListRef theRawValue = CopyValue(inKey, inCurrentUser, inCurrentHost);
	
	if(theRawValue != NULL)
	{
		//	get it's type ID and make sure it's a CFDictionary
		CFTypeID theTypeID = CFGetTypeID(theRawValue);
		if(theTypeID == CFDictionaryGetTypeID())
		{
			//	cast the value
			theAnswer = static_cast<CFDictionaryRef>(theRawValue);
		}
		else
		{
			CFRelease(theRawValue);
			DebugMessage("CACFPreferences::CopyDictionaryValue: not a CFDictionary");
		}
	}
	
	return theAnswer;
}

void	CACFPreferences::SetValue(CFStringRef inKey, CFPropertyListRef inValue, bool inCurrentUser, bool inCurrentHost, bool inSynchronize)
{
	CFStringRef theUser = inCurrentUser ? kCFPreferencesCurrentUser : kCFPreferencesAnyUser;
	CFStringRef theHost = inCurrentHost ? kCFPreferencesCurrentHost : kCFPreferencesAnyHost;
	CFPreferencesSetValue(inKey, inValue, kCFPreferencesAnyApplication, theUser, theHost);
	
	if(inSynchronize)
	{
		Synchronize(inCurrentUser, inCurrentHost, true);
	}
}

void	CACFPreferences::DeleteValue(CFStringRef inKey, bool inCurrentUser, bool inCurrentHost, bool inSynchronize)
{
	CFStringRef theUser = inCurrentUser ? kCFPreferencesCurrentUser : kCFPreferencesAnyUser;
	CFStringRef theHost = inCurrentHost ? kCFPreferencesCurrentHost : kCFPreferencesAnyHost;
	CFPreferencesSetValue(inKey, NULL, kCFPreferencesAnyApplication, theUser, theHost);
	
	if(inSynchronize)
	{
		Synchronize(theUser, inCurrentHost, true);
	}
}

void	CACFPreferences::Synchronize(bool inCurrentUser, bool inCurrentHost, bool inForce)
{
	if(inForce || ArePrefsOutOfDate(inCurrentUser, inCurrentHost))
	{
		CFStringRef theUser = inCurrentUser ? kCFPreferencesCurrentUser : kCFPreferencesAnyUser;
		CFStringRef theHost = inCurrentHost ? kCFPreferencesCurrentHost : kCFPreferencesAnyHost;
		CFPreferencesSynchronize(kCFPreferencesAnyApplication, theUser, theHost);
		MarkPrefsClean(inCurrentUser, inCurrentHost);
	}
}

void	CACFPreferences::MarkPrefsOutOfDate(bool inCurrentUser, bool inCurrentHost)
{
	if(!inCurrentUser && !inCurrentHost)
	{
		sAnyUserAnyHostPrefsOutOfDate = true;
	}
	else if(!inCurrentUser && inCurrentHost)
	{
		sAnyUserCurrentHostPrefsOutOfDate = true;
	}
	else if(inCurrentUser && !inCurrentHost)
	{
		sCurrentUserAnyHostPrefsOutOfDate = true;
	}
	else if(inCurrentUser && inCurrentHost)
	{
		sCurrentUserCurrentHostPrefsOutOfDate = true;
	}
}

void	CACFPreferences::MarkPrefsClean(bool inCurrentUser, bool inCurrentHost)
{
	if(!inCurrentUser && !inCurrentHost)
	{
		sAnyUserAnyHostPrefsOutOfDate = false;
	}
	else if(!inCurrentUser && inCurrentHost)
	{
		sAnyUserCurrentHostPrefsOutOfDate = false;
	}
	else if(inCurrentUser && !inCurrentHost)
	{
		sCurrentUserAnyHostPrefsOutOfDate = false;
	}
	else if(inCurrentUser && inCurrentHost)
	{
		sCurrentUserCurrentHostPrefsOutOfDate = false;
	}
}

void	CACFPreferences::SendNotification(CFStringRef inName)
{
	CACFDistributedNotification::PostNotification(inName, NULL, true);
}

bool	CACFPreferences::ArePrefsOutOfDate(bool inCurrentUser, bool inCurrentHost)
{
	bool theAnswer = false;
	
	if(!inCurrentUser && !inCurrentHost)
	{
		theAnswer = sAnyUserAnyHostPrefsOutOfDate;
	}
	else if(!inCurrentUser && inCurrentHost)
	{
		theAnswer = sAnyUserCurrentHostPrefsOutOfDate;
	}
	else if(inCurrentUser && !inCurrentHost)
	{
		theAnswer = sCurrentUserAnyHostPrefsOutOfDate;
	}
	else if(inCurrentUser && inCurrentHost)
	{
		theAnswer = sCurrentUserCurrentHostPrefsOutOfDate;
	}
	
	return theAnswer;
}

bool	CACFPreferences::sAnyUserAnyHostPrefsOutOfDate = false;
bool	CACFPreferences::sAnyUserCurrentHostPrefsOutOfDate = false;
bool	CACFPreferences::sCurrentUserAnyHostPrefsOutOfDate = false;
bool	CACFPreferences::sCurrentUserCurrentHostPrefsOutOfDate = false;
