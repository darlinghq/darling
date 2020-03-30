/*
     File: CACFMachPort.cpp
 Abstract: CACFMachPort.h
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

#include "CACFMachPort.h"
#include "CAException.h"
#include "CADebugMacros.h"

//==================================================================================================
//	CACFMachPort
//==================================================================================================

//	This constructor is the short form. The CFMachPort will own the send and receive rights.
CACFMachPort::CACFMachPort(CFMachPortCallBack inCallBack, void* inUserData)
:
	mMachPort(NULL),
	mRunLoopSource(NULL),
	mOwnsPort(true)
{
	CFMachPortContext theContext = { 1, inUserData, NULL, NULL, NULL };
	mMachPort = CFMachPortCreate(NULL, inCallBack, &theContext, NULL);
	ThrowIfNULL(mMachPort, CAException('what'), "CACFMachPort::CACFMachPort(s): couldn't create the CFMachPort");

	mRunLoopSource = CFMachPortCreateRunLoopSource(NULL, mMachPort, 0);
	if(mRunLoopSource == NULL)
	{
		CFMachPortInvalidate(mMachPort);
		CFRelease(mMachPort);
		mMachPort = NULL;
		DebugMessage("CACFMachPort::CACFMachPort(s): couldn't create the CFRunLoopSource");
		throw CAException('what');
	}
}

//	This constructor is the general form:
//	-	If inMachPort is MACH_PORT_NULL, the CFMachPort will allocate the port and own the send and
//		receive rights. Otherwise, the caller owns the rights and is resposible for cleaning them
//		up.
//	-	If inCallBack is NULL, then received messages will just get swallowed by the CFMachPort.
//		This is useful if you are only using the CFMachPort to track port death (aka invalidation).
//	-	If inInvalidationCallBack is non-NULL, then it will be installed as the invalidation
//		callback on the CFMachPort.
CACFMachPort::CACFMachPort(mach_port_t inMachPort, CFMachPortCallBack inCallBack, CFMachPortInvalidationCallBack inInvalidationCallBack, void* inUserData)
:
	mMachPort(NULL),
	mRunLoopSource(NULL),
	mOwnsPort(false)
{
	CFMachPortContext theContext = { 1, inUserData, NULL, NULL, NULL };
	
	if(inMachPort == MACH_PORT_NULL)
	{
		mMachPort = CFMachPortCreate(NULL, inCallBack, &theContext, NULL);
		ThrowIfNULL(mMachPort, CAException('what'), "CACFMachPort::CACFMachPort: couldn't create the CFMachPort");
		mOwnsPort = true;
	}
	else
	{
		mMachPort = CFMachPortCreateWithPort(NULL, inMachPort, inCallBack, &theContext, NULL);
		ThrowIfNULL(mMachPort, CAException('what'), "CACFMachPort::CACFMachPort: couldn't create the CFMachPort with a port");
		mOwnsPort = false;
	}

	mRunLoopSource = CFMachPortCreateRunLoopSource(NULL, mMachPort, 0);
	if(mRunLoopSource == NULL)
	{
		if(mOwnsPort)
		{
			CFMachPortInvalidate(mMachPort);
		}
		CFRelease(mMachPort);
		mMachPort = NULL;
		DebugMessage("CACFMachPort::CACFMachPort: couldn't create the CFRunLoopSource");
		throw CAException('what');
	}
	
	if(inInvalidationCallBack != NULL)
	{
		CFMachPortSetInvalidationCallBack(mMachPort, inInvalidationCallBack);
	}
}

CACFMachPort::~CACFMachPort()
{
	if(mRunLoopSource != NULL)
	{
		CFRelease(mRunLoopSource);
	}
	
	if(mMachPort != NULL)
	{
		if(mOwnsPort)
		{
			CFMachPortInvalidate(mMachPort);
		}
		CFRelease(mMachPort);
	}
}

kern_return_t	CACFMachPort::ReceiveMessage(UInt32 inMaxMessageSize, mach_msg_header_t* outMessage, mach_msg_timeout_t inTimeOut)
{
	//	snag the port
	mach_port_t thePort = CFMachPortGetPort(mMachPort);

	//	fill out the message header
	outMessage->msgh_bits = 0;
	outMessage->msgh_size = 0;
	outMessage->msgh_remote_port = MACH_PORT_NULL;
	outMessage->msgh_local_port = thePort;
	outMessage->msgh_reserved = 0;
	outMessage->msgh_id = 0;

	//	figure the options
	mach_msg_options_t theOptions = MACH_RCV_MSG;
	if(inTimeOut > 0)
	{
		theOptions |= MACH_RCV_TIMEOUT;
	}
	
	//	receive the messsage
	return mach_msg(outMessage, theOptions, 0, inMaxMessageSize, thePort, inTimeOut, MACH_PORT_NULL);
}
