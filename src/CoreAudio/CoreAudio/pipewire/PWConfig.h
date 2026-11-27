/*
This file is part of Darling.

Copyright (C) 2026 Darling developers

Darling is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.
*/

#ifndef AUDIOHARDWARE_PW_CONFIG_H
#define AUDIOHARDWARE_PW_CONFIG_H

#include <CoreAudio/AudioHardware.h>
#include <pipe_client.h>
#include <vector>

// The stream objects of this backend live outside the range used for audio
// objects. Their low 16 bits are the ID of the device that owns the stream.
static const AudioObjectID kPWStreamObjectBase = 0x44570000u;

// The PipeWire HAL exposes the three roles of CoreAudio's default devices.
// They are object IDs, not arbitrary indices: they are what gets returned for
// the default device properties and what appears in the device list.
static const AudioObjectID kPWDefaultOutputObject = kAudioObjectSystemObject + 1;
static const AudioObjectID kPWDefaultInputObject = kAudioObjectSystemObject + 2;
static const AudioObjectID kPWSystemOutputObject = kAudioObjectSystemObject + 3;

// True for a stream object of this backend. When deviceID is not null, the
// owning device's ID is stored there.
bool PWIsStreamObject(AudioObjectID objectID, AudioObjectID* deviceID);

// The UID strings of the three devices. NULL for an unknown object ID.
const char* PWUIDForDevice(AudioObjectID deviceID);
// The object ID of a device with this UID, or kAudioObjectUnknown.
AudioObjectID PWDeviceForUID(const char* uid);

// Fills a PipeWire audio stream configuration from the HAL's format. The
// shared client always negotiates interleaved 32-bit float; rate and channel
// count come from the ASBD. Returns false for an unusable format.
bool PWStreamConfigForASBD(const AudioStreamBasicDescription& asbd, bool capture,
	const char* role, darling_pw_stream_config* out);

// The rates this backend can ask PipeWire to negotiate, plus the current one.
std::vector<Float64> PWAvailableSampleRates(Float64 currentRate);

// Number of whole frames in a byte count, rounded down.
uint32_t PWFramesForBytes(size_t bytes, uint32_t bytesPerFrame);

#endif /* AUDIOHARDWARE_PW_CONFIG_H */
