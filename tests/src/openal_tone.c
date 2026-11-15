/*
 * OpenAL framework: API contract checks and an audible tone.
 *
 * Checks the framework's OpenAL 1.1 behaviour and Apple's extensions as
 * documented in the framework headers (alGetProcAddress entry points,
 * AL_EXT_STATIC_BUFFER, the alut loaders), then plays a 440 Hz tone whose
 * amplitude ramps linearly from 0 to 0.8 over the given number of seconds
 * (default 3), so that a capture of the output shows where each part of the
 * buffer landed. Exit status is 0 when every check passed.
 *
 * Copyright (C) 2026 Lubos Dolezel
 */
#include <OpenAL/al.h>
#include <OpenAL/alc.h>
#include <OpenAL/alut.h>
#include <OpenAL/MacOSX_OALExtensions.h>
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

static void put32(unsigned char *p, unsigned v) { p[0]=v; p[1]=v>>8; p[2]=v>>16; p[3]=v>>24; }
static void put16(unsigned char *p, unsigned v) { p[0]=v; p[1]=v>>8; }

/* A RIFF/WAVE image with 16 bit PCM */
static unsigned char *make_wav(unsigned channels, unsigned rate, unsigned nbytes, size_t *outlen)
{
	size_t len = 44 + nbytes;
	unsigned char *w = calloc(1, len);
	memcpy(w, "RIFF", 4); put32(w+4, (unsigned)len - 8); memcpy(w+8, "WAVEfmt ", 8);
	put32(w+16, 16); put16(w+20, 1); put16(w+22, channels); put32(w+24, rate);
	put32(w+28, rate*channels*2); put16(w+32, channels*2); put16(w+34, 16);
	memcpy(w+36, "data", 4); put32(w+40, nbytes);
	for (unsigned i = 0; i < nbytes; i++) w[44+i] = (unsigned char)i;
	*outlen = len;
	return w;
}

int main(int argc, char **argv)
{
	double seconds = argc > 1 ? atof(argv[1]) : 3.0;
	if (seconds < 0.5) seconds = 0.5;

	/* Without a device, ALC reports ALC_INVALID_DEVICE for a context */
	CHECK(alcCreateContext(NULL, NULL) == NULL, "context created without a device");
	CHECK(alcGetError(NULL) == ALC_INVALID_DEVICE, "no ALC_INVALID_DEVICE");

	ALCdevice *dev = alcOpenDevice(NULL);
	CHECK(dev != NULL, "alcOpenDevice(NULL) failed");
	if (!dev) return 1;
	ALCcontext *ctx = alcCreateContext(dev, NULL);
	CHECK(ctx != NULL, "alcCreateContext failed");
	CHECK(alcMakeContextCurrent(ctx) == ALC_TRUE, "alcMakeContextCurrent failed");
	CHECK(alcGetCurrentContext() == ctx, "current context differs");
	CHECK(alcGetContextsDevice(ctx) == dev, "context device differs");

	/* Identification strings */
	const char *ver = alGetString(AL_VERSION);
	CHECK(ver && strncmp(ver, "1.1", 3) == 0, "AL_VERSION=%s", ver ? ver : "(null)");
	CHECK(alGetString(AL_RENDERER) != NULL, "no AL_RENDERER");
	CHECK(alGetString(AL_VENDOR) != NULL, "no AL_VENDOR");
	CHECK(alcGetString(dev, ALC_DEVICE_SPECIFIER) != NULL, "no ALC_DEVICE_SPECIFIER");

	/* Documented extensions */
	CHECK(alcIsExtensionPresent(dev, "ALC_EXT_MAC_OSX"), "ALC_EXT_MAC_OSX missing");
	CHECK(alcIsExtensionPresent(dev, "ALC_EXT_CAPTURE"), "ALC_EXT_CAPTURE missing");
	CHECK(alIsExtensionPresent("AL_EXT_STATIC_BUFFER"), "AL_EXT_STATIC_BUFFER missing");
	CHECK(alIsExtensionPresent("AL_EXT_FLOAT32"), "AL_EXT_FLOAT32 missing");
	CHECK(!alIsExtensionPresent("AL_EXT_no_such_extension"), "bogus extension present");

	/* Apple's entry points are reached through the proc address functions */
	static const char *const alcprocs[] = {
		"alcMacOSXRenderingQuality", "alcMacOSXGetRenderingQuality",
		"alMacOSXRenderChannelCount", "alMacOSXGetRenderChannelCount",
		"alcMacOSXMixerMaxiumumBusses", "alcMacOSXGetMixerMaxiumumBusses",
		"alcMacOSXMixerOutputRate", "alcMacOSXGetMixerOutputRate",
	};
	for (unsigned i = 0; i < sizeof alcprocs/sizeof *alcprocs; i++)
		CHECK(alcGetProcAddress(NULL, alcprocs[i]) != NULL, "%s not found", alcprocs[i]);
	CHECK(alGetProcAddress("alBufferDataStatic") != NULL, "alBufferDataStatic not found");
	CHECK(alGetProcAddress("alNoSuchFunction") == NULL, "bogus function found");

	alcMacOSXGetMixerOutputRateProcPtr getrate =
		(alcMacOSXGetMixerOutputRateProcPtr)alcGetProcAddress(NULL, "alcMacOSXGetMixerOutputRate");
	CHECK(getrate && getrate() > 8000.0, "mixer rate %f", getrate ? getrate() : -1.0);
	alcMacOSXGetMixerMaxiumumBussesProcPtr getbus =
		(alcMacOSXGetMixerMaxiumumBussesProcPtr)alcGetProcAddress(NULL, "alcMacOSXGetMixerMaxiumumBusses");
	alcMacOSXMixerMaxiumumBussesProcPtr setbus =
		(alcMacOSXMixerMaxiumumBussesProcPtr)alcGetProcAddress(NULL, "alcMacOSXMixerMaxiumumBusses");
	setbus(32);
	CHECK(getbus() == 32, "bus count %d", getbus());
	setbus(64);
	alcMacOSXGetRenderingQualityProcPtr getq =
		(alcMacOSXGetRenderingQualityProcPtr)alcGetProcAddress(NULL, "alcMacOSXGetRenderingQuality");
	alcMacOSXRenderingQualityProcPtr setq =
		(alcMacOSXRenderingQualityProcPtr)alcGetProcAddress(NULL, "alcMacOSXRenderingQuality");
	setq(ALC_MAC_OSX_SPATIAL_RENDERING_QUALITY_LOW);
	CHECK(getq() == ALC_MAC_OSX_SPATIAL_RENDERING_QUALITY_LOW, "rendering quality 0x%x", getq());
	setq(ALC_MAC_OSX_SPATIAL_RENDERING_QUALITY_HIGH);

	/* Errors */
	alGetError();
	ALuint bad = 0;
	alBufferData(12345, AL_FORMAT_MONO16, "", 0, 44100);
	CHECK(alGetError() == AL_INVALID_NAME, "alBufferData on a bad name");
	ALuint buf;
	alGenBuffers(1, &buf);
	CHECK(alGetError() == AL_NO_ERROR, "alGenBuffers failed");
	CHECK(alIsBuffer(buf), "alIsBuffer false");
	alBufferData(buf, 0x1234, "xxxx", 4, 44100);
	CHECK(alGetError() == AL_INVALID_ENUM, "alBufferData with a bad format");
	alSourcei(bad, AL_LOOPING, 1);
	CHECK(alGetError() == AL_INVALID_NAME, "alSourcei on a bad name");

	/* AL_EXT_STATIC_BUFFER: contents and size match what was passed */
	short pcm[400];
	for (int i = 0; i < 400; i++) pcm[i] = (short)(i * 50);
	alBufferDataStaticProcPtr bufferDataStatic =
		(alBufferDataStaticProcPtr)alGetProcAddress("alBufferDataStatic");
	bufferDataStatic((ALint)buf, AL_FORMAT_MONO16, pcm, sizeof pcm, 22050);
	CHECK(alGetError() == AL_NO_ERROR, "alBufferDataStatic failed");
	ALint v = 0;
	alGetBufferi(buf, AL_SIZE, &v);
	CHECK(v == (int)sizeof pcm, "AL_SIZE %d", v);
	alGetBufferi(buf, AL_FREQUENCY, &v);
	CHECK(v == 22050, "AL_FREQUENCY %d", v);
	alGetBufferi(buf, AL_BITS, &v);
	CHECK(v == 16, "AL_BITS %d", v);
	alGetBufferi(buf, AL_CHANNELS, &v);
	CHECK(v == 1, "AL_CHANNELS %d", v);

	/* alut loaders */
	size_t wavlen;
	unsigned char *wav = make_wav(2, 32000, 64, &wavlen);
	ALenum fmt = 0; ALvoid *data = NULL; ALsizei size = 0, freq = 0;
	alutLoadWAVMemory((ALbyte *)wav, &fmt, &data, &size, &freq);
	CHECK(data != NULL && fmt == AL_FORMAT_STEREO16 && size == 64 && freq == 32000,
	      "fmt=0x%x size=%d freq=%d", fmt, size, freq);
	CHECK(data && memcmp(data, wav + 44, 64) == 0, "sample data differs");
	alutUnloadWAV(fmt, data, size, freq);
	alutLoadWAVMemory((ALbyte *)"RIFFxxxxnotawave", &fmt, &data, &size, &freq);
	CHECK(data == NULL && size == 0, "garbage accepted");
	free(wav);

	/* The tone */
	const int rate = 44100;
	int n = (int)(seconds * rate);
	short *tone = malloc(n * sizeof *tone);
	for (int i = 0; i < n; i++)
		tone[i] = (short)(32767.0 * 0.8 * ((double)i / n) * sin(2*M_PI*440.0*i/rate));
	ALuint tbuf, src;
	alGenBuffers(1, &tbuf);
	alBufferData(tbuf, AL_FORMAT_MONO16, tone, n * (int)sizeof *tone, rate);
	CHECK(alGetError() == AL_NO_ERROR, "tone alBufferData failed");
	alGenSources(1, &src);
	alSourcei(src, AL_BUFFER, (ALint)tbuf);
	alSourcef(src, AL_GAIN, 1.0f);
	CHECK(alGetError() == AL_NO_ERROR, "source setup failed");
	alGetSourcei(src, AL_SOURCE_STATE, &v);
	CHECK(v == AL_INITIAL, "initial state 0x%x", v);
	alSourcePlay(src);
	alGetSourcei(src, AL_SOURCE_STATE, &v);
	CHECK(v == AL_PLAYING, "state after play 0x%x", v);
	printf("TONE START\n");
	fflush(stdout);

	/* Poll the source: it has to start advancing and then stop by itself,
	 * within the tone's length plus a margin for the output start-up. */
	ALint offset = 0, maxoffset = 0;
	double waited = 0;
	v = AL_PLAYING;
	while (waited < seconds + 6.0) {
		usleep(100000);
		waited += 0.1;
		alGetSourcei(src, AL_SAMPLE_OFFSET, &offset);
		if (offset > maxoffset) maxoffset = offset;
		alGetSourcei(src, AL_SOURCE_STATE, &v);
		if (v != AL_PLAYING) break;
	}
	CHECK(v == AL_STOPPED, "state after the tone 0x%x (waited %.1f s)", v, waited);
	CHECK(maxoffset > rate, "offset advanced to %d of %d", maxoffset, n);
	CHECK(waited >= seconds - 0.3, "the tone ended after %.1f s, shorter than %.1f s", waited, seconds);
	printf("TONE END\n");

	alDeleteSources(1, &src);
	alDeleteBuffers(1, &tbuf);
	alDeleteBuffers(1, &buf);
	CHECK(alGetError() == AL_NO_ERROR, "cleanup error");
	alcMakeContextCurrent(NULL);
	alcDestroyContext(ctx);
	CHECK(alcCloseDevice(dev) == ALC_TRUE, "alcCloseDevice failed");
	free(tone);

	printf(failures ? "RESULT FAIL (%d)\n" : "RESULT PASS\n", failures);
	return failures != 0;
}
