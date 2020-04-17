/*
     File: CACFMessagePort.cpp
 Abstract: CACFMessagePort.h
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
//=============================================================================
//	Includes
//=============================================================================

#include "CACFMessagePort.h"
#include "CADebugMacros.h"
#include "CAException.h"

//=============================================================================
//	CACFLocalMessagePort
//=============================================================================

CACFLocalMessagePort::CACFLocalMessagePort(CFStringRef inName, CFMessagePortCallBack inPortCallBack, CFMessagePortInvalidationCallBack inInvalidationCallBack, void* inUserData)
:
	mMessagePort(NULL),
	mRunLoopSource(NULL),
	mDispatchQueue(NULL)
{
	//	create the CFMessagePort
	CFMessagePortContext theContext = { 0, inUserData, NULL, NULL, NULL };
	mMessagePort = CFMessagePortCreateLocal(NULL, inName, inPortCallBack, &theContext, NULL);
	if(mMessagePort != NULL)
	{
		//	add the invalidation callback, if any
		if(inInvalidationCallBack != NULL)
		{
			CFMessagePortSetInvalidationCallBack(mMessagePort, inInvalidationCallBack);
		}
	}
}

CACFLocalMessagePort::~CACFLocalMessagePort()
{
	if(mRunLoopSource != NULL)
	{
		CFRelease(mRunLoopSource);
	}
	if(mMessagePort != NULL)
	{
		CFMessagePortInvalidate(mMessagePort);
		CFRelease(mMessagePort);
	}
}

CFRunLoopSourceRef	CACFLocalMessagePort::GetRunLoopSource() const
{
	Assert(mDispatchQueue == NULL, "CACFLocalMessagePort::SetDispatchQueue: should have both a run loop source and a dispatch queue");
	if(mRunLoopSource == NULL)
	{
		const_cast<CACFLocalMessagePort*>(this)->mRunLoopSource = CFMessagePortCreateRunLoopSource(NULL, mMessagePort, 0);
	}
	return mRunLoopSource;
}

void	CACFLocalMessagePort::SetDispatchQueue(dispatch_queue_t inDispatchQueue)
{
	Assert(mRunLoopSource == NULL, "CACFLocalMessagePort::SetDispatchQueue: should have both a run loop source and a dispatch queue");
	mDispatchQueue = inDispatchQueue;
	CFMessagePortSetDispatchQueue(mMessagePort, mDispatchQueue);
}

//=============================================================================
//	CACFRemoteMessagePort
//=============================================================================

CACFRemoteMessagePort::CACFRemoteMessagePort(CFStringRef inName, CFMessagePortInvalidationCallBack inInvalidationCallBack)
:
	mMessagePort(NULL),
	mRunLoopSource(NULL),
	mDispatchQueue(NULL)
{
	//	create the CFMessagePort
	mMessagePort = CFMessagePortCreateRemote(NULL, inName);
	if(mMessagePort != NULL)
	{
		//	failure to create a remote port does not need to throw an exception
		//	because it isn't really an error since the port in question may not
		//	exist and this fact requires a more complex response than an excpeption
		//	provides for.
		
		//	add the invalidation callback, if any
		if(inInvalidationCallBack != NULL)
		{
			CFMessagePortSetInvalidationCallBack(mMessagePort, inInvalidationCallBack);
		}
	}
}

CACFRemoteMessagePort::~CACFRemoteMessagePort()
{
	if(mRunLoopSource != NULL)
	{
		CFRelease(mRunLoopSource);
	}
	if(mMessagePort != NULL)
	{
		//CFMessagePortInvalidate(mMessagePort);
		CFRelease(mMessagePort);
	}
}

CFRunLoopSourceRef	CACFRemoteMessagePort::GetRunLoopSource() const
{
	Assert(mDispatchQueue == NULL, "CACFRemoteMessagePort::SetDispatchQueue: should have both a run loop source and a dispatch queue");
	if(mRunLoopSource == NULL)
	{
		const_cast<CACFRemoteMessagePort*>(this)->mRunLoopSource = CFMessagePortCreateRunLoopSource(NULL, mMessagePort, 0);
	}
	return mRunLoopSource;
}

void	CACFRemoteMessagePort::SetDispatchQueue(dispatch_queue_t inDispatchQueue)
{
	Assert(mRunLoopSource == NULL, "CACFRemoteMessagePort::SetDispatchQueue: should have both a run loop source and a dispatch queue");
	mDispatchQueue = inDispatchQueue;
	CFMessagePortSetDispatchQueue(mMessagePort, mDispatchQueue);
}
