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

#include "AudioFileAVFormat.h"
#include "AudioFileComponentBase.h"
#include "AudioFileFormatGeneric.h"

using Format = AudioFileFormatGeneric::Format;
using Description = AudioFileFormatGeneric::Description;

static const UInt32 PACKED_SIGNED = kAudioFormatFlagIsSignedInteger | kAudioFormatFlagIsPacked;
static const UInt32 PACKED_FLOAT = kAudioFormatFlagIsFloat | kAudioFormatFlagIsPacked;
static const UInt32 BIG = kAudioFormatFlagIsBigEndian;

static const std::vector<Format> BIG_ENDIAN_PCM = {
	Format(kAudioFormatLinearPCM, PACKED_SIGNED | BIG, 8),
	Format(kAudioFormatLinearPCM, PACKED_SIGNED | BIG, 16),
	Format(kAudioFormatLinearPCM, PACKED_SIGNED | BIG, 24),
	Format(kAudioFormatLinearPCM, PACKED_SIGNED | BIG, 32),
	Format(kAudioFormatLinearPCM, PACKED_FLOAT | BIG, 32),
	Format(kAudioFormatLinearPCM, PACKED_FLOAT | BIG, 64),
};

static const Description WAVE_DESCRIPTION = {
	kAudioFileWAVEType, "WAVE", "wav", "wav",
	{ "wav", "wave" },
	{ "com.microsoft.waveform-audio", "public.audio", "public.data" },
	{ "audio/wav", "audio/x-wav", "audio/vnd.wave" },
	{
		Format(kAudioFormatLinearPCM, kAudioFormatFlagIsPacked, 8),
		Format(kAudioFormatLinearPCM, PACKED_SIGNED, 16),
		Format(kAudioFormatLinearPCM, PACKED_SIGNED, 24),
		Format(kAudioFormatLinearPCM, PACKED_SIGNED, 32),
		Format(kAudioFormatLinearPCM, PACKED_FLOAT, 32),
		Format(kAudioFormatLinearPCM, PACKED_FLOAT, 64),
		Format(kAudioFormatULaw),
		Format(kAudioFormatALaw),
	},
};

static const Description AIFF_DESCRIPTION = {
	kAudioFileAIFFType, "AIFF", "aiff", "aiff",
	{ "aif", "aiff" },
	{ "public.aiff-audio", "public.audio", "public.data" },
	{ "audio/aiff", "audio/x-aiff" },
	BIG_ENDIAN_PCM,
};

static const Description AIFC_DESCRIPTION = {
	kAudioFileAIFCType, "AIFC", "aiff", "aiff",
	{ "aifc", "cdda" },
	{ "public.aifc-audio", "public.audio", "public.data" },
	{ "audio/aiff", "audio/x-aiff" },
	BIG_ENDIAN_PCM,
};

static const Description CAF_DESCRIPTION = {
	kAudioFileCAFType, "CAF", "caf", "caf",
	{ "caf" },
	{ "com.apple.coreaudio-format", "public.audio", "public.data" },
	{ "audio/x-caf" },
	{
		Format(kAudioFormatLinearPCM, PACKED_SIGNED, 16),
		Format(kAudioFormatLinearPCM, PACKED_SIGNED | BIG, 16),
		Format(kAudioFormatLinearPCM, PACKED_SIGNED, 24),
		Format(kAudioFormatLinearPCM, PACKED_SIGNED | BIG, 24),
		Format(kAudioFormatLinearPCM, PACKED_SIGNED, 32),
		Format(kAudioFormatLinearPCM, PACKED_SIGNED | BIG, 32),
		Format(kAudioFormatLinearPCM, PACKED_FLOAT, 32),
		Format(kAudioFormatLinearPCM, PACKED_FLOAT | BIG, 32),
		Format(kAudioFormatLinearPCM, PACKED_FLOAT, 64),
		Format(kAudioFormatLinearPCM, PACKED_FLOAT | BIG, 64),
		Format(kAudioFormatULaw),
		Format(kAudioFormatALaw),
	},
};

static const Description NEXT_DESCRIPTION = {
	kAudioFileNextType, "NeXT/Sun", "au", "au",
	{ "au", "snd" },
	{ "public.ulaw-audio", "public.audio", "public.data" },
	{ "audio/basic" },
	{
		Format(kAudioFormatLinearPCM, PACKED_SIGNED | BIG, 8),
		Format(kAudioFormatLinearPCM, PACKED_SIGNED | BIG, 16),
		Format(kAudioFormatLinearPCM, PACKED_SIGNED | BIG, 24),
		Format(kAudioFormatLinearPCM, PACKED_SIGNED | BIG, 32),
		Format(kAudioFormatLinearPCM, PACKED_FLOAT | BIG, 32),
		Format(kAudioFormatLinearPCM, PACKED_FLOAT | BIG, 64),
		Format(kAudioFormatULaw),
		Format(kAudioFormatALaw),
	},
};

static const Description MP3_DESCRIPTION = {
	kAudioFileMP3Type, "MPEG Layer 3", "mp3", nullptr,
	{ "mp3" },
	{ "public.mp3", "public.audio", "public.data" },
	{ "audio/mpeg", "audio/mp3" },
	{ Format(kAudioFormatMPEGLayer3) },
};

static const Description ADTS_DESCRIPTION = {
	kAudioFileAAC_ADTSType, "AAC ADTS", "aac", nullptr,
	{ "aac", "adts" },
	{ "public.aac-audio", "public.audio", "public.data" },
	{ "audio/aac", "audio/x-aac" },
	{ Format(kAudioFormatMPEG4AAC) },
};

static const Description M4A_DESCRIPTION = {
	kAudioFileM4AType, "Apple MPEG-4 Audio", "mov", nullptr,
	{ "m4a", "m4r" },
	{ "com.apple.m4a-audio", "public.audio", "public.data" },
	{ "audio/mp4", "audio/x-m4a" },
	{ Format(kAudioFormatMPEG4AAC), Format(kAudioFormatAppleLossless) },
};

static const Description MP4_DESCRIPTION = {
	kAudioFileMPEG4Type, "MPEG-4 Audio", "mov", nullptr,
	{ "mp4" },
	{ "public.mpeg-4-audio", "public.audio", "public.data" },
	{ "audio/mp4" },
	{ Format(kAudioFormatMPEG4AAC), Format(kAudioFormatAppleLossless) },
};

static const Description FLAC_DESCRIPTION = {
	kAudioFileFLACType, "FLAC", "flac", nullptr,
	{ "flac" },
	{ "org.xiph.flac", "public.audio", "public.data" },
	{ "audio/flac", "audio/x-flac" },
	{ Format(kAudioFormatFLAC) },
};

static const Description AC3_DESCRIPTION = {
	kAudioFileAC3Type, "AC-3", "ac3", nullptr,
	{ "ac3" },
	{ "public.ac3-audio", "public.audio", "public.data" },
	{ "audio/ac3" },
	{ Format(kAudioFormatAC3) },
};

template <const Description& D>
class AVFormatComponent : public AudioFileObjectComponentBase
{
public:
	AVFormatComponent(AudioComponentInstance inInstance)
	: AudioFileObjectComponentBase(inInstance)
	{
		mAudioFileObject = new AudioFileAVFormat(D.fileType, D.demuxer, D.muxer);
	}

	AudioFileFormat* GetAudioFormat() const override
	{
		static AudioFileFormatGeneric inst(D);
		return &inst;
	}
};

#define AVFORMAT_COMPONENT(Name, D) \
	class Name : public AVFormatComponent<D> \
	{ \
	public: \
		Name(AudioComponentInstance inInstance) : AVFormatComponent(inInstance) {} \
	}; \
	AUDIOCOMPONENT_ENTRY(AudioFileComponentFactory, Name)

#pragma GCC visibility push(default)
AVFORMAT_COMPONENT(WAVEComponent, WAVE_DESCRIPTION);
AVFORMAT_COMPONENT(AIFFComponent, AIFF_DESCRIPTION);
AVFORMAT_COMPONENT(AIFCComponent, AIFC_DESCRIPTION);
AVFORMAT_COMPONENT(CAFComponent, CAF_DESCRIPTION);
AVFORMAT_COMPONENT(NeXTComponent, NEXT_DESCRIPTION);
AVFORMAT_COMPONENT(MP3Component, MP3_DESCRIPTION);
AVFORMAT_COMPONENT(ADTSComponent, ADTS_DESCRIPTION);
AVFORMAT_COMPONENT(M4AComponent, M4A_DESCRIPTION);
AVFORMAT_COMPONENT(MP4Component, MP4_DESCRIPTION);
AVFORMAT_COMPONENT(FLACComponent, FLAC_DESCRIPTION);
AVFORMAT_COMPONENT(AC3Component, AC3_DESCRIPTION);
#pragma GCC visibility pop
