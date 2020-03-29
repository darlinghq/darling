/*
     File: MIDIControlHandler.h
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
#ifndef __MIDICONTROLHANDLER_H__
#define	__MIDICONTROLHANDLER_H__

#include <CoreAudio/CoreAudio.h>

/*! Abstract interface base class for classes which handle all incoming MIDI data */

class MIDIControlHandler
{
public:
	virtual			~MIDIControlHandler() {}
	virtual void	Reset() = 0;								//! Restore all state to defaults
	virtual bool	SetProgramChange(UInt16	inProgram) = 0;
	virtual bool	SetPitchWheel(UInt16 inValue) = 0;
	virtual bool	SetChannelPressure(UInt8 inValue) = 0;
	virtual bool	SetPolyPressure(UInt8 inKey, UInt8 inValue) = 0;
	virtual bool	SetController(UInt8 inControllerNumber, UInt8 inValue) = 0;
	virtual bool	SetSysex(void *inSysexMsg) = 0;
	
	virtual float	GetPitchBend() const = 0;

	/*! Default controller values.  These represent MSB values unless indicated in the name */
	
	enum
	{
		kDefault_Midpoint				= 0x40,		//! Used for all center-null-point controllers
		kDefault_Volume				= 100,
		kDefault_Pan					= kDefault_Midpoint,
		kDefault_ModWheel				= 0,
		kDefault_Pitch					= kDefault_Midpoint,
		kDefault_Expression			= 0x7f,
		kDefault_ChannelPressure	= 0,
		kDefault_ReverbSend			= 40,
		kDefault_ChorusSend			= 0,
		
		kDefault_RPN_LSB				= 0x7f,
		kDefault_RPN_MSB				= 0x7f,
		kDefault_PitchBendRange		= 2,
		kDefault_FineTuning			= kDefault_Midpoint,
		kDefault_CoarseTuning		= kDefault_Midpoint,
		kDefault_ModDepthRange		= 0,
		kDefault_ModDepthRangeLSB	= kDefault_Midpoint
	};
};

#endif	// __MIDICONTROLHANDLER_H__
