/*
This file is part of Darling.

Copyright (C) 2026 Lubos Dolezel

Darling is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Darling is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Darling.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef _CARBONSOUND_SOUND_H
#define _CARBONSOUND_SOUND_H

#include <CoreServices/MacTypes.h>
#include <CarbonCore/MacErrors.h>

#ifdef __cplusplus
extern "C" {
#endif

// The Sound Manager structures use 2-byte packing, as in the public Sound.h.
#pragma pack(push, 2)

enum {
	stdQLength = 128
};

// Synthesizers a channel can be opened for.
enum {
	squareWaveSynth = 1,
	waveTableSynth = 3,
	sampledSynth = 5
};

// Initialization options of a channel.
enum {
	initChanLeft = 0x0002,
	initChanRight = 0x0003,
	initNoInterp = 0x0004,
	initNoDrop = 0x0008,
	initMono = 0x0080,
	initStereo = 0x00C0,
	initMACE3 = 0x0300,
	initMACE6 = 0x0400,
	initPanMask = 0x0003,
	initSRateMask = 0x0030,
	initStereoMask = 0x00C0,
	initCompMask = 0xFF00
};

// Sound commands (SndCommand.cmd).
enum {
	nullCmd = 0,
	quietCmd = 3,
	flushCmd = 4,
	reInitCmd = 5,
	waitCmd = 10,
	pauseCmd = 11,
	resumeCmd = 12,
	callBackCmd = 13,
	syncCmd = 14,
	emptyCmd = 15,
	availableCmd = 24,
	versionCmd = 25,
	volumeCmd = 46,
	getVolumeCmd = 47,
	soundCmd = 80,
	bufferCmd = 81,
	rateMultiplierCmd = 86,
	getRateMultiplierCmd = 87
};

// Set in the command when param2 is an offset into a resource instead of a pointer.
enum {
	dataOffsetFlag = 0x8000
};

// Sound header encodings (SoundHeader.encode).
enum {
	stdSH = 0x00,
	extSH = 0xFF,
	cmpSH = 0xFE
};

// Common sample rates as unsigned fixed-point values.
enum {
	rate48khz = (int) 0xBB800000,
	rate44khz = (int) 0xAC440000,
	rate32khz = 0x7D000000,
	rate22050hz = 0x56220000,
	rate22khz = 0x56EE8BA3,
	rate16khz = 0x3E800000,
	rate11khz = 0x2B7745D1,
	rate11025hz = 0x2B110000,
	rate8khz = 0x1F400000
};

typedef struct SndCommand SndCommand;
typedef struct SndChannel SndChannel;
typedef SndChannel* SndChannelPtr;

typedef void (*SndCallBackProcPtr)(SndChannelPtr chan, SndCommand* cmd);
typedef SndCallBackProcPtr SndCallBackUPP;

struct SndCommand {
	unsigned short cmd;
	short param1;
	SInt32 param2;
};

struct SndChannel {
	SndChannelPtr nextChan;
	Ptr firstMod; // reserved for the Sound Manager
	SndCallBackUPP callBack;
	long userInfo;
	long wait; // internal use
	SndCommand cmdInProgress; // internal use
	short flags; // internal use
	short qLength;
	short qHead; // internal use
	short qTail; // internal use
	SndCommand queue[stdQLength];
};

struct SoundHeader {
	Ptr samplePtr; // NULL when the samples follow in sampleArea
	UInt32 length; // number of samples (bytes of 8-bit mono sound)
	UnsignedFixed sampleRate;
	UInt32 loopStart;
	UInt32 loopEnd;
	UInt8 encode;
	UInt8 baseFrequency;
	UInt8 sampleArea[1];
};
typedef struct SoundHeader SoundHeader;
typedef SoundHeader* SoundHeaderPtr;

struct ExtSoundHeader {
	Ptr samplePtr;
	UInt32 numChannels;
	UnsignedFixed sampleRate;
	UInt32 loopStart;
	UInt32 loopEnd;
	UInt8 encode;
	UInt8 baseFrequency;
	UInt32 numFrames;
	UInt8 AIFFSampleRate[10]; // 80-bit extended
	Ptr markerChunk;
	Ptr instrumentChunks;
	Ptr AESRecording;
	UInt16 sampleSize;
	UInt16 futureUse1;
	UInt32 futureUse2;
	UInt32 futureUse3;
	UInt32 futureUse4;
	UInt8 sampleArea[1];
};
typedef struct ExtSoundHeader ExtSoundHeader;
typedef ExtSoundHeader* ExtSoundHeaderPtr;

struct SCStatus {
	UnsignedFixed scStartTime;
	UnsignedFixed scEndTime;
	UnsignedFixed scCurrentTime;
	Boolean scChannelBusy;
	Boolean scChannelDisposed;
	Boolean scChannelPaused;
	Boolean scUnused;
	UInt32 scChannelAttributes;
	long scCPULoad;
};
typedef struct SCStatus SCStatus;

#pragma pack(pop)

SndCallBackUPP NewSndCallBackUPP(SndCallBackProcPtr userRoutine);
void DisposeSndCallBackUPP(SndCallBackUPP userUPP);
void InvokeSndCallBackUPP(SndChannelPtr chan, SndCommand* cmd, SndCallBackUPP userUPP);

OSErr SndNewChannel(SndChannelPtr* chan, short synth, long init, SndCallBackUPP userRoutine);
OSErr SndDisposeChannel(SndChannelPtr chan, Boolean quietNow);
OSErr SndDoCommand(SndChannelPtr chan, const SndCommand* cmd, Boolean noWait);
OSErr SndDoImmediate(SndChannelPtr chan, const SndCommand* cmd);
OSErr SndChannelStatus(SndChannelPtr chan, short theLength, SCStatus* theStatus);

#ifdef __cplusplus
}
#endif

#endif
