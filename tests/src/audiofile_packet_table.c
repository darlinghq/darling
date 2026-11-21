/*
 * What AudioFile and ExtAudioFile say about the length of compressed audio: the packet length (frames per
 * packet) of AAC, FLAC and Apple Lossless, the priming and remainder frames of the packet table
 * (kAudioFilePropertyPacketTableInfo), the estimated duration, and ExtAudioFile's length in frames and what it reads, which leave out the
 * priming frames (the decoder's warm-up at the start of AAC) and the padding after the last frame.
 *
 * The expected values were observed on macOS (10.14 and 26) with the same files through these public functions.
 * The files are tones-aac.m4a, tones-flac.flac and tones-alac.m4a of tests/avplayer-fixtures (a 440 Hz sine,
 * then 880 Hz, two seconds) and aac-noedit.m4a of tests/avaudioplayer-fixtures (AAC in an MPEG-4 file without an
 * edit list). Run with both directories:
 *   audiofile_packet_table <tests/avplayer-fixtures> <tests/avaudioplayer-fixtures>
 *
 * Exit status is 0 when every check passed, 77 if the files cannot be read (the host's FFmpeg is missing).
 *
 * Copyright (C) 2026 Lubos Dolezel
 */
#include <AudioToolbox/AudioToolbox.h>
#include <CoreFoundation/CoreFoundation.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int failures;

#define CHECK(cond, ...) do { \
	if (cond) printf("PASS %s\n", #cond); \
	else { printf("FAIL %s: ", #cond); printf(__VA_ARGS__); printf("\n"); failures++; } \
} while (0)

struct expected {
	const char *dir; // 0: the first argument, 1: the second
	const char *file;
	UInt32 format;
	UInt32 flags;
	UInt32 framesPerPacket;
	SInt64 valid;
	SInt32 priming;
	SInt32 remainder;
	double sampleRate;
};

static const struct expected files[] = {
	{ "0", "tones-aac.m4a", kAudioFormatMPEG4AAC, 0, 1024, 88200, 1024, 888, 44100 },
	{ "1", "aac-noedit.m4a", kAudioFormatMPEG4AAC, 0, 1024, 88000, 2112, 0, 44100 },
	{ "0", "tones-flac.flac", kAudioFormatFLAC, 3, 1024, 44100, 0, 956, 22050 },
	{ "0", "tones-alac.m4a", kAudioFormatAppleLossless, 3, 4096, 44100, 0, 956, 22050 },
};

// Returns 1 if the file cannot be read here
static int check_file(const struct expected *e, const char *path)
{
	CFURLRef url = CFURLCreateFromFileSystemRepresentation(NULL, (const UInt8 *)path, strlen(path), false);
	AudioFileID file = NULL;
	printf("== %s\n", e->file);

	OSStatus status = AudioFileOpenURL(url, kAudioFileReadPermission, 0, &file);
	if (status != noErr) {
		printf("SKIP %s: AudioFileOpenURL %d\n", e->file, (int)status);
		CFRelease(url);
		return 1;
	}

	AudioStreamBasicDescription format;
	UInt32 size = sizeof(format);
	CHECK(AudioFileGetProperty(file, kAudioFilePropertyDataFormat, &size, &format) == noErr, "data format");
	CHECK(format.mFormatID == e->format && format.mSampleRate == e->sampleRate, "format %08x at %.0f Hz", (unsigned)format.mFormatID, format.mSampleRate);
	CHECK(format.mFramesPerPacket == e->framesPerPacket, "frames per packet %u", (unsigned)format.mFramesPerPacket);
	CHECK(format.mFormatFlags == e->flags, "format flags %u", (unsigned)format.mFormatFlags);

	AudioFilePacketTableInfo info = {};
	size = sizeof(info);
	status = AudioFileGetProperty(file, kAudioFilePropertyPacketTableInfo, &size, &info);
	CHECK(status == noErr, "packet table info: %d", (int)status);
	CHECK(info.mNumberValidFrames == e->valid && info.mPrimingFrames == e->priming && info.mRemainderFrames == e->remainder,
	      "valid %lld, priming %d, remainder %d", (long long)info.mNumberValidFrames, (int)info.mPrimingFrames, (int)info.mRemainderFrames);

	double duration = 0;
	size = sizeof(duration);
	CHECK(AudioFileGetProperty(file, kAudioFilePropertyEstimatedDuration, &size, &duration) == noErr &&
	      fabs(duration - e->valid / e->sampleRate) < 1e-6, "estimated duration %f", duration);

	ExtAudioFileRef ext = NULL;
	CHECK(ExtAudioFileWrapAudioFileID(file, false, &ext) == noErr, "wrap");
	SInt64 length = 0;
	size = sizeof(length);
	CHECK(ExtAudioFileGetProperty(ext, kExtAudioFileProperty_FileLengthFrames, &size, &length) == noErr && length == e->valid, "length %lld frames", (long long)length);

	AudioStreamBasicDescription client = {};
	client.mSampleRate = format.mSampleRate;
	client.mFormatID = kAudioFormatLinearPCM;
	client.mFormatFlags = kAudioFormatFlagsNativeFloatPacked;
	client.mChannelsPerFrame = format.mChannelsPerFrame;
	client.mBitsPerChannel = 32;
	client.mFramesPerPacket = 1;
	client.mBytesPerFrame = client.mBytesPerPacket = 4 * client.mChannelsPerFrame;
	CHECK(ExtAudioFileSetProperty(ext, kExtAudioFileProperty_ClientDataFormat, sizeof(client), &client) == noErr, "client format");

	float *buffer = malloc(4096 * client.mBytesPerFrame);
	long total = 0;
	double early = 0; // energy of frames 50 to 250: the sound has begun there, and the priming frames are not in the way
	for (;;) {
		AudioBufferList list;
		list.mNumberBuffers = 1;
		list.mBuffers[0].mNumberChannels = client.mChannelsPerFrame;
		list.mBuffers[0].mDataByteSize = 4096 * client.mBytesPerFrame;
		list.mBuffers[0].mData = buffer;
		UInt32 frames = 4096;
		if (ExtAudioFileRead(ext, &frames, &list) != noErr || frames == 0)
			break;
		for (UInt32 i = 0; i < frames; i++)
			if (total + (long)i >= 50 && total + (long)i < 250)
				early += buffer[i * client.mChannelsPerFrame] * buffer[i * client.mChannelsPerFrame];
		total += frames;
	}
	CHECK(total == e->valid, "reads %ld frames", total);
	CHECK(sqrt(early / 200) > 0.15, "the sound is there from the start: rms %f of frames 50-250", sqrt(early / 200));

	// A seek is counted in the frames of the sound
	CHECK(ExtAudioFileSeek(ext, e->valid / 2) == noErr, "seek");
	total = 0;
	for (;;) {
		AudioBufferList list;
		list.mNumberBuffers = 1;
		list.mBuffers[0].mNumberChannels = client.mChannelsPerFrame;
		list.mBuffers[0].mDataByteSize = 4096 * client.mBytesPerFrame;
		list.mBuffers[0].mData = buffer;
		UInt32 frames = 4096;
		if (ExtAudioFileRead(ext, &frames, &list) != noErr || frames == 0)
			break;
		total += frames;
	}
	CHECK(total == e->valid - e->valid / 2, "after a seek to the middle, %ld frames are left", total);

	free(buffer);
	ExtAudioFileDispose(ext);
	AudioFileClose(file);
	CFRelease(url);
	return 0;
}

int main(int argc, char **argv)
{
	if (argc < 3) {
		printf("usage: audiofile_packet_table <tests/avplayer-fixtures> <tests/avaudioplayer-fixtures>\n");
		return 1;
	}

	char path[1024];
	int skipped = 0;
	for (unsigned i = 0; i < sizeof(files) / sizeof(files[0]); i++) {
		snprintf(path, sizeof(path), "%s/%s", argv[atoi(files[i].dir) + 1], files[i].file);
		skipped += check_file(&files[i], path);
	}

	if (skipped == (int)(sizeof(files) / sizeof(files[0]))) {
		printf("SKIP: the files cannot be read\n");
		return 77;
	}

	printf("%s (%d failed)\n", failures ? "FAIL" : "PASS", failures);
	return failures ? 1 : 0;
}
