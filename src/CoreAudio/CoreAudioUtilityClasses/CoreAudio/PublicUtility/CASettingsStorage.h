/*
     File: CASettingsStorage.h
 Abstract: Part of CoreAudio Utility Classes
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
#if !defined(__CASettingsStorage_h__)
#define __CASettingsStorage_h__

//==================================================================================================
//	Includes
//==================================================================================================

//	System Includes
#include <CoreAudio/CoreAudioTypes.h>
#include <CoreFoundation/CoreFoundation.h>

//	Stamdard Library Includes
#include <stdio.h>
#include <sys/stat.h>

//==================================================================================================
//	CASettingsStorage
//==================================================================================================

class CASettingsStorage
{

//	Construction/Destruction
public:
							CASettingsStorage(const char* inSettingsFilePath, mode_t inSettingsFileAccessMode = 0, CFPropertyListFormat inSettingsCacheFormat = kCFPropertyListXMLFormat_v1_0, bool inIsSingleProcessOnly = false, bool inIsReadOnly = false);
							~CASettingsStorage();

//	Operations
public:
	UInt32					GetNumberKeys() const;
	void					GetKeys(UInt32 inNumberKeys, UInt32& outNumberKeys, CFStringRef* outKeys) const;

	void					CopyBoolValue(const CFStringRef inKey, bool& outValue, bool inDefaultValue = false) const;
	void					CopySInt32Value(const CFStringRef inKey, SInt32& outValue, SInt32 inDefaultValue = 0) const;
	void					CopyUInt32Value(const CFStringRef inKey, UInt32& outValue, UInt32 inDefaultValue = 0) const;
	void					CopySInt64Value(const CFStringRef inKey, SInt64& outValue, SInt64 inDefaultValue = 0) const;
	void					CopyUInt64Value(const CFStringRef inKey, UInt64& outValue, UInt64 inDefaultValue = 0) const;
	void					CopyFloat32Value(const CFStringRef inKey, Float32& outValue, Float32 inDefaultValue = 0) const;
	void					CopyFloat64Value(const CFStringRef inKey, Float64& outValue, Float64 inDefaultValue = 0) const;
	void					CopyNumberValue(const CFStringRef inKey, CFNumberRef& outValue, CFNumberRef inDefaultValue = NULL) const;	
	void					CopyStringValue(const CFStringRef inKey, CFStringRef& outValue, CFStringRef inDefaultValue = NULL) const;	
	void					CopyArrayValue(const CFStringRef inKey, CFArrayRef& outValue, CFArrayRef inDefaultValue = NULL) const;	
	void					CopyDictionaryValue(const CFStringRef inKey, CFDictionaryRef& outValue, CFDictionaryRef inDefaultValue = NULL) const;	
	void					CopyDataValue(const CFStringRef inKey, CFDataRef& outValue, CFDataRef inDefaultValue = NULL) const;
	void					CopyCFTypeValue(const CFStringRef inKey, CFTypeRef& outValue, CFTypeRef inDefaultValue = NULL) const;
	
	void					SetSInt32Value(const CFStringRef inKey, SInt32 inValue);
	void					SetUInt32Value(const CFStringRef inKey, UInt32 inValue);
	void					SetSInt64Value(const CFStringRef inKey, SInt64 inValue);
	void					SetUInt64Value(const CFStringRef inKey, UInt64 inValue);
	void					SetFloat32Value(const CFStringRef inKey, Float32 inValue);
	void					SetFloat64Value(const CFStringRef inKey, Float64 inValue);
	void					SetNumberValue(const CFStringRef inKey, const CFNumberRef inValue);
	void					SetStringValue(const CFStringRef inKey, const CFStringRef inValue);
	void					SetArrayValue(const CFStringRef inKey, const CFArrayRef inValue);
	void					SetDictionaryValue(const CFStringRef inKey, const CFDictionaryRef inValue);
	void					SetDataValue(const CFStringRef inKey, const CFDataRef inValue);
	void					SetCFTypeValue(const CFStringRef inKey, const CFTypeRef inValue);
	
	void					RemoveValue(const CFStringRef inKey);
	void					RemoveAllValues();
	
	void					SendNotification(const CFStringRef inName, CFDictionaryRef inData = NULL, bool inPostToAllSessions = true) const;
	void					ForceRefresh();

//	Implementation
private:
	void					RefreshSettings();
	void					SaveSettings();

	char*					mSettingsFilePath;
	mode_t					mSettingsFileAccessMode;
	CFMutableDictionaryRef	mSettingsCache;
	CFPropertyListFormat	mSettingsCacheFormat;
	struct timespec			mSettingsCacheTime;
	bool					mSettingsCacheForceRefresh;
	bool					mIsSingleProcessOnly;
	bool					mIsReadOnly;

};

#endif
