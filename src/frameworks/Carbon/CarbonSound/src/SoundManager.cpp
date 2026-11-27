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

// Sound Manager channels (sampled sound only), played through an AudioUnit on the default output
// device. Each channel owns an output unit and a play list: a command that makes sound (a sound
// header, a wait) is turned into samples when it is queued, and the other commands that take their
// place in the sequence (callBackCmd, volumeCmd, flushCmd) become entries between them, so that the
// render callback of the output unit moves from one entry to the next without a gap and a callback
// is made when the sound before it has been played.

#include <CarbonSound/Sound.h>
#include <AudioToolbox/AudioToolbox.h>
#include <CoreAudio/CoreAudio.h>

#include <algorithm>
#include <atomic>
#include <condition_variable>
#include <deque>
#include <map>
#include <memory>
#include <mutex>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <thread>
#include <vector>

namespace
{

// QUIRK: SndNewChannel fails with -2004 when the system has no default output device, before it
// looks at its other arguments (a bad synthesizer number fails the same way). The error has no
// name in the public headers. Not documented. Evidence: black-box observation on macOS 10.14.4, i386
// (tier 2), on a machine without an audio output device (the call does not exist in 64-bit
// processes), vd-upstream/mac-probes/v666-snd1.c and v666-snd2.c.
// Test: tests/src/sound_manager_channels.c
const OSErr kNoOutputDeviceErr = -2004;

enum class Kind
{
	Audio,    // samples to play
	Callback, // a callBackCmd waiting for the sound before it
	Volume,   // a volumeCmd waiting for the sound before it
	Flush     // a flushCmd waiting for the sound before it
};

struct Item
{
	Kind kind = Kind::Audio;
	std::vector<float> frames; // interleaved left and right samples
	size_t position = 0;       // frames already played
	SndCommand command = {};
};

struct Channel
{
	SndChannelPtr chan = nullptr;
	bool ownsChan = false;
	SndCallBackUPP callback = nullptr;

	std::mutex mutex; // protects everything below the next line
	std::condition_variable changed;
	std::deque<Item> items;
	std::deque<SndCommand> due; // callbacks to make
	bool paused = false;
	bool disposing = false;
	float volumeLeft = 1, volumeRight = 1;
	Float64 outputRate = 44100;
	std::thread callbackThread;

	std::mutex unitMutex; // protects the unit and its state; taken before `mutex`
	AudioComponentInstance unit = nullptr;
	Float64 unitRate = 0;
	std::atomic<bool> started{false};
};

std::mutex g_channelsLock;
std::map<SndChannelPtr, std::shared_ptr<Channel>> g_channels;

std::shared_ptr<Channel> findChannel(SndChannelPtr chan)
{
	std::lock_guard<std::mutex> lock(g_channelsLock);
	auto it = g_channels.find(chan);
	return it == g_channels.end() ? nullptr : it->second;
}

OSStatus renderCallback(void* refCon, AudioUnitRenderActionFlags* flags, const AudioTimeStamp*, UInt32, UInt32 frames, AudioBufferList* data)
{
	Channel* ch = (Channel*) refCon;
	float* out = (float*) data->mBuffers[0].mData;
	memset(out, 0, (size_t) frames * 2 * sizeof(float));

	std::lock_guard<std::mutex> lock(ch->mutex);
	UInt32 produced = 0;
	bool consumed = false;

	while (!ch->paused && produced < frames && !ch->items.empty())
	{
		Item& item = ch->items.front();
		switch (item.kind)
		{
			case Kind::Audio:
			{
				const size_t total = item.frames.size() / 2;
				const size_t n = std::min<size_t>(frames - produced, total - item.position);
				for (size_t i = 0; i < n; i++)
				{
					out[2 * (produced + i)] = item.frames[2 * (item.position + i)] * ch->volumeLeft;
					out[2 * (produced + i) + 1] = item.frames[2 * (item.position + i) + 1] * ch->volumeRight;
				}
				produced += n;
				item.position += n;
				if (item.position >= total)
				{
					ch->items.pop_front();
					consumed = true;
				}
				break;
			}
			case Kind::Callback:
				ch->due.push_back(item.command);
				ch->items.pop_front();
				consumed = true;
				break;
			case Kind::Volume:
				ch->volumeLeft = std::min(1.0f, (item.command.param2 & 0xFFFF) / 256.0f);
				ch->volumeRight = std::min(1.0f, ((item.command.param2 >> 16) & 0xFFFF) / 256.0f);
				ch->items.pop_front();
				consumed = true;
				break;
			case Kind::Flush:
				ch->items.clear();
				consumed = true;
				break;
		}
	}

	if (produced == 0 && flags)
		*flags |= kAudioUnitRenderAction_OutputIsSilence;
	if (consumed)
		ch->changed.notify_all();
	return noErr;
}

// Creates the output unit, or changes its sample rate. Called with unitMutex held.
bool prepareUnit(Channel* ch, Float64 rate)
{
	if (ch->unit && ch->unitRate == rate)
		return true;

	if (ch->unit)
	{
		AudioOutputUnitStop(ch->unit);
		AudioUnitUninitialize(ch->unit);
		AudioComponentInstanceDispose(ch->unit);
		ch->unit = nullptr;
		ch->started = false;
	}

	AudioComponentDescription desc = {};
	desc.componentType = kAudioUnitType_Output;
	desc.componentSubType = kAudioUnitSubType_DefaultOutput;
	desc.componentManufacturer = kAudioUnitManufacturer_Apple;
	AudioComponent component = AudioComponentFindNext(nullptr, &desc);
	if (!component || AudioComponentInstanceNew(component, &ch->unit) != noErr)
	{
		ch->unit = nullptr;
		return false;
	}

	AURenderCallbackStruct render = { renderCallback, ch };
	AudioStreamBasicDescription format = {};
	format.mSampleRate = rate;
	format.mFormatID = kAudioFormatLinearPCM;
	format.mFormatFlags = kAudioFormatFlagsNativeFloatPacked;
	format.mBytesPerPacket = format.mBytesPerFrame = 2 * sizeof(float);
	format.mFramesPerPacket = 1;
	format.mChannelsPerFrame = 2;
	format.mBitsPerChannel = 32;

	if (AudioUnitSetProperty(ch->unit, kAudioUnitProperty_SetRenderCallback, kAudioUnitScope_Input, 0, &render, sizeof(render)) != noErr
		|| AudioUnitSetProperty(ch->unit, kAudioUnitProperty_StreamFormat, kAudioUnitScope_Input, 0, &format, sizeof(format)) != noErr
		|| AudioUnitInitialize(ch->unit) != noErr)
	{
		AudioComponentInstanceDispose(ch->unit);
		ch->unit = nullptr;
		return false;
	}

	ch->unitRate = rate;
	return true;
}

bool startOutput(Channel* ch)
{
	std::lock_guard<std::mutex> unitLock(ch->unitMutex);
	if (ch->started)
		return true;

	Float64 rate;
	{
		std::lock_guard<std::mutex> lock(ch->mutex);
		rate = ch->outputRate;
	}
	if (!prepareUnit(ch, rate) || AudioOutputUnitStart(ch->unit) != noErr)
		return false;

	ch->started = true;
	return true;
}

// Stops the output unit when nothing is left to play.
void stopOutputIfIdle(Channel* ch)
{
	std::lock_guard<std::mutex> unitLock(ch->unitMutex);
	if (!ch->started)
		return;

	{
		std::lock_guard<std::mutex> lock(ch->mutex);
		if (!ch->items.empty())
			return;
	}
	AudioOutputUnitStop(ch->unit);
	ch->started = false;
}

void callbackLoop(std::shared_ptr<Channel> ch)
{
	std::unique_lock<std::mutex> lock(ch->mutex);
	for (;;)
	{
		ch->changed.wait(lock, [&] { return ch->disposing || !ch->due.empty() || (ch->items.empty() && ch->started); });

		if (!ch->due.empty())
		{
			SndCommand cmd = ch->due.front();
			ch->due.pop_front();
			ch->changed.notify_all();
			lock.unlock();
			if (ch->callback)
				ch->callback(ch->chan, &cmd);
			lock.lock();
			continue;
		}
		if (ch->disposing)
			break;

		lock.unlock();
		stopOutputIfIdle(ch.get());
		lock.lock();
	}
}

// ---- Sound headers ---------------------------------------------------------------------------

// Converts the sound header at `param2` to interleaved stereo samples at the sound's own rate.
// Standard headers hold unsigned 8-bit mono; extended headers 8-bit unsigned or 16-bit signed
// big-endian samples in one or two channels. Compressed headers, other sample sizes and sounds
// found through an offset into a resource are not supported.
OSErr parseSound(const SndCommand& cmd, Float64* sourceRate, std::vector<float>* stereoOut)
{
#if __LP64__
	// The command's 32-bit parameter cannot hold the address of the header.
	return unimpErr;
#else
	if (cmd.cmd & dataOffsetFlag)
		return badFormat;
	if (cmd.param2 == 0)
		return paramErr;

	const SoundHeader* header = (const SoundHeader*) (intptr_t) cmd.param2;
	uint64_t frames;
	unsigned channels, bits;
	const uint8_t* data;

	if (header->encode == stdSH)
	{
		frames = header->length;
		channels = 1;
		bits = 8;
		data = header->samplePtr ? (const uint8_t*) header->samplePtr : header->sampleArea;
	}
	else if (header->encode == extSH)
	{
		const ExtSoundHeader* ext = (const ExtSoundHeader*) header;
		frames = ext->numFrames;
		channels = ext->numChannels;
		bits = ext->sampleSize;
		data = ext->samplePtr ? (const uint8_t*) ext->samplePtr : ext->sampleArea;
	}
	else
		return badFormat;

	if ((channels != 1 && channels != 2) || (bits != 8 && bits != 16) || header->sampleRate == 0)
		return badFormat;

	*sourceRate = header->sampleRate / 65536.0;
	std::vector<float>& samples = *stereoOut;
	samples.resize((size_t) frames * 2);

	for (uint64_t i = 0; i < frames; i++)
	{
		float value[2];
		for (unsigned c = 0; c < channels; c++)
		{
			if (bits == 16)
			{
				const uint8_t* p = data + (i * channels + c) * 2;
				value[c] = (int16_t) ((p[0] << 8) | p[1]) / 32768.0f;
			}
			else
				value[c] = (data[i * channels + c] - 128) / 128.0f;
		}
		samples[2 * i] = value[0];
		samples[2 * i + 1] = channels == 2 ? value[1] : value[0];
	}
	return noErr;
#endif
}

std::vector<float> resample(const std::vector<float>& in, Float64 from, Float64 to)
{
	if (from == to || in.empty())
		return in;

	const size_t inFrames = in.size() / 2;
	const size_t outFrames = std::max<size_t>(1, (size_t) ((double) inFrames * to / from + 0.5));
	std::vector<float> out(outFrames * 2);
	for (size_t i = 0; i < outFrames; i++)
	{
		const double at = (double) i * from / to;
		const size_t a = std::min(inFrames - 1, (size_t) at);
		const size_t b = std::min(inFrames - 1, a + 1);
		const float t = (float) (at - (double) a);
		for (int c = 0; c < 2; c++)
			out[2 * i + c] = in[2 * a + c] * (1 - t) + in[2 * b + c] * t;
	}
	return out;
}

// ---- Commands --------------------------------------------------------------------------------

OSErr enqueue(Channel* ch, Item item, bool noWait, bool atFront)
{
	bool needStart = false;
	{
		std::unique_lock<std::mutex> lock(ch->mutex);

		if (!atFront)
		{
			while (!ch->disposing && ch->items.size() >= stdQLength)
			{
				if (noWait)
					return queueFull;
				ch->changed.wait(lock);
			}
		}
		if (ch->disposing)
			return badChannel;

		if (item.kind == Kind::Audio)
			needStart = true;
		else if (ch->items.empty() && !ch->paused)
		{
			// Nothing is playing for the command to wait for: act on it now.
			if (item.kind == Kind::Callback)
				ch->due.push_back(item.command);
			else if (item.kind == Kind::Volume)
			{
				ch->volumeLeft = std::min(1.0f, (item.command.param2 & 0xFFFF) / 256.0f);
				ch->volumeRight = std::min(1.0f, ((item.command.param2 >> 16) & 0xFFFF) / 256.0f);
			}
			ch->changed.notify_all();
			return noErr;
		}

		if (atFront)
		{
			// An immediate sound replaces the one that is playing.
			if (!ch->items.empty() && ch->items.front().kind == Kind::Audio)
				ch->items.pop_front();
			ch->items.push_front(std::move(item));
		}
		else
			ch->items.push_back(std::move(item));
	}

	if (needStart && !startOutput(ch))
	{
		std::lock_guard<std::mutex> lock(ch->mutex);
		ch->items.clear();
		ch->changed.notify_all();
		return siDeviceBusyErr;
	}
	return noErr;
}

OSErr submit(Channel* ch, const SndCommand& cmd, bool noWait, bool immediate)
{
	const unsigned base = cmd.cmd & ~(unsigned) dataOffsetFlag;

	switch (base)
	{
		case nullCmd:
		case syncCmd:
		case emptyCmd:
		case reInitCmd:
			return noErr;

		case quietCmd:
		{
			// A quiet command that waits its turn finds nothing playing.
			if (!immediate)
				return noErr;
			std::lock_guard<std::mutex> lock(ch->mutex);
			if (!ch->items.empty() && ch->items.front().kind == Kind::Audio)
				ch->items.pop_front();
			ch->changed.notify_all();
			return noErr;
		}

		case flushCmd:
		{
			if (immediate)
			{
				std::lock_guard<std::mutex> lock(ch->mutex);
				if (!ch->items.empty() && ch->items.front().kind == Kind::Audio)
					ch->items.erase(ch->items.begin() + 1, ch->items.end());
				else
					ch->items.clear();
				ch->changed.notify_all();
				return noErr;
			}
			Item item;
			item.kind = Kind::Flush;
			return enqueue(ch, std::move(item), noWait, false);
		}

		case pauseCmd:
		case resumeCmd:
		{
			std::lock_guard<std::mutex> lock(ch->mutex);
			ch->paused = base == pauseCmd;
			ch->changed.notify_all();
			return noErr;
		}

		case callBackCmd:
		{
			SndCommand queued = cmd;
			queued.cmd = callBackCmd;
			if (immediate)
			{
				std::lock_guard<std::mutex> lock(ch->mutex);
				ch->due.push_back(queued);
				ch->changed.notify_all();
				return noErr;
			}
			Item item;
			item.kind = Kind::Callback;
			item.command = queued;
			return enqueue(ch, std::move(item), noWait, false);
		}

		case volumeCmd:
		{
			if (immediate)
			{
				std::lock_guard<std::mutex> lock(ch->mutex);
				ch->volumeLeft = std::min(1.0f, (cmd.param2 & 0xFFFF) / 256.0f);
				ch->volumeRight = std::min(1.0f, ((cmd.param2 >> 16) & 0xFFFF) / 256.0f);
				return noErr;
			}
			Item item;
			item.kind = Kind::Volume;
			item.command = cmd;
			return enqueue(ch, std::move(item), noWait, false);
		}

		case waitCmd:
		{
			if (immediate)
				return noErr;
			Item item;
			{
				std::lock_guard<std::mutex> lock(ch->mutex);
				const Float64 rate = (ch->items.empty() && !ch->started) ? 44100 : ch->outputRate;
				ch->outputRate = rate;
				// param1 is the duration in half-milliseconds.
				item.frames.assign((size_t) ((double) std::max<short>(0, cmd.param1) * rate / 2000.0) * 2, 0.0f);
			}
			return enqueue(ch, std::move(item), noWait, false);
		}

		case soundCmd:
		case bufferCmd:
		{
			Float64 sourceRate = 0;
			std::vector<float> samples;
			OSErr err = parseSound(cmd, &sourceRate, &samples);
			if (err != noErr)
				return err;

			Item item;
			{
				// While nothing is playing, the rate of the new sound becomes the rate of the output.
				std::lock_guard<std::mutex> lock(ch->mutex);
				if (ch->items.empty() && !ch->started)
					ch->outputRate = sourceRate;
				item.frames = resample(samples, sourceRate, ch->outputRate);
			}
			return enqueue(ch, std::move(item), noWait, immediate);
		}

		default:
			return unimpErr;
	}
}

} // namespace

// ---- Public interface ------------------------------------------------------------------------

SndCallBackUPP NewSndCallBackUPP(SndCallBackProcPtr userRoutine)
{
	return userRoutine;
}

void DisposeSndCallBackUPP(SndCallBackUPP userUPP)
{
	// A UPP is the routine itself; there is nothing to release.
}

void InvokeSndCallBackUPP(SndChannelPtr chan, SndCommand* cmd, SndCallBackUPP userUPP)
{
	userUPP(chan, cmd);
}

OSErr SndNewChannel(SndChannelPtr* chan, short synth, long init, SndCallBackUPP userRoutine)
{
	AudioObjectPropertyAddress address = { kAudioHardwarePropertyDefaultOutputDevice, kAudioObjectPropertyScopeGlobal, kAudioObjectPropertyElementMaster };
	AudioObjectID device = kAudioObjectUnknown;
	UInt32 size = sizeof(device);
	if (AudioObjectGetPropertyData(kAudioObjectSystemObject, &address, 0, nullptr, &size, &device) != noErr || device == kAudioObjectUnknown)
		return kNoOutputDeviceErr;

	if (!chan)
		return paramErr;
	// Only sampled sound is played; the square-wave and wave-table synthesizers are not available.
	if (synth != sampledSynth)
		return notEnoughHardwareErr;

	std::shared_ptr<Channel> ch(new Channel);
	ch->callback = userRoutine;
	ch->ownsChan = *chan == nullptr;
	ch->chan = ch->ownsChan ? (SndChannelPtr) calloc(1, sizeof(SndChannel)) : *chan;
	if (!ch->chan)
		return notEnoughHardwareErr;

	ch->chan->callBack = userRoutine;
	ch->chan->qLength = stdQLength;

	{
		std::lock_guard<std::mutex> unitLock(ch->unitMutex);
		if (!prepareUnit(ch.get(), ch->outputRate))
		{
			if (ch->ownsChan)
				free(ch->chan);
			return kNoOutputDeviceErr;
		}
	}

	ch->callbackThread = std::thread(callbackLoop, ch);
	{
		std::lock_guard<std::mutex> lock(g_channelsLock);
		g_channels[ch->chan] = ch;
	}
	*chan = ch->chan;
	return noErr;
}

OSErr SndDisposeChannel(SndChannelPtr chan, Boolean quietNow)
{
	std::shared_ptr<Channel> ch = findChannel(chan);
	if (!ch)
		return badChannel;

	const bool fromCallback = std::this_thread::get_id() == ch->callbackThread.get_id();
	{
		std::unique_lock<std::mutex> lock(ch->mutex);
		if (quietNow || fromCallback)
		{
			ch->items.clear();
			ch->due.clear();
		}
		else
		{
			// Let what is queued play out, callbacks included.
			ch->paused = false;
			ch->changed.wait(lock, [&] { return ch->items.empty() && ch->due.empty(); });
		}
		ch->disposing = true;
		ch->changed.notify_all();
	}

	{
		std::lock_guard<std::mutex> lock(g_channelsLock);
		g_channels.erase(chan);
	}

	if (fromCallback)
		ch->callbackThread.detach();
	else
		ch->callbackThread.join();

	{
		std::lock_guard<std::mutex> unitLock(ch->unitMutex);
		if (ch->unit)
		{
			AudioOutputUnitStop(ch->unit);
			AudioUnitUninitialize(ch->unit);
			AudioComponentInstanceDispose(ch->unit);
			ch->unit = nullptr;
			ch->started = false;
		}
	}

	if (ch->ownsChan)
		free(ch->chan);
	return noErr;
}

OSErr SndDoCommand(SndChannelPtr chan, const SndCommand* cmd, Boolean noWait)
{
	std::shared_ptr<Channel> ch = findChannel(chan);
	if (!ch)
		return badChannel;
	if (!cmd)
		return paramErr;
	return submit(ch.get(), *cmd, noWait, false);
}

OSErr SndDoImmediate(SndChannelPtr chan, const SndCommand* cmd)
{
	std::shared_ptr<Channel> ch = findChannel(chan);
	if (!ch)
		return badChannel;
	if (!cmd)
		return paramErr;
	return submit(ch.get(), *cmd, true, true);
}

OSErr SndChannelStatus(SndChannelPtr chan, short theLength, SCStatus* theStatus)
{
	std::shared_ptr<Channel> ch = findChannel(chan);
	if (!ch)
		return badChannel;
	if (!theStatus || theLength < 0)
		return paramErr;

	SCStatus status;
	memset(&status, 0, sizeof(status));
	{
		std::lock_guard<std::mutex> lock(ch->mutex);
		status.scChannelBusy = !ch->items.empty() || !ch->due.empty();
		status.scChannelPaused = ch->paused;
	}
	memcpy(theStatus, &status, std::min<size_t>(theLength, sizeof(status)));
	return noErr;
}

#if !__LP64__

// The alert sound: Darling has no sound of the user's choice to play, so the alert is a short falling two-note beep,
// 8-bit mono at 22050 Hz, played on a channel of its own that a thread of the call disposes of when the sound is
// over. A beep that is asked for while one is still sounding is not played again (alerts do not pile up). A system
// with no output device has no beep, as on macOS. SysBeep's duration is not used.
void SysBeep(short duration)
{
	static std::atomic<bool> beeping(false);

	if (beeping.exchange(true))
		return;
	std::thread([]
	{
		const unsigned rate = 22050, frames = rate * 3 / 10;
		const size_t size = sizeof(SoundHeader) + frames;
		SoundHeader* header = static_cast<SoundHeader*>(calloc(1, size));
		SndChannelPtr channel = nullptr;

		if (!header)
		{
			beeping = false;
			return;
		}
		header->samplePtr = nullptr;
		header->length = frames;
		header->sampleRate = rate << 16;
		header->encode = stdSH;
		header->baseFrequency = 60;
		for (unsigned i = 0; i < frames; i++)
		{
			// two notes of 150 ms, 880 Hz and 660 Hz, each fading out
			double t = (i % (frames / 2)) / double(rate), frequency = i < frames / 2 ? 880.0 : 660.0;
			double level = 0.4 * (1.0 - double(i % (frames / 2)) / (frames / 2));
			header->sampleArea[i] = static_cast<UInt8>(128 + 127 * level * sin(2 * M_PI * frequency * t));
		}
		if (SndNewChannel(&channel, sampledSynth, initMono, nullptr) == noErr)
		{
			SndCommand command = { bufferCmd, 0, static_cast<SInt32>(reinterpret_cast<intptr_t>(header)) };

			if (SndDoCommand(channel, &command, false) == noErr)
				SndDisposeChannel(channel, false); // false: after the queued sound
			else
				SndDisposeChannel(channel, true);
		}
		free(header);
		beeping = false;
	}).detach();
}

#endif
