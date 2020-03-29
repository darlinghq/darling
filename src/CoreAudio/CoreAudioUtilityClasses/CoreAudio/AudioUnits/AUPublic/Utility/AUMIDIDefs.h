/*
     File: AUMIDIDefs.h
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
#ifndef __AUMIDIDefs_h__
#define __AUMIDIDefs_h__

#if !defined(__TMidiMessage)	/* DAS HACK */
enum
{
	kMidiMessage_NoteOff 			= 0x80,
	kMidiMessage_NoteOn 			= 0x90,
	kMidiMessage_PolyPressure 		= 0xA0,
	kMidiMessage_ControlChange 		= 0xB0,
	kMidiMessage_ProgramChange 		= 0xC0,
	kMidiMessage_ChannelPressure 	= 0xD0,
	kMidiMessage_PitchWheel 		= 0xE0,
	kMidiMessage_SysEx				= 0xF0,
	kMidiMessage_SysEx_End			= 0xF7,
	kMidiMessage_MetaEvent			= 0xFF
};
#endif

enum
{
	kMidiController_BankSelect				= 0,
	kMidiController_ModWheel				= 1,
	kMidiController_Breath					= 2,
	kMidiController_Foot					= 4,
	kMidiController_PortamentoTime			= 5,
	kMidiController_DataEntry				= 6,
	kMidiController_Volume					= 7,
	kMidiController_Balance					= 8,
	kMidiController_Pan						= 10,
	kMidiController_Expression				= 11,
	
	// these controls have a (0-63) == off, (64-127) == on
	kMidiController_Sustain					= 64, //hold1
	kMidiController_Portamento				= 65,
	kMidiController_Sostenuto				= 66,
	kMidiController_Soft					= 67,
	kMidiController_LegatoPedal				= 68,
	kMidiController_Hold2Pedal				= 69,
	kMidiController_FilterResonance			= 71,
	kMidiController_ReleaseTime				= 72,
	kMidiController_AttackTime				= 73,
	kMidiController_Brightness				= 74,
	kMidiController_DecayTime				= 75,
	kMidiController_VibratoRate				= 76,
	kMidiController_VibratoDepth			= 77,
	kMidiController_VibratoDelay			= 78,
	
	// these controls have a 0-127 range and in MIDI they have no LSB (so fractional values are lost in MIDI)
	kMidiController_ReverbLevel				= 91,
	kMidiController_ChorusLevel				= 93,

	kMidiController_RPN_LSB					= 100,
	kMidiController_RPN_MSB					= 101,

	kMidiController_AllSoundOff				= 120,
	kMidiController_ResetAllControllers		= 121,
	kMidiController_AllNotesOff				= 123,
	kMidiController_OmniModeOff				= 124,
	kMidiController_OmniModeOn				= 125,
    kMidiController_MonoModeOn              = 126,
    kMidiController_MonoModeOff             = 127
};

// RPN values
enum 
{
	kMidiControllerValue_RPNPitchBendSensitivity	= 0,
	kMidiControllerValue_RPNChannelFineTuning		= 1,
	kMidiControllerValue_RPNChannelCoarseTuning		= 2,
	kMidiControllerValue_RPNModDepthRange			= 5,
	kMidiControllerValue_RPNNull					= 0x3fff	//! 0x7f/0x7f
};

// GM2 Sound Bank Constants
enum 
{
	kGM2MelodicBank						= 0x7900,
	kGM2PercussionBank					= 0x7800,
	kGSPercussionBank					= 0x7f00,
	kXGSFXBank							= 0x7E00,
	kXGPercussionBank					= kGSPercussionBank,
	kBankMSBMask						= 0xff00
};

enum 
{
	kMSBController_MidPoint			= 0x40
};

#endif	// __AUMIDIDefs_h__

