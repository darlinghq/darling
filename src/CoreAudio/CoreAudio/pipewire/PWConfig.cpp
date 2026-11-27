/*
This file is part of Darling.

Copyright (C) 2026 Darling developers

Darling is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.
*/

#include "PWConfig.h"

#include <algorithm>
#include <cmath>
#include <cstring>

bool PWIsStreamObject(AudioObjectID objectID, AudioObjectID* deviceID)
{
	if ((objectID & 0xffff0000u) != kPWStreamObjectBase)
		return false;
	if (deviceID)
		*deviceID = objectID & 0xffffu;
	return true;
}

const char* PWUIDForDevice(AudioObjectID deviceID)
{
	switch (deviceID)
	{
		case kPWDefaultOutputObject: return "PipeWire:Output";
		case kPWDefaultInputObject: return "PipeWire:Input";
		case kPWSystemOutputObject: return "PipeWire:SystemOutput";
		default: return nullptr;
	}
}

AudioObjectID PWDeviceForUID(const char* uid)
{
	if (!uid)
		return kAudioObjectUnknown;
	if (!std::strcmp(uid, "PipeWire:Output"))
		return kPWDefaultOutputObject;
	if (!std::strcmp(uid, "PipeWire:Input"))
		return kPWDefaultInputObject;
	if (!std::strcmp(uid, "PipeWire:SystemOutput"))
		return kPWSystemOutputObject;
	return kAudioObjectUnknown;
}

bool PWStreamConfigForASBD(const AudioStreamBasicDescription& asbd, bool capture,
	const char* role, darling_pw_stream_config* out)
{
	if (!out || !asbd.mChannelsPerFrame || asbd.mSampleRate <= 0
		|| asbd.mSampleRate > 0xffffffffULL)
		return false;

	*out = darling_pw_stream_config{};
	out->media = DARLING_PW_MEDIA_AUDIO;
	out->capture = capture ? 1 : 0;
	out->name = "CoreAudio";
	out->role = role ? role : (capture ? "capture" : "playback");
	out->rate = static_cast<uint32_t>(asbd.mSampleRate);
	out->channels = asbd.mChannelsPerFrame;
	return true;
}

std::vector<Float64> PWAvailableSampleRates(Float64 currentRate)
{
	std::vector<Float64> rates = { 44100.0, 48000.0, 96000.0 };
	if (currentRate > 0)
		rates.push_back(currentRate);
	std::sort(rates.begin(), rates.end());
	rates.erase(std::unique(rates.begin(), rates.end()), rates.end());
	return rates;
}

uint32_t PWFramesForBytes(size_t bytes, uint32_t bytesPerFrame)
{
	if (!bytesPerFrame)
		return 0;
	const size_t frames = bytes / bytesPerFrame;
	return frames > 0xffffffffULL ? 0xffffffffu : static_cast<uint32_t>(frames);
}
