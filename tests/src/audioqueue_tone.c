/*
 * AudioQueue output of a PCM tone, and the FFmpeg-dependent calls.
 *
 * Plays a 440 Hz tone whose amplitude ramps linearly from 0 to 0.8 over the
 * given number of seconds (default 3) through an AudioQueue, so that a capture
 * of the output shows where each part of the buffer landed. Linear PCM output
 * must not need any codec library. The calls that do need one (a compressed
 * audio converter, opening a container file) must either work or fail with
 * their documented error status, never take the process down.
 *
 * Exit status is 0 when every check passed.
 *
 * Copyright (C) 2026 Lubos Dolezel
 */
#include <AudioToolbox/AudioToolbox.h>
#include <CoreFoundation/CoreFoundation.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static int failures;

#define CHECK(cond, ...) do { \
	if (cond) printf("PASS %s\n", #cond); \
	else { printf("FAIL %s: ", #cond); printf(__VA_ARGS__); printf("\n"); failures++; } \
} while (0)

#define RATE 44100
#define BUFFER_FRAMES 4410

struct player {
	long frame;
	long total;
	volatile int finished;
};

static void callback(void *ud, AudioQueueRef q, AudioQueueBufferRef buf)
{
	struct player *p = ud;
	short *out = buf->mAudioData;
	UInt32 n = 0;

	if (p->finished)
		return;
	while (n < BUFFER_FRAMES && p->frame < p->total) {
		double amp = 0.8 * (double)p->frame / (double)p->total;
		out[n++] = (short)(amp * 32767.0 * sin(2.0 * M_PI * 440.0 * p->frame / RATE));
		p->frame++;
	}
	buf->mAudioDataByteSize = n * sizeof(short);
	if (n)
		AudioQueueEnqueueBuffer(q, buf, 0, NULL);
	if (p->frame >= p->total)
		p->finished = 1;
}

static void check_codec_calls(void)
{
	AudioStreamBasicDescription pcm = {0}, aac = {0};
	AudioConverterRef conv = NULL;
	OSStatus st;

	pcm.mSampleRate = RATE;
	pcm.mFormatID = kAudioFormatLinearPCM;
	pcm.mFormatFlags = kAudioFormatFlagIsSignedInteger | kAudioFormatFlagIsPacked;
	pcm.mBytesPerPacket = pcm.mBytesPerFrame = 4;
	pcm.mFramesPerPacket = 1;
	pcm.mChannelsPerFrame = 2;
	pcm.mBitsPerChannel = 16;

	aac.mSampleRate = RATE;
	aac.mFormatID = kAudioFormatMPEG4AAC;
	aac.mFramesPerPacket = 1024;
	aac.mChannelsPerFrame = 2;

	/* Compressing needs a codec: it works or reports the format as unsupported */
	st = AudioConverterNew(&pcm, &aac, &conv);
	printf("AudioConverterNew(PCM -> AAC) = %d\n", (int)st);
	CHECK(st == noErr || st == kAudioConverterErr_FormatNotSupported || st == kAudioConverterErr_RequiresPacketDescriptionsError,
		"unexpected status %d", (int)st);
	if (st == noErr)
		AudioConverterDispose(conv);

	/* Opening a container file likewise works or reports the file type as unsupported */
	static const char path[] = "audioqueue_tone.wav";
	unsigned char wav[48] = { 'R','I','F','F', 40,0,0,0, 'W','A','V','E', 'f','m','t',' ', 16,0,0,0,
		1,0, 1,0, 0x44,0xac,0,0, 0x88,0x58,1,0, 2,0, 16,0, 'd','a','t','a', 4,0,0,0 };
	char full[1024];
	snprintf(full, sizeof full, "/tmp/%s", path);
	FILE *f = fopen(full, "wb");
	if (f) {
		fwrite(wav, 1, sizeof wav, f);
		fwrite("\0\0\0\0", 1, 4, f);
		fclose(f);
	}
	CFURLRef url = CFURLCreateFromFileSystemRepresentation(NULL, (const UInt8 *)full, strlen(full), false);
	AudioFileID file = NULL;
	st = AudioFileOpenURL(url, kAudioFileReadPermission, 0, &file);
	printf("AudioFileOpenURL(WAV) = %d\n", (int)st);
	CHECK(st == noErr || st == kAudioFileUnsupportedFileTypeError || st == kAudioFileInvalidFileError,
		"unexpected status %d", (int)st);
	if (st == noErr)
		AudioFileClose(file);
	CFRelease(url);
	unlink(full);
}

int main(int argc, char **argv)
{
	double seconds = argc > 1 ? atof(argv[1]) : 3.0;
	if (seconds < 0.5) seconds = 0.5;

	AudioStreamBasicDescription fmt = {0};
	fmt.mSampleRate = RATE;
	fmt.mFormatID = kAudioFormatLinearPCM;
	fmt.mFormatFlags = kAudioFormatFlagIsSignedInteger | kAudioFormatFlagIsPacked;
	fmt.mBytesPerPacket = fmt.mBytesPerFrame = 2;
	fmt.mFramesPerPacket = 1;
	fmt.mChannelsPerFrame = 1;
	fmt.mBitsPerChannel = 16;

	struct player p = { 0, (long)(seconds * RATE), 0 };
	AudioQueueRef q = NULL;
	OSStatus st = AudioQueueNewOutput(&fmt, callback, &p, NULL, NULL, 0, &q);
	CHECK(st == noErr, "AudioQueueNewOutput = %d", (int)st);
	if (st != noErr) {
		printf("RESULT FAIL\n");
		return 1;
	}

	for (int i = 0; i < 3; i++) {
		AudioQueueBufferRef buf;
		st = AudioQueueAllocateBuffer(q, BUFFER_FRAMES * sizeof(short), &buf);
		CHECK(st == noErr, "AudioQueueAllocateBuffer = %d", (int)st);
		callback(&p, q, buf);
	}
	st = AudioQueueStart(q, NULL);
	CHECK(st == noErr, "AudioQueueStart = %d", (int)st);

	for (int i = 0; i < (int)(seconds * 10) + 100 && !p.finished; i++)
		usleep(100000);
	CHECK(p.finished, "the tone was not consumed");
	st = AudioQueueStop(q, false);
	CHECK(st == noErr, "AudioQueueStop = %d", (int)st);
	AudioQueueDispose(q, true);

	check_codec_calls();

	printf("RESULT %s\n", failures ? "FAIL" : "PASS");
	return failures != 0;
}
