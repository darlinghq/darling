// Copyright (C) 2026 Lubos Dolezel
//
// Sound Manager channels (32-bit only, where macOS has them): the error codes for a channel that
// does not exist, SndNewChannel, the order in which queued sounds and callback commands run,
// waitCmd, quietCmd, flushCmd, a full queue, queuing from inside a callback, and SndDisposeChannel
// with and without waiting for the queue.
// Observed on macOS 10.14.4 (i386): -205 (badChannel) for a NULL channel in every call; with an
// output device SndNewChannel succeeds and every check below passes, and waitCmd's duration is in
// half-milliseconds; without one SndNewChannel returns -2004 and the rest of the test is skipped.
// Build for i386 (x86_64 only prints that the test does not apply).
#include <stdio.h>

#if __LP64__

int main(void)
{
	printf("SKIP: the Sound Manager command interface is 32-bit only\n");
	return 0;
}

#else

#include <Carbon/Carbon.h>
#include <math.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>

#pragma clang diagnostic ignored "-Wdeprecated-declarations"

static int g_failures = 0;

#define CHECK(cond) do { if (!(cond)) { printf("FAIL %d: %s\n", __LINE__, #cond); g_failures++; } } while (0)

static double now_ms(void)
{
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return tv.tv_sec * 1000.0 + tv.tv_usec / 1000.0;
}

typedef struct {
	int tag;
	double time;
	SndChannelPtr chan;
} Event;

static Event g_events[256];
static int g_event_count;
static double g_start;
static pthread_mutex_t g_lock = PTHREAD_MUTEX_INITIALIZER;
static int g_chain_left;
static ExtSoundHeader* g_chain_header;

static void reset_events(void)
{
	pthread_mutex_lock(&g_lock);
	g_event_count = 0;
	g_start = now_ms();
	pthread_mutex_unlock(&g_lock);
}

static int event_count(void)
{
	pthread_mutex_lock(&g_lock);
	int n = g_event_count;
	pthread_mutex_unlock(&g_lock);
	return n;
}

static void callback(SndChannelPtr chan, SndCommand* cmd)
{
	pthread_mutex_lock(&g_lock);
	if (g_event_count < 256) {
		g_events[g_event_count].tag = cmd->param2;
		g_events[g_event_count].time = now_ms() - g_start;
		g_events[g_event_count].chan = chan;
		g_event_count++;
	}
	pthread_mutex_unlock(&g_lock);
	CHECK(cmd->cmd == callBackCmd);
}

static void chain_callback(SndChannelPtr chan, SndCommand* cmd)
{
	callback(chan, cmd);
	if (g_chain_left > 0) {
		g_chain_left--;
		SndCommand next = { bufferCmd, 0, (SInt32) (intptr_t) g_chain_header };
		CHECK(SndDoCommand(chan, &next, 0) == noErr);
		SndCommand again = { callBackCmd, 0, 100 + g_chain_left };
		CHECK(SndDoCommand(chan, &again, 0) == noErr);
	}
}

static ExtSoundHeader* make_tone(int frames, int rate, int channels, int bits)
{
	size_t bytes = (size_t) frames * channels * (bits / 8);
	ExtSoundHeader* h = calloc(1, sizeof(ExtSoundHeader) + bytes);
	h->numChannels = channels;
	h->sampleRate = (UnsignedFixed) rate << 16;
	h->encode = extSH;
	h->baseFrequency = 60;
	h->numFrames = frames;
	h->sampleSize = bits;
	uint8_t* d = (uint8_t*) &h->sampleArea[0];
	for (int i = 0; i < frames; i++) {
		for (int c = 0; c < channels; c++) {
			double v = sin(i * 2 * M_PI * 440.0 / rate) * 0.2;
			if (bits == 16) {
				int16_t s = (int16_t) (v * 32767);
				d[(i * channels + c) * 2] = (uint8_t) (s >> 8);
				d[(i * channels + c) * 2 + 1] = (uint8_t) (s & 0xff);
			} else {
				d[i * channels + c] = (uint8_t) (128 + v * 127);
			}
		}
	}
	return h;
}

static OSErr do_cmd(SndChannelPtr c, int cmd, int param1, long param2, int no_wait)
{
	SndCommand s = { (unsigned short) cmd, (short) param1, (SInt32) param2 };
	return SndDoCommand(c, &s, no_wait);
}

static OSErr do_imm(SndChannelPtr c, int cmd, int param1, long param2)
{
	SndCommand s = { (unsigned short) cmd, (short) param1, (SInt32) param2 };
	return SndDoImmediate(c, &s);
}

static OSErr play(SndChannelPtr c, ExtSoundHeader* h)
{
	return do_cmd(c, bufferCmd, 0, (long) (intptr_t) h, 0);
}

static void wait_ms(int ms)
{
	usleep(ms * 1000);
}

// Waits for `count` events, at most `limit` ms.
static void wait_events(int count, int limit)
{
	for (int waited = 0; waited < limit && event_count() < count; waited += 5)
		wait_ms(5);
}

static int tag_at(int i)
{
	pthread_mutex_lock(&g_lock);
	int tag = i < g_event_count ? g_events[i].tag : -1;
	pthread_mutex_unlock(&g_lock);
	return tag;
}

static double time_at(int i)
{
	pthread_mutex_lock(&g_lock);
	double t = i < g_event_count ? g_events[i].time : -1;
	pthread_mutex_unlock(&g_lock);
	return t;
}

int main(void)
{
	// Calls on a channel that does not exist.
	SndCommand any = { callBackCmd, 0, 1 };
	SCStatus status;
	CHECK(SndDoCommand(NULL, &any, 0) == badChannel);
	CHECK(SndDoImmediate(NULL, &any) == badChannel);
	CHECK(SndDisposeChannel(NULL, 1) == badChannel);
	CHECK(SndChannelStatus(NULL, sizeof(status), &status) == badChannel);

	SndChannelPtr chan = NULL;
	OSErr err = SndNewChannel(&chan, sampledSynth, initStereo, callback);
	if (err == -2004) {
		CHECK(chan == NULL);
		printf("SKIP: no audio output device; the checks that need a channel were not run\n");
		printf(g_failures ? "%d failure(s)\n" : "PASS\n", g_failures);
		return g_failures != 0;
	}
	CHECK(err == noErr);
	CHECK(chan != NULL);
	if (!chan)
		return 1;
	CHECK(chan->callBack == callback);
	CHECK(chan->qLength == stdQLength);

	// A channel the caller allocated is used as it is.
	{
		SndChannelPtr own = calloc(1, sizeof(SndChannel));
		SndChannelPtr keep = own;
		own->qLength = stdQLength;
		CHECK(SndNewChannel(&own, sampledSynth, initMono, callback) == noErr);
		CHECK(own == keep);
		CHECK(SndDisposeChannel(own, 1) == noErr);
		free(keep);
	}

	ExtSoundHeader* ms100 = make_tone(4410, 44100, 2, 16);
	ExtSoundHeader* ms600 = make_tone(26460, 44100, 2, 16);

	// Callback commands wait for the sound in front of them, in queue order.
	reset_events();
	CHECK(play(chan, ms100) == noErr);
	CHECK(do_cmd(chan, callBackCmd, 0, 1, 0) == noErr);
	CHECK(play(chan, ms100) == noErr);
	CHECK(do_cmd(chan, callBackCmd, 0, 2, 0) == noErr);
	CHECK(do_cmd(chan, callBackCmd, 0, 3, 0) == noErr);
	CHECK(SndChannelStatus(chan, sizeof(status), &status) == noErr);
	CHECK(status.scChannelBusy);
	wait_events(3, 1500);
	CHECK(event_count() == 3);
	CHECK(tag_at(0) == 1 && tag_at(1) == 2 && tag_at(2) == 3);
	CHECK(time_at(0) >= 60 && time_at(0) < 400);
	CHECK(time_at(1) >= 160 && time_at(1) < 600);
	CHECK(tag_at(0) == 1 && time_at(2) >= time_at(1));
	CHECK(g_events[0].chan == chan);
	wait_ms(50);
	CHECK(SndChannelStatus(chan, sizeof(status), &status) == noErr);
	CHECK(!status.scChannelBusy);

	// On an idle channel callbacks come at once and in order.
	reset_events();
	CHECK(do_cmd(chan, callBackCmd, 0, 11, 0) == noErr);
	CHECK(do_cmd(chan, callBackCmd, 0, 12, 0) == noErr);
	wait_events(2, 500);
	CHECK(event_count() == 2 && tag_at(0) == 11 && tag_at(1) == 12);
	CHECK(time_at(1) < 250);

	// waitCmd holds back what follows; its parameter is in half-milliseconds (macOS 10.14.4
	// measured 76 ms for 150 and 151 ms for 300).
	reset_events();
	CHECK(do_cmd(chan, waitCmd, 300, 0, 0) == noErr);
	CHECK(do_cmd(chan, callBackCmd, 0, 21, 0) == noErr);
	wait_events(1, 1000);
	CHECK(event_count() == 1 && tag_at(0) == 21);
	CHECK(time_at(0) >= 110 && time_at(0) < 250);

	// quietCmd stops the sound that is playing; what is queued behind it still runs.
	reset_events();
	CHECK(play(chan, ms600) == noErr);
	CHECK(do_cmd(chan, callBackCmd, 0, 31, 0) == noErr);
	CHECK(do_cmd(chan, callBackCmd, 0, 32, 0) == noErr);
	wait_ms(100);
	CHECK(do_imm(chan, quietCmd, 0, 0) == noErr);
	wait_events(2, 1000);
	CHECK(event_count() == 2 && tag_at(0) == 31 && tag_at(1) == 32);
	CHECK(time_at(0) < 450);

	// flushCmd drops what is queued; the sound that is playing goes on until quietCmd.
	reset_events();
	CHECK(play(chan, ms600) == noErr);
	CHECK(do_cmd(chan, callBackCmd, 0, 41, 0) == noErr);
	CHECK(do_cmd(chan, callBackCmd, 0, 42, 0) == noErr);
	wait_ms(100);
	CHECK(do_imm(chan, flushCmd, 0, 0) == noErr);
	wait_ms(700);
	CHECK(event_count() == 0);
	CHECK(do_imm(chan, quietCmd, 0, 0) == noErr);
	wait_ms(50);
	CHECK(SndChannelStatus(chan, sizeof(status), &status) == noErr);
	CHECK(!status.scChannelBusy);

	// A queue of stdQLength commands is full: with noWait the call fails with queueFull.
	reset_events();
	CHECK(play(chan, ms600) == noErr);
	int accepted = 0;
	OSErr last = noErr;
	for (int i = 0; i < 300; i++) {
		last = do_cmd(chan, callBackCmd, 0, 1000 + i, 1);
		if (last != noErr)
			break;
		accepted++;
	}
	CHECK(last == queueFull);
	CHECK(accepted > 0 && accepted <= stdQLength);
	CHECK(do_imm(chan, flushCmd, 0, 0) == noErr);
	CHECK(do_imm(chan, quietCmd, 0, 0) == noErr);
	wait_ms(100);

	// Queuing more from inside a callback.
	SndChannelPtr chained = NULL;
	CHECK(SndNewChannel(&chained, sampledSynth, initStereo, chain_callback) == noErr);
	g_chain_header = ms100;
	g_chain_left = 3;
	reset_events();
	CHECK(play(chained, ms100) == noErr);
	CHECK(do_cmd(chained, callBackCmd, 0, 90, 0) == noErr);
	wait_events(4, 2000);
	CHECK(event_count() == 4);
	CHECK(tag_at(0) == 90 && tag_at(1) == 102 && tag_at(2) == 101 && tag_at(3) == 100);
	CHECK(SndDisposeChannel(chained, 1) == noErr);

	// SndDisposeChannel(quietNow = false) lets the queue run out, callbacks included;
	// quietNow = true drops it, and no callback comes after the call returns.
	SndChannelPtr waiting = NULL;
	CHECK(SndNewChannel(&waiting, sampledSynth, initStereo, callback) == noErr);
	reset_events();
	CHECK(play(waiting, ms100) == noErr);
	CHECK(do_cmd(waiting, callBackCmd, 0, 201, 0) == noErr);
	CHECK(SndDisposeChannel(waiting, 0) == noErr);
	CHECK(event_count() == 1 && tag_at(0) == 201);
	CHECK(SndDoCommand(waiting, &any, 0) == badChannel);

	SndChannelPtr quiet = NULL;
	CHECK(SndNewChannel(&quiet, sampledSynth, initStereo, callback) == noErr);
	reset_events();
	CHECK(play(quiet, ms600) == noErr);
	CHECK(do_cmd(quiet, callBackCmd, 0, 202, 0) == noErr);
	wait_ms(50);
	double before = now_ms();
	CHECK(SndDisposeChannel(quiet, 1) == noErr);
	CHECK(now_ms() - before < 400);
	wait_ms(700);
	CHECK(event_count() == 0);

	CHECK(SndDisposeChannel(chan, 1) == noErr);
	CHECK(SndDisposeChannel(chan, 1) == badChannel);

	printf(g_failures ? "%d failure(s)\n" : "PASS\n", g_failures);
	return g_failures != 0;
}

#endif
