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

