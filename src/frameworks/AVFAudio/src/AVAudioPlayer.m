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


#import <AVFAudio/AVAudioPlayer.h>
#import <AVFAudio/AVAudioSettings.h>
#import <AudioToolbox/AudioToolbox.h>
#import <dispatch/dispatch.h>
#import <mach/mach_time.h>
#import <pthread.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

// AVAudioPlayer sits on AudioFile/ExtAudioFile (reading and decoding) and AudioQueue (output).
//
// A "session" is the set of things that exist while a sound is prepared or playing: the output queue,
// its buffers and a worker thread that keeps the buffers filled. A session starts at a position of the
// file and goes away on stop, at the end of the sound or on destruction. Pausing keeps it.
//
// What has been handed to the queue is remembered as segments (one per buffer): where the buffer starts on
// the queue's timeline and at which position of the file. The position that is heard (currentTime) is
// found by looking up the queue's current time among the segments.

enum {
	BufferCount = 6,
	SegmentCapacity = BufferCount + 4,
	SourceChunk = 4096,
	WorkerTickMs = 20
};

// QUIRK: the level meters. The documentation says only that they report average and peak power in decibels. Observed:
// the values describe the sound as it is read from the file (before volume and pan); the average follows the loudest
// recent samples (it equals the peak of a steady tone) and falls at a constant rate in dB; the peak is held for a
// while and then falls more slowly; a silent channel of a playing sound is at -120 dB, and everything is at -160 dB
// while nothing plays, while the meters are off, and for channels the sound does not have.
// Evidence: black-box observation of AVAudioPlayer on macOS (tier 2); the constants below reproduce the observed
// readings of a full-scale sine (0 dB peak), a sine at half the scale (-6.02 dB) and a tone that ends (average
// falling about 50 dB per second, the peak holding for about 0.6 s and then falling about 25 dB per second).
// Test: tests/avaudioplayer.m, "== meters".
static const double MeterFloorWhilePlaying = -120.0;
static const double MeterFloorIdle = -160.0;
static const double AverageFallDbPerSecond = 50.0;
static const double PeakHoldSeconds = 0.6;
static const double PeakFallDbPerSecond = 25.0;

typedef struct {
	int64_t serial;
	int64_t queueStart;   // on the queue's timeline, in frames
	double sourceStart;   // position in the file, in frames
	double sourceStep;    // file frames per output frame: the playback rate, 0 for silence
	uint32_t frames;
	float peak[2];        // of the samples in this buffer, before volume and pan
} Segment;

@interface AVAudioPlayer ()
- (void)playbackEndedWithGeneration:(uint32_t)generation;
@end

typedef struct Core {
	pthread_mutex_t lock;     // everything below, except what is marked
	pthread_mutex_t bufLock;  // busy[], endGeneration, bufCond
	pthread_cond_t bufCond;
	uint64_t uid;             // names the player in the registry; the callbacks of the queue only know the core
	id delegate;              // not retained

	// The sound
	id url, data, format, settings;
	NSUInteger channels;
	NSTimeInterval duration;
	ExtAudioFileRef ext;
	AudioFileID file;
	const void *dataBytes;
	SInt64 dataLength;
	double sampleRate;
	UInt32 fileChannels;
	SInt64 fileFrames;

	// Settings
	float volume, pan, rate;
	bool enableRate, metering;
	bool timePitch;           // the queue's time/pitch effect applies the rate (set when a session starts)
	NSInteger loops;
	bool fading;
	float fadeFrom, fadeTo;
	double fadeStart, fadeDuration;

	// Playback state
	bool playing;             // play() was called and the sound has not been paused, stopped or ended
	bool started;             // the queue was started and is not paused
	double restPosition;      // position in frames while there is no session
	int64_t startDelay;       // frames of silence before the sound, for playAtTime:

	// Session
	bool session;
	AudioQueueRef queue;
	AudioQueueBufferRef buffers[BufferCount];
	bool busy[BufferCount];   // bufLock
	uint32_t bufferFrames;
	pthread_t worker;
	bool exitWorker;          // bufLock
	uint32_t generation;
	uint32_t endGeneration;   // bufLock: nonzero while the queue is draining at the end of the sound
	bool drained;             // no more audio to read
	bool stopRequested;
	OSStatus decodeError;

	// Reading
	float *source;            // stereo frames
	float *raw;               // as in the file
	int sourceCount;
	double sourcePosition;    // within source[]
	double sourceOrigin;      // file frame of source[0]
	bool eof;
	NSInteger loopsLeft;
	int64_t enqueued;

	Segment segments[SegmentCapacity];
	int segmentCount;
	int64_t serial;
	int64_t meterApplied;

	// Meters, in dB
	float avg[2], peak[2], hold[2];
	float shownAvg[2], shownPeak[2];
} Core;

// The players that exist, for the notification that the sound has ended: it arrives on another thread and
// must not touch a player that is being destroyed.
static pthread_mutex_t APRegistryLock = PTHREAD_MUTEX_INITIALIZER;
static NSMutableDictionary *APRegistry;
static uint64_t APNextId = 1;

static void APRegister(Core *c, id player)
{
	pthread_mutex_lock(&APRegistryLock);
	if (!APRegistry)
		APRegistry = [[NSMutableDictionary alloc] init];
	c->uid = APNextId++;
	APRegistry[@(c->uid)] = [NSValue valueWithPointer:player];
	pthread_mutex_unlock(&APRegistryLock);
}

static void APUnregister(Core *c)
{
	pthread_mutex_lock(&APRegistryLock);
	[APRegistry removeObjectForKey:@(c->uid)];
	pthread_mutex_unlock(&APRegistryLock);
}

// The player, retained, or nil if it is gone.
static id APRetainedPlayer(uint64_t uid)
{
	pthread_mutex_lock(&APRegistryLock);
	id player = [(NSValue *) APRegistry[@(uid)] pointerValue];
	[player retain];
	pthread_mutex_unlock(&APRegistryLock);
	return player;
}

static double APSeconds(void)
{
	static mach_timebase_info_data_t base;
	if (base.denom == 0)
		mach_timebase_info(&base);
	return (double) mach_absolute_time() * base.numer / base.denom / 1e9;
}

static float APDecibels(float linear)
{
	if (linear <= 0)
		return (float) MeterFloorWhilePlaying;
	float db = 20.0f * log10f(linear);
	return db < MeterFloorWhilePlaying ? (float) MeterFloorWhilePlaying : db;
}

static float APClamp(float v, float lo, float hi)
{
	return v < lo ? lo : (v > hi ? hi : v);
}

// ---- opening ----

static OSStatus APReadData(void *ctx, SInt64 pos, UInt32 request, void *buffer, UInt32 *actual)
{
	Core *c = ctx;
	if (pos >= c->dataLength)
	{
		*actual = 0;
		return noErr;
	}
	UInt32 n = request;
	if (pos + n > c->dataLength)
		n = (UInt32) (c->dataLength - pos);
	memcpy(buffer, (const char *) c->dataBytes + pos, n);
	*actual = n;
	return noErr;
}

// The sound is only read
static OSStatus APWriteData(void *ctx, SInt64 pos, UInt32 request, const void *buffer, UInt32 *actual)
{
	return kAudioFilePermissionsError;
}

static SInt64 APDataSize(void *ctx)
{
	return ((Core *) ctx)->dataLength;
}

static OSStatus APSetDataSize(void *ctx, SInt64 size)
{
	return kAudioFilePermissionsError;
}

// The type of audio file that a UTI names, 0 if none.
static AudioFileTypeID APTypeForUTI(NSString *uti)
{
	if (!uti.length)
		return 0;

	UInt32 size = 0;
	if (AudioFileGetGlobalInfoSize(kAudioFileGlobalInfo_ReadableTypes, 0, NULL, &size) != noErr || size == 0)
		return 0;

	AudioFileTypeID *types = malloc(size);
	AudioFileTypeID result = 0;
	if (types && AudioFileGetGlobalInfo(kAudioFileGlobalInfo_ReadableTypes, 0, NULL, &size, types) == noErr)
	{
		for (UInt32 i = 0; i < size / sizeof(AudioFileTypeID) && !result; i++)
		{
			CFArrayRef utis = NULL;
			UInt32 utiSize = sizeof(utis);
			if (AudioFileGetGlobalInfo(kAudioFileGlobalInfo_UTIsForType, sizeof(types[i]), &types[i], &utiSize, &utis) == noErr && utis)
			{
				for (NSString *candidate in (NSArray *) utis)
				{
					if ([candidate caseInsensitiveCompare:uti] == NSOrderedSame)
						result = types[i];
				}
				CFRelease(utis);
			}
		}
	}
	free(types);
	return result;
}

static void APCloseSource(Core *c)
{
	if (c->ext)
		ExtAudioFileDispose(c->ext);
	if (c->file)
		AudioFileClose(c->file);
	c->ext = NULL;
	c->file = NULL;
}

static OSStatus APOpenSource(Core *c, NSURL *url, AudioFileTypeID hint, AudioStreamBasicDescription *fileFormat, AudioFileTypeID *fileType)
{
	OSStatus status;

	if (url)
		status = AudioFileOpenURL((CFURLRef) url, kAudioFileReadPermission, hint, &c->file);
	else
		status = AudioFileOpenWithCallbacks(c, APReadData, APWriteData, APDataSize, APSetDataSize, hint, &c->file);
	if (status != noErr)
	{
		c->file = NULL;
		return status;
	}

	if ((status = ExtAudioFileWrapAudioFileID(c->file, false, &c->ext)) != noErr)
	{
		c->ext = NULL;
		APCloseSource(c);
		return status;
	}

	UInt32 size = sizeof(*fileFormat);
	if ((status = ExtAudioFileGetProperty(c->ext, kExtAudioFileProperty_FileDataFormat, &size, fileFormat)) == noErr)
	{
		size = sizeof(c->fileFrames);
		status = ExtAudioFileGetProperty(c->ext, kExtAudioFileProperty_FileLengthFrames, &size, &c->fileFrames);
	}
	if (status == noErr)
	{
		size = sizeof(*fileType);
		if (AudioFileGetProperty(c->file, kAudioFilePropertyFileFormat, &size, fileType) != noErr)
			*fileType = 0;
	}
	if (status == noErr && (fileFormat->mSampleRate <= 0 || fileFormat->mChannelsPerFrame == 0))
		status = kAudioFileUnsupportedDataFormatError;

	if (status == noErr)
	{
		// The client side of the converter: float samples, as the file has them (the audio is mixed down for the queue by this class)
		AudioStreamBasicDescription client = {};
		client.mSampleRate = fileFormat->mSampleRate;
		client.mFormatID = kAudioFormatLinearPCM;
		client.mFormatFlags = kAudioFormatFlagsNativeFloatPacked;
		client.mChannelsPerFrame = fileFormat->mChannelsPerFrame;
		client.mBitsPerChannel = 32;
		client.mFramesPerPacket = 1;
		client.mBytesPerFrame = client.mBytesPerPacket = 4 * client.mChannelsPerFrame;
		status = ExtAudioFileSetProperty(c->ext, kExtAudioFileProperty_ClientDataFormat, sizeof(client), &client);
	}

	if (status != noErr)
		APCloseSource(c);
	return status;
}

// ---- reading and rate conversion ----

static float APEffectiveRate(Core *c)
{
	return c->enableRate ? APClamp(c->rate, 0.0f, 4.0f) : 1.0f;
}

// The play rate of the queue's time/pitch effect. The queue's sample time is the time of the sound at the normal rate.
static void APApplyRate(Core *c)
{
	if (c->session && c->timePitch)
	{
		const float rate = APEffectiveRate(c);
		AudioQueueSetParameter(c->queue, kAudioQueueParam_PlayRate, rate > 0 ? rate : 1.0f);
	}
}

// Reads the next chunk of the file into source[], mixed to two channels. False at the end or on error.
static bool APRead(Core *c)
{
	if (c->eof)
		return false;

	AudioBufferList list;
	list.mNumberBuffers = 1;
	list.mBuffers[0].mNumberChannels = c->fileChannels;
	list.mBuffers[0].mDataByteSize = SourceChunk * c->fileChannels * sizeof(float);
	list.mBuffers[0].mData = c->raw;

	UInt32 frames = SourceChunk;
	OSStatus status = ExtAudioFileRead(c->ext, &frames, &list);
	if (status != noErr)
	{
		c->decodeError = status;
		c->eof = true;
		return false;
	}
	if (frames == 0)
	{
		c->eof = true;
		return false;
	}

	const UInt32 n = c->fileChannels;
	float *out = c->source + (size_t) c->sourceCount * 2;
	const float *in = c->raw;

	for (UInt32 i = 0; i < frames; i++, in += n, out += 2)
	{
		if (n == 1)
			out[0] = out[1] = in[0];
		else if (n == 6)
		{
			// Front left and right, centre and the two surrounds folded in; the low frequency channel is left out
			out[0] = 0.414f * (in[0] + 0.7071f * (in[2] + in[4]));
			out[1] = 0.414f * (in[1] + 0.7071f * (in[2] + in[5]));
		}
		else
		{
			out[0] = in[0];
			out[1] = in[1];
		}
	}
	c->sourceCount += frames;
	return true;
}

static void APRestartSource(Core *c, SInt64 frame)
{
	if (frame < 0)
		frame = 0;
	ExtAudioFileSeek(c->ext, frame);
	c->eof = false;
	c->sourceCount = 0;
	c->sourcePosition = 0;
	c->sourceOrigin = (double) frame;
}

// Fills one queue buffer. Returns false if there was nothing to put in it.
static bool APFill(Core *c, int index)
{
	AudioQueueBufferRef buffer = c->buffers[index];
	float *out = buffer->mAudioData;
	const uint32_t capacity = c->bufferFrames;
	Segment seg = {};
	uint32_t k = 0;

	seg.serial = ++c->serial;
	seg.sourceStart = c->sourceOrigin + c->sourcePosition;

	const float rate = APEffectiveRate(c);
	// With the queue's time/pitch effect the sound is enqueued as it is and the queue plays it at the rate.
	const float step = c->timePitch && rate != 0 ? 1.0f : rate;

	if (c->startDelay > 0 || rate == 0)
	{
		// Silence, with the position of the sound standing still
		k = capacity;
		if (c->startDelay > 0)
		{
			k = c->startDelay < capacity ? (uint32_t) c->startDelay : capacity;
			c->startDelay -= k;
		}
		memset(out, 0, (size_t) k * 2 * sizeof(float));
		seg.sourceStep = 0;
	}
	else
	{
		seg.sourceStep = step;
		bool atEnd = false;

		while (k < capacity)
		{
			int i = (int) c->sourcePosition;

			if (i + 1 >= c->sourceCount && !c->eof)
			{
				// Drop what has been used and read on
				if (i > 0)
				{
					memmove(c->source, c->source + (size_t) i * 2, (size_t) (c->sourceCount - i) * 2 * sizeof(float));
					c->sourceCount -= i;
					c->sourceOrigin += i;
					c->sourcePosition -= i;
				}
				APRead(c);
				continue;
			}
			if (i >= c->sourceCount)
			{
				atEnd = true;
				break;
			}

			const float *a = c->source + (size_t) i * 2;
			const float *b = (i + 1 < c->sourceCount) ? a + 2 : a;
			const float f = (float) (c->sourcePosition - i);

			for (int ch = 0; ch < 2; ch++)
			{
				float v = f == 0 ? a[ch] : a[ch] + (b[ch] - a[ch]) * f;
				out[k * 2 + ch] = v;
				float m = fabsf(v);
				if (m > seg.peak[ch])
					seg.peak[ch] = m;
			}
			c->sourcePosition += step;
			k++;
		}

		if (atEnd || (c->eof && (int) c->sourcePosition >= c->sourceCount))
		{
			if (c->decodeError == noErr && c->loopsLeft != 0 && c->fileFrames > 0)
			{
				if (c->loopsLeft > 0)
					c->loopsLeft--;
				APRestartSource(c, 0);
			}
			else
				c->drained = true;
		}
		if (c->decodeError != noErr)
			c->drained = true;
	}

	if (k == 0)
		return false;

	seg.frames = k;
	seg.queueStart = c->enqueued;
	if (c->started)
	{
		AudioTimeStamp stamp;
		Boolean discontinuity;
		if (AudioQueueGetCurrentTime(c->queue, NULL, &stamp, &discontinuity) == noErr && (stamp.mFlags & kAudioTimeStampSampleTimeValid) &&
		    (int64_t) stamp.mSampleTime > seg.queueStart)
			seg.queueStart = (int64_t) stamp.mSampleTime; // the queue ran dry: it has gone on without us
	}
	c->enqueued = seg.queueStart + k;

	buffer->mAudioDataByteSize = k * 2 * sizeof(float);
	if (c->segmentCount < SegmentCapacity)
		c->segments[c->segmentCount++] = seg;

	if (AudioQueueEnqueueBuffer(c->queue, buffer, 0, NULL) != noErr)
	{
		c->segmentCount--;
		return false;
	}
	return true;
}

// When the last audio is in the queue, the queue is told to stop once it has played it; that is how the end of the sound is noticed.
static void APFinishIfDrained(Core *c)
{
	if (!c->drained || !c->started || c->stopRequested)
		return;
	c->stopRequested = true;
	pthread_mutex_lock(&c->bufLock);
	c->endGeneration = c->generation;
	pthread_mutex_unlock(&c->bufLock);
	AudioQueueStop(c->queue, false);
}

static void APFillFree(Core *c)
{
	for (int i = 0; i < BufferCount && !c->drained; i++)
	{
		pthread_mutex_lock(&c->bufLock);
		bool free = !c->busy[i];
		if (free)
			c->busy[i] = true;
		pthread_mutex_unlock(&c->bufLock);
		if (!free)
			continue;

		if (!APFill(c, i))
		{
			pthread_mutex_lock(&c->bufLock);
			c->busy[i] = false;
			pthread_mutex_unlock(&c->bufLock);
			break;
		}
	}
	APFinishIfDrained(c);
}

// ---- meters ----

static void APMeterApply(Core *c, const Segment *seg)
{
	if (seg->serial <= c->meterApplied)
		return;
	c->meterApplied = seg->serial;

	const double dt = seg->frames / c->sampleRate;
	const unsigned channels = c->fileChannels > 1 ? 2 : 1;

	for (unsigned ch = 0; ch < channels; ch++)
	{
		const float level = APDecibels(seg->peak[ch]);

		float avg = c->avg[ch] - (float) (AverageFallDbPerSecond * dt);
		if (level > avg)
			avg = level;
		c->avg[ch] = avg < MeterFloorWhilePlaying ? (float) MeterFloorWhilePlaying : avg;

		if (level >= c->peak[ch])
		{
			c->peak[ch] = level;
			c->hold[ch] = (float) PeakHoldSeconds;
		}
		else
		{
			float fall = (float) dt;
			if (c->hold[ch] > 0)
			{
				const float held = c->hold[ch] < fall ? c->hold[ch] : fall;
				c->hold[ch] -= held;
				fall -= held;
			}
			c->peak[ch] -= (float) PeakFallDbPerSecond * fall;
			if (c->peak[ch] < MeterFloorWhilePlaying)
				c->peak[ch] = (float) MeterFloorWhilePlaying;
		}
	}
}

static void APMeterReset(Core *c)
{
	for (int i = 0; i < 2; i++)
	{
		c->avg[i] = c->peak[i] = (float) MeterFloorWhilePlaying;
		c->hold[i] = 0;
	}
}

// ---- position ----

static bool APHeard(Core *c, double *frames)
{
	AudioTimeStamp stamp;
	Boolean discontinuity;

	if (!c->started && c->segmentCount == 0)
		return false;
	if (AudioQueueGetCurrentTime(c->queue, NULL, &stamp, &discontinuity) != noErr || !(stamp.mFlags & kAudioTimeStampSampleTimeValid))
		return false;
	*frames = stamp.mSampleTime;
	return true;
}

// The file position that is being heard, in frames.
static double APPosition(Core *c)
{
	if (!c->session || !c->started || c->segmentCount == 0)
		return c->restPosition; // not playing: the position where it was left

	double heard;
	if (!APHeard(c, &heard))
		return c->segments[0].sourceStart;

	while (c->segmentCount > 1 && c->segments[1].queueStart <= heard)
	{
		if (c->metering)
			APMeterApply(c, &c->segments[0]);
		memmove(&c->segments[0], &c->segments[1], (size_t) (c->segmentCount - 1) * sizeof(Segment));
		c->segmentCount--;
	}

	const Segment *seg = &c->segments[0];
	double into = heard - seg->queueStart;
	if (into < 0)
		into = 0;
	if (into > seg->frames)
		into = seg->frames;
	return seg->sourceStart + into * seg->sourceStep;
}

// ---- volume ----

static float APGain(Core *c)
{
	if (c->fading)
	{
		const double t = (APSeconds() - c->fadeStart) / c->fadeDuration;
		if (t < 1)
			return c->fadeFrom + (c->fadeTo - c->fadeFrom) * (float) (t < 0 ? 0 : t);
		c->fading = false;
	}
	return c->volume;
}

static void APApplyVolume(Core *c)
{
	if (!c->session)
		return;
	AudioQueueSetParameter(c->queue, kAudioQueueParam_Volume, APClamp(APGain(c), 0.0f, 1.0f));
}

static void APApplyPan(Core *c)
{
	if (c->session)
		AudioQueueSetParameter(c->queue, kAudioQueueParam_Pan, APClamp(c->pan, -1.0f, 1.0f));
}

// ---- the session ----

static void APBufferDone(void *ctx, AudioQueueRef queue, AudioQueueBufferRef buffer)
{
	Core *c = ctx;
	pthread_mutex_lock(&c->bufLock);
	for (int i = 0; i < BufferCount; i++)
	{
		if (c->buffers[i] == buffer)
			c->busy[i] = false;
	}
	pthread_cond_broadcast(&c->bufCond);
	pthread_mutex_unlock(&c->bufLock);
}

static void APRunningChanged(void *ctx, AudioQueueRef queue, AudioQueuePropertyID property);

static void *APWorker(void *arg)
{
	Core *c = arg;

	for (;;)
	{
		pthread_mutex_lock(&c->bufLock);
		if (!c->exitWorker)
		{
			struct timespec until;
			clock_gettime(CLOCK_REALTIME, &until);
			until.tv_nsec += WorkerTickMs * 1000000L;
			if (until.tv_nsec >= 1000000000L)
			{
				until.tv_sec++;
				until.tv_nsec -= 1000000000L;
			}
			pthread_cond_timedwait(&c->bufCond, &c->bufLock, &until);
		}
		const bool exiting = c->exitWorker;
		pthread_mutex_unlock(&c->bufLock);
		if (exiting)
			break;

		pthread_mutex_lock(&c->lock);
		if (c->session)
		{
			APApplyVolume(c);
			APPosition(c);
			APFillFree(c);
		}
		pthread_mutex_unlock(&c->lock);
	}
	return NULL;
}

static void APSetFormat(Core *c, AudioStreamBasicDescription *f)
{
	memset(f, 0, sizeof(*f));
	f->mSampleRate = c->sampleRate;
	f->mFormatID = kAudioFormatLinearPCM;
	f->mFormatFlags = kAudioFormatFlagsNativeFloatPacked;
	f->mChannelsPerFrame = 2;
	f->mBitsPerChannel = 32;
	f->mFramesPerPacket = 1;
	f->mBytesPerFrame = f->mBytesPerPacket = 8;
}

// Called with the lock held. Prepares the queue with its first buffers; the sound begins at `frame`.
static bool APSessionStart(Core *c, double frame)
{
	AudioStreamBasicDescription format;
	APSetFormat(c, &format);

	c->bufferFrames = (uint32_t) (c->sampleRate / 20);
	if (c->bufferFrames < 1024)
		c->bufferFrames = 1024;

	if (AudioQueueNewOutput(&format, APBufferDone, c, NULL, NULL, 0, &c->queue) != noErr)
	{
		c->queue = NULL;
		return false;
	}

	for (int i = 0; i < BufferCount; i++)
	{
		if (AudioQueueAllocateBuffer(c->queue, c->bufferFrames * 8, &c->buffers[i]) != noErr)
		{
			AudioQueueDispose(c->queue, true);
			c->queue = NULL;
			return false;
		}
		c->busy[i] = false;
	}
	AudioQueueAddPropertyListener(c->queue, kAudioQueueProperty_IsRunning, APRunningChanged, c);
	// The rate is applied by the queue's time/pitch effect, which keeps the pitch (it has to be enabled before the
	// queue is primed or started). Without it the sound is resampled to the rate here, so the pitch follows.
	{
		const UInt32 enable = 1, algorithm = kAudioQueueTimePitchAlgorithm_Spectral;
		c->timePitch = AudioQueueSetProperty(c->queue, kAudioQueueProperty_EnableTimePitch, &enable, sizeof(enable)) == noErr &&
			AudioQueueSetProperty(c->queue, kAudioQueueProperty_TimePitchAlgorithm, &algorithm, sizeof(algorithm)) == noErr;
	}

	c->session = true;
	c->generation++;
	c->drained = c->stopRequested = false;
	c->decodeError = noErr;
	c->segmentCount = 0;
	c->enqueued = 0;
	c->loopsLeft = c->loops;
	c->started = false;
	pthread_mutex_lock(&c->bufLock);
	c->endGeneration = 0;
	c->exitWorker = false;
	pthread_mutex_unlock(&c->bufLock);
	APMeterReset(c);
	c->meterApplied = c->serial;

	c->restPosition = frame;
	APRestartSource(c, (SInt64) frame);
	c->sourcePosition = frame - (double) (SInt64) frame; // the fraction a rate change left
	APApplyVolume(c);
	APApplyPan(c);
	APApplyRate(c);
	APFillFree(c);

	if (pthread_create(&c->worker, NULL, APWorker, c) != 0)
	{
		AudioQueueDispose(c->queue, true);
		c->queue = NULL;
		c->session = false;
		return false;
	}
	return true;
}

// Called with the lock held, and gives it up for a moment.
static void APSessionEnd(Core *c)
{
	if (!c->session)
		return;

	pthread_mutex_lock(&c->bufLock);
	c->exitWorker = true;
	c->endGeneration = 0;
	pthread_cond_broadcast(&c->bufCond);
	pthread_mutex_unlock(&c->bufLock);

	pthread_mutex_unlock(&c->lock);
	pthread_join(c->worker, NULL);
	pthread_mutex_lock(&c->lock);

	AudioQueueDispose(c->queue, true); // no callback runs after this returns
	c->queue = NULL;
	c->session = false;
	c->started = false;
	c->segmentCount = 0;
}

// Moves a prepared or playing session to another position (the lock is held).
static void APReposition(Core *c, double frame)
{
	AudioQueueReset(c->queue); // the buffers come back through APBufferDone before it returns
	pthread_mutex_lock(&c->bufLock);
	for (int i = 0; i < BufferCount; i++)
		c->busy[i] = false;
	c->endGeneration = 0;
	pthread_mutex_unlock(&c->bufLock);

	c->segmentCount = 0;
	c->drained = c->stopRequested = false;
	c->decodeError = noErr;
	c->loopsLeft = c->loops;
	c->enqueued = 0;
	double heard;
	if (APHeard(c, &heard))
		c->enqueued = (int64_t) heard;
	c->meterApplied = c->serial;
	APMeterReset(c);
	c->restPosition = frame;
	APRestartSource(c, (SInt64) frame);
	c->sourcePosition = frame - (double) (SInt64) frame;
	APFillFree(c);
}

// Called with the lock held, after the effective rate changed from `before`. The queue plays what it holds at the new rate; a
// sound that was queued as silence (rate 0) or as sound has to be read again when the rate becomes or stops being 0, and
// without the time/pitch effect whatever was queued at the old rate.
static void APRateChanged(Core *c, float before)
{
	if (!c->session)
		return;
	APApplyRate(c);
	if (!c->timePitch || (before == 0) != (APEffectiveRate(c) == 0))
		APReposition(c, APPosition(c));
}

@implementation AVAudioPlayer

static NSError *APError(OSStatus status)
{
	return [NSError errorWithDomain:NSOSStatusErrorDomain code:status userInfo:nil];
}

- (instancetype)initWithContentsOfURL:(NSURL *)url error:(NSError **)outError
{
	return [self initWithContentsOfURL:url fileTypeHint:nil error:outError];
}

- (instancetype)initWithData:(NSData *)data error:(NSError **)outError
{
	return [self initWithData:data fileTypeHint:nil error:outError];
}

- (instancetype)initWithContentsOfURL:(NSURL *)url fileTypeHint:(NSString *)hint error:(NSError **)outError
{
	if (!url || !url.isFileURL)
	{
		if (outError)
			*outError = APError(kAudioFileUnspecifiedError);
		return nil;
	}
	return [self initWithURL:url data:nil hint:hint error:outError];
}

- (instancetype)initWithData:(NSData *)data fileTypeHint:(NSString *)hint error:(NSError **)outError
{
	if (!data || data.length == 0)
	{
		// QUIRK: no data is reported as -39 (eofErr) in the OSStatus error domain, not as the status of an unsupported file type.
		// Evidence: black-box observation of AVAudioPlayer on macOS 10.14 and 26 (tier 2).
		// Test: tests/avaudioplayer.m, errors: "empty data".
		if (outError)
			*outError = APError(-39);
		return nil;
	}
	return [self initWithURL:nil data:data hint:hint error:outError];
}

- (instancetype)initWithURL:(NSURL *)url data:(NSData *)data hint:(NSString *)hint error:(NSError **)outError
{
	if (!(self = [super init]))
		return nil;

	Core *c = calloc(1, sizeof(Core));
	if (!c)
	{
		[self release];
		return nil;
	}

	c->url = [url retain];
	c->data = [data retain];
	c->dataBytes = data.bytes;
	c->dataLength = data.length;

	AudioStreamBasicDescription fileFormat;
	AudioFileTypeID fileType = 0;
	OSStatus status = APOpenSource(c, url, APTypeForUTI(hint), &fileFormat, &fileType);
	if (status != noErr)
	{
		[c->url release];
		[c->data release];
		free(c);
		if (outError)
			*outError = APError(status);
		[self release];
		return nil;
	}

	pthread_mutex_init(&c->lock, NULL);
	pthread_mutex_init(&c->bufLock, NULL);
	pthread_cond_init(&c->bufCond, NULL);
	APRegister(c, self);

	c->sampleRate = fileFormat.mSampleRate;
	c->fileChannels = fileFormat.mChannelsPerFrame;
	c->volume = 1.0f;
	c->rate = 1.0f;
	c->source = malloc((size_t) SourceChunk * 3 * 2 * sizeof(float));
	c->raw = malloc((size_t) SourceChunk * c->fileChannels * sizeof(float));
	APMeterReset(c);
	for (int i = 0; i < 2; i++)
		c->shownAvg[i] = c->shownPeak[i] = (float) MeterFloorIdle;
	_core = c;

	c->channels = fileFormat.mChannelsPerFrame;
	c->duration = c->fileFrames / fileFormat.mSampleRate;

	// QUIRK: the format property is that of the file: for PCM its description (double precision samples as single precision),
	// for anything else only the kind of format, the sample rate and the channels, without packet or frame sizes.
	// Evidence: black-box observation of AVAudioPlayer on macOS 10.14 and 26 (tier 2).
	// Test: tests/avaudioplayer.m, "== creation and properties" and "== compressed sounds".
	AudioStreamBasicDescription shown = fileFormat;
	if (fileFormat.mFormatID != kAudioFormatLinearPCM)
	{
		memset(&shown, 0, sizeof(shown));
		shown.mSampleRate = fileFormat.mSampleRate;
		shown.mFormatID = fileFormat.mFormatID;
		shown.mChannelsPerFrame = fileFormat.mChannelsPerFrame;
	}
	else if ((fileFormat.mFormatFlags & kAudioFormatFlagIsFloat) && fileFormat.mBitsPerChannel == 64)
	{
		// Double precision samples are played as single precision, and that is what the format reports
		shown.mFormatFlags = kAudioFormatFlagsNativeFloatPacked | (fileFormat.mFormatFlags & kAudioFormatFlagIsNonInterleaved);
		shown.mBitsPerChannel = 32;
		shown.mBytesPerFrame = shown.mBytesPerPacket = 4 * ((fileFormat.mFormatFlags & kAudioFormatFlagIsNonInterleaved) ? 1 : shown.mChannelsPerFrame);
	}
	c->format = [[AVAudioFormat alloc] initWithStreamDescription:&shown];

	NSMutableDictionary *settings = [NSMutableDictionary dictionary];
	settings[AVFormatIDKey] = @(fileFormat.mFormatID);
	settings[AVSampleRateKey] = @(fileFormat.mSampleRate);
	settings[AVNumberOfChannelsKey] = @(fileFormat.mChannelsPerFrame);
	settings[AVAudioFileTypeKey] = @(fileType);
	settings[AVEncoderBitRateKey] = @0;
	if (fileFormat.mFormatID == kAudioFormatLinearPCM)
	{
		settings[AVLinearPCMBitDepthKey] = @(fileFormat.mBitsPerChannel);
		settings[AVLinearPCMIsBigEndianKey] = @((fileFormat.mFormatFlags & kAudioFormatFlagIsBigEndian) != 0);
		settings[AVLinearPCMIsFloatKey] = @((fileFormat.mFormatFlags & kAudioFormatFlagIsFloat) != 0);
		settings[AVLinearPCMIsNonInterleaved] = @((fileFormat.mFormatFlags & kAudioFormatFlagIsNonInterleaved) != 0);
	}
	c->settings = [settings copy];

	return self;
}

- (void)dealloc
{
	Core *c = (Core *) _core;
	if (!c)
	{
		[super dealloc];
		return;
	}

	APUnregister(c);
	pthread_mutex_lock(&c->lock);
	APSessionEnd(c);
	pthread_mutex_unlock(&c->lock);

	APCloseSource(c);
	[c->url release];
	[c->data release];
	[c->format release];
	[c->settings release];
	free(c->source);
	free(c->raw);
	pthread_mutex_destroy(&c->lock);
	pthread_mutex_destroy(&c->bufLock);
	pthread_cond_destroy(&c->bufCond);
	free(c);
	[super dealloc];
}

// ---- transport ----

- (BOOL)prepareToPlay
{
	Core *c = (Core *) _core;
	BOOL ok = YES;

	pthread_mutex_lock(&c->lock);
	if (!c->session)
		ok = APSessionStart(c, c->restPosition);
	pthread_mutex_unlock(&c->lock);
	return ok;
}

- (BOOL)play
{
	Core *c = (Core *) _core;
	BOOL ok = YES;

	pthread_mutex_lock(&c->lock);
	if (!(c->playing && c->started))
	{
		if (!c->session)
			ok = APSessionStart(c, c->restPosition);
		if (ok)
		{
			if (AudioQueueStart(c->queue, NULL) == noErr)
			{
				c->started = true;
				c->playing = true;
				if (c->stopRequested)
					AudioQueueStop(c->queue, false); // the end was near when it was paused: starting again cancelled the stop
				APFinishIfDrained(c);
			}
			else
				ok = NO;
		}
	}
	pthread_mutex_unlock(&c->lock);
	return ok;
}

- (BOOL)playAtTime:(NSTimeInterval)time
{
	Core *c = (Core *) _core;
	const double wait = time - APSeconds();

	pthread_mutex_lock(&c->lock);
	const bool alreadyPlaying = c->playing && c->started;
	if (!alreadyPlaying && wait > 0)
	{
		// The sound starts after some silence; a time that has passed means now
		c->restPosition = APPosition(c);
		APSessionEnd(c);
		// The delay is silence in the sound's own time, which the queue plays at the rate.
		const float rate = APEffectiveRate(c);
		c->startDelay = (int64_t) llround(wait * c->sampleRate * (rate > 0 ? rate : 1.0f));
	}
	pthread_mutex_unlock(&c->lock);
	return [self play];
}

- (void)pause
{
	Core *c = (Core *) _core;

	pthread_mutex_lock(&c->lock);
	if (c->session && c->started)
	{
		c->restPosition = APPosition(c);
		AudioQueuePause(c->queue);
		c->started = false;
	}
	c->playing = false;
	pthread_mutex_unlock(&c->lock);
}

- (void)stop
{
	// QUIRK: stopping keeps the position: currentTime stays where the sound was, and playing again goes on from there
	// (the position is only back at the start after the sound has played to its end).
	// Evidence: black-box observation of AVAudioPlayer on macOS 26 (tier 2).
	// Test: tests/avaudioplayer.m, "stop keeps the position".
	Core *c = (Core *) _core;

	pthread_mutex_lock(&c->lock);
	if (c->session)
	{
		c->restPosition = APPosition(c);
		APSessionEnd(c);
	}
	c->playing = false;
	c->startDelay = 0;
	pthread_mutex_unlock(&c->lock);
}

// The queue has played everything of the sound. Runs on the main thread.
- (void)playbackEndedWithGeneration:(uint32_t)generation
{
	Core *c = (Core *) _core;

	pthread_mutex_lock(&c->lock);
	if (!c->session || c->generation != generation || !c->playing)
	{
		pthread_mutex_unlock(&c->lock);
		return;
	}
	const OSStatus error = c->decodeError;
	APSessionEnd(c);
	c->restPosition = 0;
	c->playing = false;
	pthread_mutex_unlock(&c->lock);

	id<AVAudioPlayerDelegate> delegate = c->delegate;
	if (error != noErr && [delegate respondsToSelector:@selector(audioPlayerDecodeErrorDidOccur:error:)])
		[delegate audioPlayerDecodeErrorDidOccur:self error:APError(error)];
	if ([delegate respondsToSelector:@selector(audioPlayerDidFinishPlaying:successfully:)])
		[delegate audioPlayerDidFinishPlaying:self successfully:error == noErr];
}

static void APRunningChanged(void *ctx, AudioQueueRef queue, AudioQueuePropertyID property)
{
	Core *c = ctx;
	UInt32 running = 1, size = sizeof(running);

	if (AudioQueueGetProperty(queue, kAudioQueueProperty_IsRunning, &running, &size) != noErr || running)
		return;

	pthread_mutex_lock(&c->bufLock);
	const uint32_t generation = c->endGeneration;
	pthread_mutex_unlock(&c->bufLock);
	if (!generation)
		return; // stopped on purpose

	const uint64_t uid = c->uid;
	dispatch_async(dispatch_get_main_queue(), ^{
		AVAudioPlayer *player = APRetainedPlayer(uid);
		[player playbackEndedWithGeneration:generation];
		[player release];
	});
}

- (BOOL)isPlaying
{
	Core *c = (Core *) _core;
	pthread_mutex_lock(&c->lock);
	const BOOL playing = c->playing;
	pthread_mutex_unlock(&c->lock);
	return playing;
}

// ---- properties ----

- (NSUInteger)numberOfChannels
{
	return ((Core *) _core)->channels;
}

- (NSTimeInterval)duration
{
	return ((Core *) _core)->duration;
}

- (NSURL *)url
{
	return ((Core *) _core)->url;
}

- (NSData *)data
{
	return ((Core *) _core)->data;
}

- (AVAudioFormat *)format
{
	return ((Core *) _core)->format;
}

- (NSDictionary<NSString *, id> *)settings
{
	return ((Core *) _core)->settings;
}

- (id<AVAudioPlayerDelegate>)delegate
{
	return ((Core *) _core)->delegate;
}

- (void)setDelegate:(id<AVAudioPlayerDelegate>)delegate
{
	((Core *) _core)->delegate = delegate;
}

- (float)volume
{
	Core *c = (Core *) _core;
	pthread_mutex_lock(&c->lock);
	const float v = c->volume;
	pthread_mutex_unlock(&c->lock);
	return v;
}

- (void)setVolume:(float)volume
{
	// QUIRK: volume, pan and rate read back what was set, also outside the documented ranges (0 to 1, -1 to 1, 0.5 to 2);
	// only what is played is limited. Evidence: black-box observation on macOS 10.14, 26 and 27 (tier 2).
	// Test: tests/avaudioplayer.m, "== setters".
	[self setVolume:volume fadeDuration:0];
}

- (void)setVolume:(float)volume fadeDuration:(NSTimeInterval)duration
{
	Core *c = (Core *) _core;

	pthread_mutex_lock(&c->lock);
	if (duration > 0 && c->session)
	{
		c->fadeFrom = APGain(c);
		c->fadeStart = APSeconds();
		c->fadeDuration = duration;
		c->fading = true;
	}
	else
		c->fading = false;
	c->fadeTo = c->volume = volume;
	APApplyVolume(c);
	pthread_mutex_unlock(&c->lock);
}

- (float)pan
{
	Core *c = (Core *) _core;
	pthread_mutex_lock(&c->lock);
	const float v = c->pan;
	pthread_mutex_unlock(&c->lock);
	return v;
}

- (void)setPan:(float)pan
{
	Core *c = (Core *) _core;
	pthread_mutex_lock(&c->lock);
	c->pan = pan;
	APApplyPan(c);
	pthread_mutex_unlock(&c->lock);
}

- (BOOL)enableRate
{
	Core *c = (Core *) _core;
	pthread_mutex_lock(&c->lock);
	const BOOL v = c->enableRate;
	pthread_mutex_unlock(&c->lock);
	return v;
}

- (void)setEnableRate:(BOOL)enableRate
{
	Core *c = (Core *) _core;
	pthread_mutex_lock(&c->lock);
	const bool changed = c->enableRate != enableRate;
	const float before = APEffectiveRate(c);
	c->enableRate = enableRate;
	if (changed)
		APRateChanged(c, before);
	pthread_mutex_unlock(&c->lock);
}

- (float)rate
{
	Core *c = (Core *) _core;
	pthread_mutex_lock(&c->lock);
	const float v = c->rate;
	pthread_mutex_unlock(&c->lock);
	return v;
}

- (void)setRate:(float)rate
{
	Core *c = (Core *) _core;
	pthread_mutex_lock(&c->lock);
	const bool changed = c->rate != rate && c->enableRate;
	const float before = APEffectiveRate(c);
	c->rate = rate;
	if (changed)
		APRateChanged(c, before);
	pthread_mutex_unlock(&c->lock);
}

- (NSInteger)numberOfLoops
{
	Core *c = (Core *) _core;
	pthread_mutex_lock(&c->lock);
	const NSInteger v = c->loops;
	pthread_mutex_unlock(&c->lock);
	return v;
}

- (void)setNumberOfLoops:(NSInteger)loops
{
	Core *c = (Core *) _core;
	pthread_mutex_lock(&c->lock);
	c->loops = loops;
	if (c->session)
		c->loopsLeft = loops;
	pthread_mutex_unlock(&c->lock);
}

- (NSTimeInterval)currentTime
{
	Core *c = (Core *) _core;
	pthread_mutex_lock(&c->lock);
	double frames = APPosition(c);
	pthread_mutex_unlock(&c->lock);

	const double seconds = frames / c->sampleRate;
	return seconds > c->duration ? c->duration : seconds;
}

- (void)setCurrentTime:(NSTimeInterval)time
{
	Core *c = (Core *) _core;

	if (time < 0)
		time = 0;
	if (time > c->duration)
		time = c->duration;

	pthread_mutex_lock(&c->lock);
	// QUIRK: a position past the end that is set while the sound plays makes it start again from the
	// beginning, while it is only limited to the duration when nothing plays. Undocumented.
	// Evidence: black-box observation of AVAudioPlayer on macOS (tier 2).
	// Test: tests/avaudioplayer.m (quirk: a position past the end while playing starts over).
	if (c->playing && c->started && time >= c->duration)
		time = 0;

	const double frame = time * c->sampleRate;
	if (c->session)
		APReposition(c, frame);
	else
		c->restPosition = frame;
	pthread_mutex_unlock(&c->lock);
}

- (NSTimeInterval)deviceCurrentTime
{
	return APSeconds();
}

- (BOOL)isMeteringEnabled
{
	Core *c = (Core *) _core;
	pthread_mutex_lock(&c->lock);
	const BOOL v = c->metering;
	pthread_mutex_unlock(&c->lock);
	return v;
}

- (void)setMeteringEnabled:(BOOL)enabled
{
	Core *c = (Core *) _core;
	pthread_mutex_lock(&c->lock);
	if (enabled && !c->metering)
	{
		APMeterReset(c);
		c->meterApplied = c->serial;
	}
	c->metering = enabled;
	pthread_mutex_unlock(&c->lock);
}

- (void)updateMeters
{
	Core *c = (Core *) _core;

	pthread_mutex_lock(&c->lock);
	if (c->metering && c->session && c->started)
	{
		APPosition(c); // takes what has been played into the meters
		if (c->segmentCount > 0)
			APMeterApply(c, &c->segments[0]);
		for (int i = 0; i < 2; i++)
		{
			c->shownAvg[i] = c->avg[i];
			c->shownPeak[i] = c->peak[i];
		}
	}
	else
	{
		for (int i = 0; i < 2; i++)
			c->shownAvg[i] = c->shownPeak[i] = (float) MeterFloorIdle;
		APMeterReset(c);
	}
	pthread_mutex_unlock(&c->lock);
}

- (float)peakPowerForChannel:(NSUInteger)channel
{
	Core *c = (Core *) _core;
	pthread_mutex_lock(&c->lock);
	float v = (float) MeterFloorIdle;
	if (c->metering && channel < 2 && channel < c->channels)
		v = c->shownPeak[channel];
	pthread_mutex_unlock(&c->lock);
	return v;
}

- (float)averagePowerForChannel:(NSUInteger)channel
{
	Core *c = (Core *) _core;
	pthread_mutex_lock(&c->lock);
	float v = (float) MeterFloorIdle;
	if (c->metering && channel < 2 && channel < c->channels)
		v = c->shownAvg[channel];
	pthread_mutex_unlock(&c->lock);
	return v;
}

@end
