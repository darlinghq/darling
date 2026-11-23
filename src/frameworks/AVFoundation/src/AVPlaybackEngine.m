/*
 This file is part of Darling.

 Copyright (C) 2026 Darling Developers

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

#include "AVPlaybackEngine.h"

#include <AudioToolbox/AudioToolbox.h>
#include <CoreMedia/CoreMedia.h>
#include <VideoToolbox/VideoToolbox.h>
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
#include <libavutil/mathematics.h>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

// The FFmpeg libraries are optional host libraries (see CoreAudio/FFmpegAvailability.h): check
// before calling into them.
extern int darling_avformat_available(void);
extern int darling_avcodec_available(void);
extern int darling_avutil_available(void);

#define IO_BUFFER_SIZE 65536
#define US_PER_SEC 1000000LL
#define SEEK_REORDER_MARGIN_US US_PER_SEC

#pragma mark - Media

struct AVPEMedia {
	AVFormatContext *format;
	AVIOContext *io;
	int fd;
	int64_t fileSize;
	int videoIndex;
	int audioIndex;
	int64_t durationUs;
	int64_t startUs; // media time zero in the container's own timeline
	int trackCount;
	AVPETrackInfo *tracks;
};

// Reads through the guest file descriptor, so any file the guest can open (inside the prefix or
// under /Volumes/SystemRoot) is readable without translating its path for the host.
static int media_read(void *opaque, uint8_t *buffer, int size)
{
	AVPEMedia *media = opaque;
	ssize_t n;
	do {
		n = read(media->fd, buffer, (size_t)size);
	} while (n < 0 && errno == EINTR);
	if (n < 0)
		return AVERROR(errno);
	return n == 0 ? AVERROR_EOF : (int)n;
}

static int64_t media_seek(void *opaque, int64_t offset, int whence)
{
	AVPEMedia *media = opaque;
	if (whence & AVSEEK_SIZE)
		return media->fileSize;
	whence &= ~AVSEEK_FORCE;
	if (whence != SEEK_SET && whence != SEEK_CUR && whence != SEEK_END)
		return AVERROR(EINVAL);
	off_t pos = lseek(media->fd, (off_t)offset, whence);
	return pos < 0 ? AVERROR(errno) : (int64_t)pos;
}

static int64_t stream_start_us(const AVStream *stream)
{
	if (stream->start_time == AV_NOPTS_VALUE)
		return 0;
	return av_rescale_q(stream->start_time, stream->time_base, (AVRational){1, US_PER_SEC});
}

// VideoToolbox's decoders, by codec.
static CMVideoCodecType video_codec_type(enum AVCodecID codec)
{
	switch (codec) {
		case AV_CODEC_ID_H264: return kCMVideoCodecType_H264;
		case AV_CODEC_ID_HEVC: return kCMVideoCodecType_HEVC;
		case AV_CODEC_ID_VP9: return kCMVideoCodecType_VP9;
		case AV_CODEC_ID_AV1: return kCMVideoCodecType_AV1;
		case AV_CODEC_ID_MPEG4: return kCMVideoCodecType_MPEG4Video;
		case AV_CODEC_ID_MPEG2VIDEO: return kCMVideoCodecType_MPEG2Video;
		case AV_CODEC_ID_MPEG1VIDEO: return kCMVideoCodecType_MPEG1Video;
		case AV_CODEC_ID_H263: return kCMVideoCodecType_H263;
		case AV_CODEC_ID_MJPEG: return kCMVideoCodecType_JPEG;
		case AV_CODEC_ID_PRORES: return kCMVideoCodecType_AppleProRes422;
		default: return 0;
	}
}

#pragma mark - Audio formats

static int codec_channels(const AVCodecParameters *par)
{
#if LIBAVCODEC_VERSION_MAJOR >= 60
	return par->ch_layout.nb_channels;
#else
	return par->channels;
#endif
}

// The AudioToolbox description of an audio stream's compressed or PCM samples, for AudioConverter.
// False for codecs that have no Core Audio format.
static bool audio_source_format(const AVCodecParameters *par, AudioStreamBasicDescription *asbd)
{
	memset(asbd, 0, sizeof(*asbd));
	int channels = codec_channels(par);
	if (par->sample_rate <= 0 || channels <= 0 || channels > 8)
		return false;
	asbd->mSampleRate = par->sample_rate;
	asbd->mChannelsPerFrame = (UInt32)channels;

	UInt32 pcmBits = 0, pcmFlags = 0;
	switch (par->codec_id) {
		case AV_CODEC_ID_AAC: asbd->mFormatID = kAudioFormatMPEG4AAC; asbd->mFramesPerPacket = 1024; return true;
		case AV_CODEC_ID_MP3: asbd->mFormatID = kAudioFormatMPEGLayer3; asbd->mFramesPerPacket = 1152; return true;
		case AV_CODEC_ID_MP2: asbd->mFormatID = kAudioFormatMPEGLayer2; asbd->mFramesPerPacket = 1152; return true;
		case AV_CODEC_ID_AC3: asbd->mFormatID = kAudioFormatAC3; asbd->mFramesPerPacket = 1536; return true;
		case AV_CODEC_ID_EAC3: asbd->mFormatID = kAudioFormatEnhancedAC3; asbd->mFramesPerPacket = 1536; return true;
		case AV_CODEC_ID_OPUS: asbd->mFormatID = kAudioFormatOpus; asbd->mFramesPerPacket = 960; return true;
		case AV_CODEC_ID_FLAC: asbd->mFormatID = kAudioFormatFLAC; asbd->mFramesPerPacket = par->frame_size > 0 ? (UInt32)par->frame_size : 0; return true;
		case AV_CODEC_ID_ALAC: asbd->mFormatID = kAudioFormatAppleLossless; asbd->mFramesPerPacket = 4096; return true;
		case AV_CODEC_ID_PCM_U8: pcmBits = 8; break;
		case AV_CODEC_ID_PCM_S16LE: pcmBits = 16; pcmFlags = kAudioFormatFlagIsSignedInteger; break;
		case AV_CODEC_ID_PCM_S16BE: pcmBits = 16; pcmFlags = kAudioFormatFlagIsSignedInteger | kAudioFormatFlagIsBigEndian; break;
		case AV_CODEC_ID_PCM_S24LE: pcmBits = 24; pcmFlags = kAudioFormatFlagIsSignedInteger; break;
		case AV_CODEC_ID_PCM_S24BE: pcmBits = 24; pcmFlags = kAudioFormatFlagIsSignedInteger | kAudioFormatFlagIsBigEndian; break;
		case AV_CODEC_ID_PCM_S32LE: pcmBits = 32; pcmFlags = kAudioFormatFlagIsSignedInteger; break;
		case AV_CODEC_ID_PCM_S32BE: pcmBits = 32; pcmFlags = kAudioFormatFlagIsSignedInteger | kAudioFormatFlagIsBigEndian; break;
		case AV_CODEC_ID_PCM_F32LE: pcmBits = 32; pcmFlags = kAudioFormatFlagIsFloat; break;
		case AV_CODEC_ID_PCM_F32BE: pcmBits = 32; pcmFlags = kAudioFormatFlagIsFloat | kAudioFormatFlagIsBigEndian; break;
		default: return false;
	}
	asbd->mFormatID = kAudioFormatLinearPCM;
	asbd->mFormatFlags = pcmFlags | kAudioFormatFlagIsPacked;
	asbd->mBitsPerChannel = pcmBits;
	asbd->mFramesPerPacket = 1;
	asbd->mBytesPerFrame = asbd->mBytesPerPacket = pcmBits / 8 * (UInt32)channels;
	return true;
}

// What is played: 32-bit float, interleaved, at the stream's own sample rate, mono or stereo.
static void audio_output_format(const AudioStreamBasicDescription *source, AudioStreamBasicDescription *out)
{
	memset(out, 0, sizeof(*out));
	out->mSampleRate = source->mSampleRate;
	out->mFormatID = kAudioFormatLinearPCM;
	out->mFormatFlags = kAudioFormatFlagIsFloat | kAudioFormatFlagIsPacked;
	out->mChannelsPerFrame = source->mChannelsPerFrame > 1 ? 2 : 1;
	out->mBitsPerChannel = 32;
	out->mFramesPerPacket = 1;
	out->mBytesPerFrame = out->mBytesPerPacket = 4 * out->mChannelsPerFrame;
}

void AVPEMediaClose(AVPEMedia *media)
{
	if (!media)
		return;
	if (media->format)
		avformat_close_input(&media->format);
	if (media->io) {
		av_freep(&media->io->buffer);
		avio_context_free(&media->io);
	}
	if (media->fd >= 0)
		close(media->fd);
	free(media->tracks);
	free(media);
}

static pthread_once_t logOnce = PTHREAD_ONCE_INIT;

// FFmpeg reports damaged or unrecognized media on stderr; an application using AVFoundation expects
// the errors in the API's results only.
static void quiet_ffmpeg_log(void)
{
	av_log_set_level(AV_LOG_QUIET);
}

AVPEMedia *AVPEMediaOpen(const char *path)
{
	if (!path || !darling_avformat_available() || !darling_avcodec_available() || !darling_avutil_available())
		return NULL;
	pthread_once(&logOnce, quiet_ffmpeg_log);

	AVPEMedia *media = calloc(1, sizeof(*media));
	if (!media)
		return NULL;
	media->fd = open(path, O_RDONLY | O_CLOEXEC);
	media->videoIndex = -1;
	media->audioIndex = -1;
	struct stat st;
	if (media->fd < 0 || fstat(media->fd, &st) != 0 || !S_ISREG(st.st_mode)) {
		AVPEMediaClose(media);
		return NULL;
	}
	media->fileSize = st.st_size;

	uint8_t *buffer = av_malloc(IO_BUFFER_SIZE);
	if (!buffer) {
		AVPEMediaClose(media);
		return NULL;
	}
	media->io = avio_alloc_context(buffer, IO_BUFFER_SIZE, 0, media, media_read, NULL, media_seek);
	if (!media->io) {
		av_free(buffer);
		AVPEMediaClose(media);
		return NULL;
	}
	media->format = avformat_alloc_context();
	if (!media->format) {
		AVPEMediaClose(media);
		return NULL;
	}
	media->format->pb = media->io;
	media->format->flags |= AVFMT_FLAG_CUSTOM_IO;

	// The file name only serves as a format hint: the data comes from the custom I/O. On failure
	// the context is freed and the pointer cleared.
	if (avformat_open_input(&media->format, path, NULL, NULL) < 0 ||
	    avformat_find_stream_info(media->format, NULL) < 0) {
		AVPEMediaClose(media);
		return NULL;
	}

	AVFormatContext *fmt = media->format;
	media->tracks = calloc(fmt->nb_streams ? fmt->nb_streams : 1, sizeof(AVPETrackInfo));
	if (!media->tracks) {
		AVPEMediaClose(media);
		return NULL;
	}

	for (unsigned i = 0; i < fmt->nb_streams; i++) {
		const AVStream *stream = fmt->streams[i];
		const AVCodecParameters *par = stream->codecpar;
		int type;
		if (par->codec_type == AVMEDIA_TYPE_VIDEO && !(stream->disposition & AV_DISPOSITION_ATTACHED_PIC))
			type = AVPETrackVideo;
		else if (par->codec_type == AVMEDIA_TYPE_AUDIO)
			type = AVPETrackAudio;
		else
			continue;

		AVPETrackInfo *track = &media->tracks[media->trackCount++];
		track->streamIndex = (int)i;
		track->trackID = (int64_t)i + 1;
		track->type = type;
		if (type == AVPETrackVideo) {
			track->width = par->width;
			track->height = par->height;
			if (stream->avg_frame_rate.den > 0 && stream->avg_frame_rate.num > 0)
				track->frameRate = (float)av_q2d(stream->avg_frame_rate);
			track->decodable = video_codec_type(par->codec_id) != 0 && par->width > 0 && par->height > 0;
			if (media->videoIndex < 0 && track->decodable) {
				media->videoIndex = (int)i;
				media->startUs = stream_start_us(stream);
			}
		}
		if (type == AVPETrackAudio) {
			track->sampleRate = par->sample_rate;
			track->channels = codec_channels(par);
			// Decodable when AudioToolbox can convert the format to PCM.
			AudioStreamBasicDescription source, output;
			AudioConverterRef converter = NULL;
			if (audio_source_format(par, &source)) {
				audio_output_format(&source, &output);
				if (AudioConverterNew(&source, &output, &converter) == noErr) {
					track->decodable = true;
					AudioConverterDispose(converter);
				}
			}
			if (media->audioIndex < 0 && track->decodable)
				media->audioIndex = (int)i;
		}
		if (stream->duration != AV_NOPTS_VALUE && stream->duration > 0)
			track->durationUs = av_rescale_q(stream->duration, stream->time_base, (AVRational){1, US_PER_SEC});
	}

	// Without video the audio stream sets media time zero.
	if (media->videoIndex < 0 && media->audioIndex >= 0)
		media->startUs = stream_start_us(fmt->streams[media->audioIndex]);

	if (fmt->duration > 0 && fmt->duration != AV_NOPTS_VALUE)
		media->durationUs = av_rescale_q(fmt->duration, (AVRational){1, AV_TIME_BASE}, (AVRational){1, US_PER_SEC});
	for (int i = 0; i < media->trackCount; i++) {
		if (media->tracks[i].durationUs > media->durationUs)
			media->durationUs = media->tracks[i].durationUs;
	}
	for (int i = 0; i < media->trackCount; i++) {
		if (media->tracks[i].durationUs <= 0)
			media->tracks[i].durationUs = media->durationUs;
	}
	return media;
}

int64_t AVPEMediaDurationUs(const AVPEMedia *media) { return media->durationUs; }
int AVPEMediaTrackCount(const AVPEMedia *media) { return media->trackCount; }
const AVPETrackInfo *AVPEMediaTrack(const AVPEMedia *media, int index)
{
	return (index >= 0 && index < media->trackCount) ? &media->tracks[index] : NULL;
}

#pragma mark - Engine

// Threads, all guarded by one lock and one condition variable (engine->lock, engine->wake):
//
//   demux  reads packets, sorts them into the video and audio queues, handles seeks and detects the end.
//   video  decodes video packets through VideoToolbox and shows each frame when the clock reaches it.
//   audio  decodes audio packets through AudioConverter and plays them through an AudioQueue.
//
// While audio plays, the playback clock follows what the AudioQueue has played (the audio is the master
// clock); between the corrections it runs on the monotonic clock. A seek is a marker pushed into each
// queue behind a flush: the workers reposition when they reach it, and everything they decode for an
// older seek is dropped (see interrupted()).

#define AUDIO_BUFFERS 8
#define AUDIO_SEGMENTS 32
#define AUDIO_DECODE_FRAMES 4096
// Decoded audio kept behind the play position, so that a rate change can restart from the clock
// position although the queue holds audio ahead of it.
#define AUDIO_HISTORY_SECONDS 2
#define CLOCK_CORRECTION_US 30000
#define AUDIO_START_TIMEOUT_US (3 * US_PER_SEC)
#define VIDEO_QUEUE_BYTES (4 << 20)
#define AUDIO_QUEUE_BYTES (1 << 20)
#define QUEUE_HARD_LIMIT_BYTES (64 << 20)
// The input callback of AudioConverter has no packet right now.
#define AUDIO_NO_DATA_YET ((OSStatus)'nodt')

typedef struct PNode {
	struct PNode *next;
	AVPacket *packet; // NULL for a seek marker
	int64_t targetUs;
	uint64_t token;
	uint64_t gen;
} PNode;

typedef struct {
	PNode *head, *tail;
	size_t count, bytes;
} PQueue;

static void pq_push(PQueue *queue, PNode *node)
{
	node->next = NULL;
	if (queue->tail)
		queue->tail->next = node;
	else
		queue->head = node;
	queue->tail = node;
	queue->count++;
	if (node->packet)
		queue->bytes += (size_t)node->packet->size;
}

static PNode *pq_pop(PQueue *queue)
{
	PNode *node = queue->head;
	if (!node)
		return NULL;
	queue->head = node->next;
	if (!queue->head)
		queue->tail = NULL;
	queue->count--;
	if (node->packet)
		queue->bytes -= (size_t)node->packet->size;
	return node;
}

static void node_free(PNode *node)
{
	if (node->packet)
		av_packet_free(&node->packet);
	free(node);
}

// Discards everything queued. The tokens of the seek markers that were dropped are appended to tokens.
static void pq_flush(PQueue *queue, uint64_t *tokens, size_t *tokenCount, size_t tokenCapacity)
{
	PNode *node;
	while ((node = pq_pop(queue))) {
		if (!node->packet && node->token != 0 && tokens && *tokenCount < tokenCapacity)
			tokens[(*tokenCount)++] = node->token;
		node_free(node);
	}
}

typedef struct {
	int64_t startFrame; // in the AudioQueue's sample time
	double mediaUs;     // media time of the buffer's first frame
	double rate;        // media time passing per time played
} AudioSegment;

typedef struct {
	AudioConverterRef converter;
	AudioQueueRef queue;
	AudioStreamBasicDescription source, output;
	UInt32 bufferFrames;
	UInt32 outBytesPerFrame;
	UInt32 outChannels;
	AudioQueueBufferRef buffers[AUDIO_BUFFERS];
	bool busy[AUDIO_BUFFERS]; // guarded by engine->lock

	// Only touched by the audio thread from here on.
	bool opened;
	bool running;       // the queue plays (started, not paused)
	uint64_t gen;
	bool owner;         // reports the seek (there is no video)
	bool prerolling;
	uint64_t prerollToken;
	bool streamDone;    // the converter has nothing more to give until the next seek
	bool inputEnded;    // the packet queue ended (the end of the media)
	bool startFailed;   // the queue cannot play: the packets are only consumed
	bool recovering;    // the queue failed to start before and the audio has to catch up
	bool needFirstPosition;
	int64_t streamFrame;     // the media frame the converter's next output frame belongs to

	AVPacket *inPacket;      // the packet the converter is working on
	size_t inOffset;         // bytes of it given (linear PCM is given in slices)
	AudioStreamPacketDescription inDescription;

	// Decoded audio at the output rate. src[0] is the media frame srcBase; srcPos is the play
	// position in it (fractional while the rate is not 1).
	float *src;
	int64_t srcCapacity, srcCount, srcBase;
	double srcPos;
	int64_t expectedFrame;   // the media frame that follows the last one in src
	float *decoded;          // scratch for one converter call

	int64_t enqueuedFrames;  // frames given to the queue since its last reset, silence included
	int64_t lastHeard;
	AudioSegment segments[AUDIO_SEGMENTS];
	int segmentCount;
	double lastRate;
	bool timePitch;          // the queue's time/pitch effect plays the audio at the rate (else it is resampled here)
	bool queueStarted;       // the queue was started and not stopped since (it may be paused)
	AVPEAudioAlgorithm appliedAlgorithm;
	double appliedQueueRate;
	uint64_t seenHoldEpoch;
	int64_t holdReference;
	float appliedVolume;
	bool volumeApplied;
	int failures;
} AudioState;

struct AVPEEngine {
	AVPEMedia *media;
	AVPECallbacks callbacks;

	pthread_t demuxThread, videoThread, audioThread;
	bool demuxStarted, videoStarted, audioStarted;

	// Everything from here to the next comment is guarded by lock.
	pthread_mutex_t lock;
	pthread_cond_t wake;
	bool exiting;
	bool playing;
	float rate;
	int64_t baseMediaUs;
	int64_t baseWallUs;
	bool needSeek;
	int64_t seekTargetUs;
	uint64_t seekToken;
	uint64_t gen;           // counts the seeks the demux thread started
	PQueue videoQueue, audioQueue;
	bool demuxEOF;
	bool endHandled;
	bool hasVideo, hasAudio;
	float volume;
	bool volumeDirty;
	// Audio is the master clock while audioActive. After playback starts (or restarts at another
	// position) the clock waits, held, until the audio is really heard.
	bool audioActive;
	bool audioHold;
	uint64_t holdEpoch;
	int64_t holdStartWallUs;
	float audioRate;        // the rate the audio is rendered at: the last one played
	AVPEAudioAlgorithm audioAlgorithm;
	bool algorithmChanged;
	bool rateChanged;
	int64_t rateChangeMediaUs;

	// Only touched by the video thread.
	VTDecompressionSessionRef session;
	CMVideoFormatDescriptionRef formatDescription;
	bool prerolling; // decoding up to the target of a seek
	int64_t prerollTargetUs;
	uint64_t prerollToken;
	uint64_t videoGen;
	bool videoEndSeen;
	CVPixelBufferRef held; // the last frame before the seek target
	int64_t heldUs;

	AudioState audio;
};

static int64_t wall_us(void)
{
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	return (int64_t)ts.tv_sec * US_PER_SEC + ts.tv_nsec / 1000;
}

// Requires the lock.
static int64_t clock_now_locked(AVPEEngine *engine)
{
	int64_t now = engine->baseMediaUs;
	if (engine->playing && !engine->audioHold)
		now += (int64_t)((double)(wall_us() - engine->baseWallUs) * engine->rate);
	int64_t duration = engine->media->durationUs;
	if (duration > 0 && now > duration)
		now = duration;
	return now < 0 ? 0 : now;
}

static void rebase_clock_locked(AVPEEngine *engine, int64_t mediaUs)
{
	engine->baseMediaUs = mediaUs;
	engine->baseWallUs = wall_us();
}

// Requires the lock. With working audio the clock stands still from the start (or restart) of playback
// until the audio is heard.
static void arm_audio_hold_locked(AVPEEngine *engine)
{
	if (!engine->audioActive || !engine->playing)
		return;
	engine->audioHold = true;
	engine->holdEpoch++;
	engine->holdStartWallUs = wall_us();
}

static void wait_locked(AVPEEngine *engine, int64_t timeoutUs)
{
	if (timeoutUs < 0) {
		pthread_cond_wait(&engine->wake, &engine->lock);
		return;
	}
	struct timespec ts;
	clock_gettime(CLOCK_REALTIME, &ts);
	int64_t ns = (int64_t)ts.tv_nsec + (timeoutUs % US_PER_SEC) * 1000;
	ts.tv_sec += timeoutUs / US_PER_SEC + ns / 1000000000LL;
	ts.tv_nsec = ns % 1000000000LL;
	pthread_cond_timedwait(&engine->wake, &engine->lock, &ts);
}

// True when the work for the seek generation gen is obsolete: playback is shutting down, a seek has
// been requested, or a seek started that the thread has not reached yet.
static bool interrupted(AVPEEngine *engine, uint64_t gen)
{
	pthread_mutex_lock(&engine->lock);
	bool result = engine->exiting || engine->needSeek || engine->gen != gen;
	pthread_mutex_unlock(&engine->lock);
	return result;
}

// Blocks until the playback clock reaches the media time, while playing. Returns false if
// playback is shutting down or a seek replaced the position (generation gen is a thread's current
// one; the demux thread passes the newest).
static bool wait_until(AVPEEngine *engine, int64_t mediaUs, const uint64_t *gen)
{
	pthread_mutex_lock(&engine->lock);
	for (;;) {
		if (engine->exiting || engine->needSeek || (gen && engine->gen != *gen)) {
			pthread_mutex_unlock(&engine->lock);
			return false;
		}
		if (!engine->playing) {
			wait_locked(engine, -1);
			continue;
		}
		int64_t now = clock_now_locked(engine);
		if (mediaUs <= now)
			break;
		int64_t remaining = engine->audioHold ? 20000 : (int64_t)((double)(mediaUs - now) / engine->rate);
		wait_locked(engine, remaining < 1000 ? 1000 : remaining);
	}
	pthread_mutex_unlock(&engine->lock);
	return true;
}

#pragma mark - Video

static void release_held(AVPEEngine *engine)
{
	if (engine->held) {
		CVPixelBufferRelease(engine->held);
		engine->held = NULL;
	}
}

static void finish_preroll(AVPEEngine *engine, bool finished)
{
	if (!engine->prerolling)
		return;
	engine->prerolling = false;
	release_held(engine);
	if (engine->prerollToken != 0 && engine->callbacks.seekDone)
		engine->callbacks.seekDone(engine->callbacks.context, engine->prerollToken, finished);
}

static void present(AVPEEngine *engine, CVPixelBufferRef pixelBuffer)
{
	if (engine->callbacks.frame)
		engine->callbacks.frame(engine->callbacks.context, pixelBuffer);
}

// Runs on the video thread, inside VTDecompressionSessionDecodeFrame or ...FinishDelayedFrames:
// frames arrive in presentation order.
static void decoded_frame(void *refCon, void *sourceFrameRefCon, OSStatus status, VTDecodeInfoFlags flags,
                          CVImageBufferRef imageBuffer, CMTime pts, CMTime duration)
{
	AVPEEngine *engine = refCon;
	if (status != noErr || !imageBuffer || !CMTIME_IS_VALID(pts))
		return;
	int64_t ptsUs = pts.timescale == US_PER_SEC ? pts.value : av_rescale(pts.value, US_PER_SEC, pts.timescale);
	if (ptsUs < 0 || interrupted(engine, engine->videoGen))
		return;

	if (engine->prerolling) {
		if (ptsUs < engine->prerollTargetUs) {
			// Not there yet: the last such frame is what the target time shows.
			release_held(engine);
			engine->held = CVPixelBufferRetain(imageBuffer);
			engine->heldUs = ptsUs;
			return;
		}
		if (engine->held && ptsUs > engine->prerollTargetUs)
			present(engine, engine->held);
		else
			present(engine, imageBuffer);
		bool showedThis = !(engine->held && ptsUs > engine->prerollTargetUs);
		finish_preroll(engine, true);
		if (showedThis)
			return;
	}

	if (wait_until(engine, ptsUs, &engine->videoGen))
		present(engine, imageBuffer);
}

static void destroy_session(AVPEEngine *engine)
{
	if (engine->session) {
		VTDecompressionSessionInvalidate(engine->session);
		CFRelease(engine->session);
		engine->session = NULL;
	}
}

static bool create_session(AVPEEngine *engine)
{
	destroy_session(engine);
	if (!engine->formatDescription)
		return false;

	CFMutableDictionaryRef attributes = CFDictionaryCreateMutable(kCFAllocatorDefault, 1,
		&kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
	int32_t format = kCVPixelFormatType_32BGRA;
	CFNumberRef number = CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt32Type, &format);
	CFDictionarySetValue(attributes, kCVPixelBufferPixelFormatTypeKey, number);
	CFRelease(number);

	VTDecompressionOutputCallbackRecord record = { decoded_frame, engine };
	OSStatus status = VTDecompressionSessionCreate(kCFAllocatorDefault, engine->formatDescription, NULL,
		attributes, &record, &engine->session);
	CFRelease(attributes);
	return status == noErr && engine->session != NULL;
}

static bool create_format_description(AVPEEngine *engine)
{
	AVPEMedia *media = engine->media;
	if (media->videoIndex < 0)
		return false;
	const AVCodecParameters *par = media->format->streams[media->videoIndex]->codecpar;
	CMVideoCodecType codec = video_codec_type(par->codec_id);

	// The decoder configuration goes in as a sample description atom, which is where
	// VideoToolbox looks for it. H.264 and HEVC streams without an avcC/hvcC configuration
	// (version byte 1) are in Annex B form, which needs none.
	const char *atom = NULL;
	switch (par->codec_id) {
		case AV_CODEC_ID_H264:
			atom = (par->extradata_size > 0 && par->extradata[0] == 1) ? "avcC" : NULL;
			break;
		case AV_CODEC_ID_HEVC:
			atom = (par->extradata_size > 0 && par->extradata[0] == 1) ? "hvcC" : NULL;
			break;
		case AV_CODEC_ID_VP9: atom = "vpcC"; break;
		case AV_CODEC_ID_AV1: atom = "av1C"; break;
		case AV_CODEC_ID_MPEG4: atom = "esds"; break;
		default: break;
	}

	CFMutableDictionaryRef extensions = NULL;
	if (atom && par->extradata_size > 0) {
		extensions = CFDictionaryCreateMutable(kCFAllocatorDefault, 1,
			&kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
		CFMutableDictionaryRef atoms = CFDictionaryCreateMutable(kCFAllocatorDefault, 1,
			&kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
		CFDataRef data = CFDataCreate(kCFAllocatorDefault, par->extradata, par->extradata_size);
		CFStringRef key = CFStringCreateWithCString(kCFAllocatorDefault, atom, kCFStringEncodingASCII);
		CFDictionarySetValue(atoms, key, data);
		CFDictionarySetValue(extensions, kCMFormatDescriptionExtension_SampleDescriptionExtensionAtoms, atoms);
		CFRelease(key);
		CFRelease(data);
		CFRelease(atoms);
	}

	OSStatus status = CMVideoFormatDescriptionCreate(kCFAllocatorDefault, codec, par->width, par->height,
		extensions, &engine->formatDescription);
	if (extensions)
		CFRelease(extensions);
	return status == noErr && engine->formatDescription != NULL;
}

static void decode_packet(AVPEEngine *engine, const AVPacket *packet)
{
	if (!engine->session || packet->size <= 0)
		return;
	AVPEMedia *media = engine->media;
	const AVStream *stream = media->format->streams[media->videoIndex];
	const AVRational micro = { 1, US_PER_SEC };

	int64_t pts = packet->pts != AV_NOPTS_VALUE ? packet->pts : packet->dts;
	if (pts == AV_NOPTS_VALUE)
		return;
	int64_t dts = packet->dts != AV_NOPTS_VALUE ? packet->dts : pts;

	CMSampleTimingInfo timing;
	timing.presentationTimeStamp = CMTimeMake(av_rescale_q(pts, stream->time_base, micro) - media->startUs, US_PER_SEC);
	timing.decodeTimeStamp = CMTimeMake(av_rescale_q(dts, stream->time_base, micro) - media->startUs, US_PER_SEC);
	timing.duration = packet->duration > 0 ? CMTimeMake(av_rescale_q(packet->duration, stream->time_base, micro), US_PER_SEC)
	                                       : kCMTimeInvalid;

	void *copy = malloc((size_t)packet->size);
	if (!copy)
		return;
	memcpy(copy, packet->data, (size_t)packet->size);

	CMBlockBufferRef block = NULL;
	OSStatus status = CMBlockBufferCreateWithMemoryBlock(kCFAllocatorDefault, copy, (size_t)packet->size,
		kCFAllocatorMalloc, NULL, 0, (size_t)packet->size, 0, &block);
	if (status != noErr || !block) {
		free(copy);
		return;
	}

	size_t size = (size_t)packet->size;
	CMSampleBufferRef sample = NULL;
	status = CMSampleBufferCreate(kCFAllocatorDefault, block, true, NULL, NULL, engine->formatDescription,
		1, 1, &timing, 1, &size, &sample);
	CFRelease(block);
	if (status != noErr || !sample)
		return;

	// Synchronous: the frames this packet completes come out through decoded_frame before it returns.
	VTDecompressionSessionDecodeFrame(engine->session, sample, 0, NULL, NULL);
	CFRelease(sample);
}

static void flush_decoder(AVPEEngine *engine)
{
	if (engine->session)
		VTDecompressionSessionFinishDelayedFrames(engine->session);
}

// The video thread reached the seek marker: frames before the target are decoded but not shown.
static void video_begin_seek(AVPEEngine *engine, const PNode *marker)
{
	finish_preroll(engine, false);
	// The decoder's reference frames and queued output belong to the old position.
	create_session(engine);
	engine->videoGen = marker->gen;
	engine->videoEndSeen = false;
	engine->prerolling = true;
	engine->prerollTargetUs = marker->targetUs;
	engine->prerollToken = marker->token;
}

static void video_reach_end(AVPEEngine *engine)
{
	flush_decoder(engine);
	if (engine->prerolling) {
		// Seeking past the last frame shows the last frame.
		if (engine->held && !interrupted(engine, engine->videoGen))
			present(engine, engine->held);
		finish_preroll(engine, true);
	}
	engine->videoEndSeen = true;
}

static void *video_main(void *arg)
{
	AVPEEngine *engine = arg;
	for (;;) {
		PNode *node = NULL;
		bool end = false;
		pthread_mutex_lock(&engine->lock);
		for (;;) {
			if (engine->exiting)
				break;
			if (engine->videoQueue.head) {
				node = pq_pop(&engine->videoQueue);
				break;
			}
			if (engine->demuxEOF && !engine->videoEndSeen) {
				end = true;
				break;
			}
			wait_locked(engine, -1);
		}
		pthread_mutex_unlock(&engine->lock);
		if (!node && !end)
			break;

		if (end) {
			video_reach_end(engine);
		} else if (!node->packet) {
			video_begin_seek(engine, node);
		} else {
			decode_packet(engine, node->packet);
		}
		if (node)
			node_free(node);
	}
	finish_preroll(engine, false);
	return NULL;
}

#pragma mark - Audio

// Linear PCM is handed over in slices of what the converter asks for; compressed audio in whole packets.
static OSStatus audio_input(AudioConverterRef converter, UInt32 *packets, AudioBufferList *data,
                            AudioStreamPacketDescription **descriptions, void *context)
{
	AVPEEngine *engine = context;
	AudioState *audio = &engine->audio;
	const bool pcm = audio->source.mFormatID == kAudioFormatLinearPCM;

	if (audio->inPacket && audio->inOffset >= (size_t)audio->inPacket->size)
		av_packet_free(&audio->inPacket);

	if (!audio->inPacket) {
		pthread_mutex_lock(&engine->lock);
		PNode *head = engine->audioQueue.head;
		bool end = false;
		PNode *node = NULL;
		if (head && head->packet)
			node = pq_pop(&engine->audioQueue);
		else if (!head)
			end = engine->demuxEOF;
		pthread_mutex_unlock(&engine->lock);

		if (!node) {
			*packets = 0;
			data->mBuffers[0].mDataByteSize = 0;
			data->mBuffers[0].mData = NULL;
			if (descriptions)
				*descriptions = NULL;
			if (end)
				audio->inputEnded = true;
			// A successful call without data is the end of the stream; a seek marker or an empty
			// queue only means that there is nothing now.
			return end ? noErr : AUDIO_NO_DATA_YET;
		}

		audio->inPacket = node->packet;
		node->packet = NULL;
		node_free(node);
		audio->inOffset = 0;

		if (audio->needFirstPosition) {
			AVPEMedia *media = engine->media;
			const AVStream *stream = media->format->streams[media->audioIndex];
			int64_t pts = audio->inPacket->pts != AV_NOPTS_VALUE ? audio->inPacket->pts : audio->inPacket->dts;
			if (pts != AV_NOPTS_VALUE) {
				int64_t us = av_rescale_q(pts, stream->time_base, (AVRational){ 1, US_PER_SEC }) - media->startUs;
				audio->streamFrame = av_rescale(us, (int64_t)audio->output.mSampleRate, US_PER_SEC);
			}
			audio->needFirstPosition = false;
		}
	}

	AVPacket *packet = audio->inPacket;
	size_t bytes = (size_t)packet->size - audio->inOffset;
	if (pcm) {
		size_t frameBytes = audio->source.mBytesPerFrame;
		size_t frames = bytes / frameBytes;
		if (frames > *packets)
			frames = *packets;
		bytes = frames * frameBytes;
		*packets = (UInt32)frames;
	} else {
		*packets = 1;
	}
	data->mNumberBuffers = 1;
	data->mBuffers[0].mNumberChannels = audio->source.mChannelsPerFrame;
	data->mBuffers[0].mDataByteSize = (UInt32)bytes;
	data->mBuffers[0].mData = packet->data + audio->inOffset;
	if (descriptions) {
		audio->inDescription.mStartOffset = 0;
		audio->inDescription.mVariableFramesInPacket = 0;
		audio->inDescription.mDataByteSize = (UInt32)bytes;
		*descriptions = pcm ? NULL : &audio->inDescription;
	}
	audio->inOffset += bytes;
	if (pcm && bytes == 0)
		audio->inOffset = (size_t)packet->size; // a fragment of a frame at the end
	return noErr;
}

static void audio_queue_buffer_done(void *context, AudioQueueRef queue, AudioQueueBufferRef buffer)
{
	AVPEEngine *engine = context;
	intptr_t index = (intptr_t)buffer->mUserData - 1;
	pthread_mutex_lock(&engine->lock);
	if (index >= 0 && index < AUDIO_BUFFERS)
		engine->audio.busy[index] = false;
	pthread_cond_broadcast(&engine->wake);
	pthread_mutex_unlock(&engine->lock);
}

// The magic cookies AudioConverter takes: AAC wants the ES descriptor around the AudioSpecificConfig
// that the container has, FLAC the "fLaC" marker and a STREAMINFO block header around the STREAMINFO.
static CFDataRef audio_magic_cookie(const AVCodecParameters *par)
{
	if (par->extradata_size <= 0)
		return NULL;
	CFMutableDataRef cookie = CFDataCreateMutable(kCFAllocatorDefault, 0);
	size_t size = (size_t)par->extradata_size;
	if (par->codec_id == AV_CODEC_ID_AAC) {
		// ES_Descriptor { ES_ID, flags, DecoderConfigDescriptor { 13 bytes, DecoderSpecificInfo } }, with the
		// 4-byte length form.
		uint8_t head[] = {
			0x03, 0x80, 0x80, 0x80, (uint8_t)(3 + 5 + 13 + 5 + size), 0, 0, 0,
			0x04, 0x80, 0x80, 0x80, (uint8_t)(13 + 5 + size), 0x40, 0x15, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0x05, 0x80, 0x80, 0x80, (uint8_t)size,
		};
		if (size > 100) {
			CFRelease(cookie);
			return NULL;
		}
		CFDataAppendBytes(cookie, head, sizeof(head));
		CFDataAppendBytes(cookie, par->extradata, (CFIndex)size);
	} else if (par->codec_id == AV_CODEC_ID_FLAC && size == 34) {
		uint8_t head[] = { 'f', 'L', 'a', 'C', 0x80, 0, 0, 34 };
		CFDataAppendBytes(cookie, head, sizeof(head));
		CFDataAppendBytes(cookie, par->extradata, (CFIndex)size);
	} else {
		CFDataAppendBytes(cookie, par->extradata, (CFIndex)size);
	}
	return cookie;
}

static bool audio_set_algorithm(AudioState *audio, AVPEAudioAlgorithm algorithm)
{
	const UInt32 value = algorithm == AVPEAudioAlgorithmTimeDomain ? kAudioQueueTimePitchAlgorithm_TimeDomain
		: algorithm == AVPEAudioAlgorithmVarispeed ? kAudioQueueTimePitchAlgorithm_Varispeed : kAudioQueueTimePitchAlgorithm_Spectral;
	return AudioQueueSetProperty(audio->queue, kAudioQueueProperty_TimePitchAlgorithm, &value, sizeof(value)) == noErr;
}

static bool audio_open(AVPEEngine *engine)
{
	AudioState *audio = &engine->audio;
	AVPEMedia *media = engine->media;
	const AVCodecParameters *par = media->format->streams[media->audioIndex]->codecpar;
	if (!audio_source_format(par, &audio->source))
		return false;
	audio_output_format(&audio->source, &audio->output);
	audio->outChannels = audio->output.mChannelsPerFrame;
	audio->outBytesPerFrame = audio->output.mBytesPerFrame;
	// About 50 ms per buffer: smaller ones were starved by the AudioQueue's callback thread on a loaded machine.
	audio->bufferFrames = ((UInt32)(audio->output.mSampleRate / 20) + 255) & ~255u;

	if (AudioConverterNew(&audio->source, &audio->output, &audio->converter) != noErr)
		return false;
	if (audio->source.mFormatID != kAudioFormatLinearPCM) {
		CFDataRef cookie = audio_magic_cookie(par);
		if (cookie) {
			OSStatus status = AudioConverterSetProperty(audio->converter, kAudioConverterDecompressionMagicCookie,
				(UInt32)CFDataGetLength(cookie), CFDataGetBytePtr(cookie));
			CFRelease(cookie);
			if (status != noErr)
				return false;
		}
	}
	if (AudioQueueNewOutput(&audio->output, audio_queue_buffer_done, engine, NULL, NULL, 0, &audio->queue) != noErr)
		return false;
	// The queue's time/pitch effect plays the audio at the rate (it must be enabled before the queue starts); a queue
	// without it gets the audio resampled to the rate here, so that the pitch follows the rate.
	{
		const UInt32 enable = 1;
		pthread_mutex_lock(&engine->lock);
		audio->appliedAlgorithm = engine->audioAlgorithm;
		engine->algorithmChanged = false;
		pthread_mutex_unlock(&engine->lock);
		if (AudioQueueSetProperty(audio->queue, kAudioQueueProperty_EnableTimePitch, &enable, sizeof(enable)) == noErr &&
		    audio_set_algorithm(audio, audio->appliedAlgorithm))
			audio->timePitch = true;
		audio->appliedQueueRate = 1;
	}
	for (int i = 0; i < AUDIO_BUFFERS; i++) {
		if (AudioQueueAllocateBuffer(audio->queue, audio->bufferFrames * audio->outBytesPerFrame, &audio->buffers[i]) != noErr)
			return false;
		audio->buffers[i]->mUserData = (void *)(intptr_t)(i + 1);
	}
	audio->decoded = malloc((size_t)AUDIO_DECODE_FRAMES * audio->outBytesPerFrame);
	return audio->decoded != NULL;
}

static void audio_close(AVPEEngine *engine)
{
	AudioState *audio = &engine->audio;
	if (audio->queue)
		AudioQueueDispose(audio->queue, true); // no buffer callback runs after this returns
	audio->queue = NULL;
	if (audio->converter)
		AudioConverterDispose(audio->converter);
	audio->converter = NULL;
	if (audio->inPacket)
		av_packet_free(&audio->inPacket);
	free(audio->src);
	free(audio->decoded);
	audio->src = audio->decoded = NULL;
}

// Where the audio queue's sample time is, in frames. False if the queue is not running.
static bool audio_heard(AudioState *audio, int64_t *frames)
{
	AudioTimeStamp stamp;
	Boolean discontinuity;
	if (!audio->queue || AudioQueueGetCurrentTime(audio->queue, NULL, &stamp, &discontinuity) != noErr ||
	    !(stamp.mFlags & kAudioTimeStampSampleTimeValid))
		return false;
	*frames = (int64_t)stamp.mSampleTime;
	return true;
}

// Media time heard at the queue sample time.
static bool audio_media_at(AudioState *audio, int64_t heard, double *mediaUs)
{
	if (audio->segmentCount == 0)
		return false;
	// The segments are in order; drop those the sample time has passed.
	while (audio->segmentCount > 1 && audio->segments[1].startFrame <= heard) {
		memmove(&audio->segments[0], &audio->segments[1], (size_t)(audio->segmentCount - 1) * sizeof(AudioSegment));
		audio->segmentCount--;
	}
	const AudioSegment *segment = &audio->segments[0];
	*mediaUs = segment->mediaUs + (double)(heard - segment->startFrame) * 1e6 / audio->output.mSampleRate * segment->rate;
	return true;
}

// Corrects the playback clock to the audio that is heard.
static void audio_sync_clock(AVPEEngine *engine)
{
	AudioState *audio = &engine->audio;
	int64_t heard;
	if (!audio->running || !audio_heard(audio, &heard))
		return;
	audio->lastHeard = heard;
	double mediaUs;
	bool haveMedia = audio_media_at(audio, heard, &mediaUs);

	bool started = false;
	pthread_mutex_lock(&engine->lock);
	if (engine->playing && engine->audioActive) {
		bool changed = false;
		if (engine->audioHold) {
			if (engine->holdEpoch != audio->seenHoldEpoch) {
				audio->seenHoldEpoch = engine->holdEpoch;
				audio->holdReference = heard;
			} else if (haveMedia && heard > audio->holdReference) {
				engine->audioHold = false;
				rebase_clock_locked(engine, (int64_t)mediaUs);
				changed = true;
				started = true;
			} else if (wall_us() - engine->holdStartWallUs > AUDIO_START_TIMEOUT_US) {
				// The audio never started (no device?): the clock runs on its own.
				engine->audioHold = false;
				engine->audioActive = false;
				rebase_clock_locked(engine, clock_now_locked(engine));
				changed = true;
				started = true;
			}
		} else if (haveMedia) {
			int64_t difference = (int64_t)mediaUs - clock_now_locked(engine);
			if (difference > CLOCK_CORRECTION_US || difference < -CLOCK_CORRECTION_US) {
				rebase_clock_locked(engine, (int64_t)mediaUs);
				changed = true;
			}
		}
		if (changed)
			pthread_cond_broadcast(&engine->wake);
	}
	pthread_mutex_unlock(&engine->lock);
	if (started && engine->callbacks.started)
		engine->callbacks.started(engine->callbacks.context);
}

static void audio_clear_output(AVPEEngine *engine)
{
	AudioState *audio = &engine->audio;
	AudioQueueReset(audio->queue); // the buffers come back through the callback before this returns
	pthread_mutex_lock(&engine->lock);
	for (int i = 0; i < AUDIO_BUFFERS; i++)
		audio->busy[i] = false;
	pthread_mutex_unlock(&engine->lock);
	audio->enqueuedFrames = 0;
	audio->lastHeard = 0;
	audio->segmentCount = 0;
	// The clock waits for the new position to be heard: its reference is read again.
	audio->seenHoldEpoch = 0;
}

static void audio_finish_preroll(AVPEEngine *engine, bool finished)
{
	AudioState *audio = &engine->audio;
	if (!audio->prerolling)
		return;
	audio->prerolling = false;
	if (audio->prerollToken != 0 && engine->callbacks.seekDone)
		engine->callbacks.seekDone(engine->callbacks.context, audio->prerollToken, finished);
}

static void audio_begin_seek(AVPEEngine *engine, const PNode *marker)
{
	AudioState *audio = &engine->audio;
	audio_finish_preroll(engine, false);
	audio->gen = marker->gen;
	if (audio->converter) {
		AudioConverterReset(audio->converter);
		audio_clear_output(engine);
	}
	if (audio->inPacket)
		av_packet_free(&audio->inPacket);
	audio->inOffset = 0;
	int64_t targetFrame = audio->converter
		? av_rescale(marker->targetUs, (int64_t)audio->output.mSampleRate, US_PER_SEC) : 0;
	audio->srcBase = targetFrame;
	audio->srcCount = 0;
	audio->srcPos = 0;
	audio->expectedFrame = targetFrame;
	audio->needFirstPosition = true;
	audio->streamFrame = targetFrame;
	audio->streamDone = false;
	audio->inputEnded = false;
	if (engine->audio.owner) {
		audio->prerolling = true;
		audio->prerollToken = marker->token;
	}
}

static bool audio_reserve(AudioState *audio, int64_t frames)
{
	int64_t needed = audio->srcCount + frames;
	if (needed <= audio->srcCapacity)
		return true;
	int64_t capacity = audio->srcCapacity ? audio->srcCapacity : 16384;
	while (capacity < needed)
		capacity *= 2;
	float *grown = realloc(audio->src, (size_t)capacity * audio->outBytesPerFrame);
	if (!grown)
		return false;
	audio->src = grown;
	audio->srcCapacity = capacity;
	return true;
}

// Adds decoded frames at the position they belong to: frames before the position that already
// holds the stream are dropped (what a seek decodes before its target), a hole is filled with silence.
static void audio_append(AudioState *audio, const float *frames, int64_t count)
{
	int64_t start = audio->streamFrame;
	audio->streamFrame += count;
	if (start < audio->expectedFrame) {
		int64_t skip = audio->expectedFrame - start;
		if (skip >= count)
			return;
		frames += skip * audio->outChannels;
		count -= skip;
	} else if (start > audio->expectedFrame) {
		int64_t gap = start - audio->expectedFrame;
		int64_t limit = (int64_t)audio->output.mSampleRate * 10;
		if (gap > limit)
			gap = limit;
		if (!audio_reserve(audio, gap))
			return;
		memset(audio->src + audio->srcCount * audio->outChannels, 0, (size_t)gap * audio->outBytesPerFrame);
		audio->srcCount += gap;
		audio->expectedFrame += gap;
	}
	if (!audio_reserve(audio, count))
		return;
	memcpy(audio->src + audio->srcCount * audio->outChannels, frames, (size_t)count * audio->outBytesPerFrame);
	audio->srcCount += count;
	audio->expectedFrame += count;
}

// Decodes some audio into src. Returns the number of frames decoded.
static int64_t audio_decode_more(AVPEEngine *engine)
{
	AudioState *audio = &engine->audio;
	AudioBufferList list;
	list.mNumberBuffers = 1;
	list.mBuffers[0].mNumberChannels = audio->outChannels;
	list.mBuffers[0].mDataByteSize = AUDIO_DECODE_FRAMES * audio->outBytesPerFrame;
	list.mBuffers[0].mData = audio->decoded;
	UInt32 frames = AUDIO_DECODE_FRAMES;
	OSStatus status = AudioConverterFillComplexBuffer(audio->converter, audio_input, engine, &frames, &list, NULL);
	int64_t got = list.mBuffers[0].mDataByteSize / audio->outBytesPerFrame;
	if (status != noErr && status != AUDIO_NO_DATA_YET) {
		// A damaged packet: carry on with the next one.
		audio->failures++;
	}
	if (got > 0)
		audio_append(audio, audio->decoded, got);
	else if (audio->inputEnded)
		audio->streamDone = true;
	return got;
}

// Fills a queue buffer from src at the rate and enqueues it. False if there is not enough decoded audio yet.
static bool audio_produce(AVPEEngine *engine, int index, double rate)
{
	AudioState *audio = &engine->audio;
	const UInt32 capacity = audio->bufferFrames;
	// With the queue's time/pitch effect the audio is enqueued as it is (one source frame per frame) and the queue
	// plays it at the rate, so its sample time is media time. Without the effect the rate is applied by reading
	// the decoded audio faster or slower with linear interpolation, so the pitch follows the rate.
	// Rates above 4 are silent.
	const bool silent = rate > 4.0;
	if (audio->timePitch)
		rate = 1.0;
	// Source frames the buffer consumes, and one more to interpolate with.
	int64_t needed = (int64_t)(capacity * rate) + 2;

	while (audio->srcCount - (int64_t)audio->srcPos < needed && !audio->streamDone) {
		if (audio_decode_more(engine) == 0)
			break;
	}
	int64_t available = audio->srcCount - (int64_t)audio->srcPos;
	if (available < needed && !audio->streamDone)
		return false;

	AudioQueueBufferRef buffer = audio->buffers[index];
	float *out = buffer->mAudioData;
	const UInt32 channels = audio->outChannels;
	const double startPos = audio->srcPos;
	UInt32 produced = 0;
	while (produced < capacity) {
		int64_t k = (int64_t)audio->srcPos;
		double fraction = audio->srcPos - (double)k;
		if (fraction == 0 ? k >= audio->srcCount : k + 1 >= audio->srcCount)
			break;
		for (UInt32 c = 0; c < channels; c++) {
			float a = audio->src[k * channels + c];
			float b = fraction == 0 ? a : audio->src[(k + 1) * channels + c];
			out[produced * channels + c] = silent ? 0.0f : a + (b - a) * (float)fraction;
		}
		produced++;
		audio->srcPos += rate;
	}
	if (produced == 0)
		return false;

	int64_t start = audio->enqueuedFrames > audio->lastHeard ? audio->enqueuedFrames : audio->lastHeard;
	AudioSegment segment = {
		start,
		(double)(audio->srcBase + startPos) * 1e6 / audio->output.mSampleRate,
		rate,
	};
	if (audio->segmentCount == AUDIO_SEGMENTS) {
		memmove(&audio->segments[0], &audio->segments[1], (AUDIO_SEGMENTS - 1) * sizeof(AudioSegment));
		audio->segmentCount--;
	}
	audio->segments[audio->segmentCount++] = segment;
	audio->enqueuedFrames = start + produced;

	buffer->mAudioDataByteSize = produced * audio->outBytesPerFrame;
	pthread_mutex_lock(&engine->lock);
	audio->busy[index] = true;
	pthread_mutex_unlock(&engine->lock);
	if (AudioQueueEnqueueBuffer(audio->queue, buffer, 0, NULL) != noErr) {
		pthread_mutex_lock(&engine->lock);
		audio->busy[index] = false;
		pthread_mutex_unlock(&engine->lock);
		audio->failures++;
	}

	// Forget what is far behind the play position.
	int64_t history = (int64_t)audio->output.mSampleRate * AUDIO_HISTORY_SECONDS;
	int64_t drop = (int64_t)audio->srcPos - history;
	if (drop > 16384) {
		memmove(audio->src, audio->src + drop * channels, (size_t)(audio->srcCount - drop) * audio->outBytesPerFrame);
		audio->srcBase += drop;
		audio->srcCount -= drop;
		audio->srcPos -= (double)drop;
	}

	if (audio->prerolling) {
		// The audio of the target is queued (if the seek is that of a stream without video).
		audio_finish_preroll(engine, true);
	}
	return true;
}

// The rate changed: the queue still holds audio rendered at the old one. Starts again from the clock
// position, if the decoded audio still reaches back that far.
static void audio_restart_at(AVPEEngine *engine, int64_t mediaUs)
{
	AudioState *audio = &engine->audio;
	int64_t frame = av_rescale(mediaUs, (int64_t)audio->output.mSampleRate, US_PER_SEC);
	if (frame < audio->srcBase || frame > audio->srcBase + audio->srcCount)
		return;
	audio_clear_output(engine);
	audio->srcPos = (double)(frame - audio->srcBase);
}

static void audio_discard_packets(AVPEEngine *engine)
{
	// Without a working output the packets are only consumed, so that the demuxer does not block.
	for (;;) {
		pthread_mutex_lock(&engine->lock);
		PNode *node = engine->audioQueue.head ? pq_pop(&engine->audioQueue) : NULL;
		pthread_mutex_unlock(&engine->lock);
		if (!node)
			return;
		bool marker = !node->packet;
		if (marker) {
			AudioState *audio = &engine->audio;
			audio_finish_preroll(engine, false);
			audio->gen = node->gen;
			if (engine->audio.owner) {
				audio->prerolling = true;
				audio->prerollToken = node->token;
				audio_finish_preroll(engine, true); // nothing to wait for
			}
		}
		node_free(node);
	}
}

static void *audio_main(void *arg)
{
	AVPEEngine *engine = arg;
	AudioState *audio = &engine->audio;
	audio->opened = audio_open(engine);
	if (!audio->opened) {
		for (;;) {
			pthread_mutex_lock(&engine->lock);
			while (!engine->exiting && !engine->audioQueue.head)
				wait_locked(engine, -1);
			bool exiting = engine->exiting;
			pthread_mutex_unlock(&engine->lock);
			if (exiting)
				break;
			audio_discard_packets(engine);
		}
		audio_close(engine);
		return NULL;
	}

	pthread_mutex_lock(&engine->lock);
	engine->audioActive = true;
	arm_audio_hold_locked(engine);
	pthread_mutex_unlock(&engine->lock);

	audio->lastRate = 1;
	for (;;) {
		pthread_mutex_lock(&engine->lock);
		if (engine->exiting) {
			pthread_mutex_unlock(&engine->lock);
			break;
		}
		const bool playing = engine->playing;
		const double rate = engine->audioRate;
		float volume = engine->volume;
		bool volumeDirty = engine->volumeDirty || !audio->volumeApplied;
		engine->volumeDirty = false;
		bool rateChanged = engine->rateChanged;
		int64_t rateChangeUs = engine->rateChangeMediaUs;
		engine->rateChanged = false;
		const bool algorithmChanged = engine->algorithmChanged;
		const AVPEAudioAlgorithm algorithm = engine->audioAlgorithm;
		engine->algorithmChanged = false;
		bool markerNext = engine->audioQueue.head && !engine->audioQueue.head->packet;
		int freeIndex = -1;
		for (int i = 0; i < AUDIO_BUFFERS; i++) {
			if (!audio->busy[i]) {
				freeIndex = i;
				break;
			}
		}
		pthread_mutex_unlock(&engine->lock);

		if (volumeDirty) {
			AudioQueueSetParameter(audio->queue, kAudioQueueParam_Volume, volume);
			audio->volumeApplied = true;
		}

		if (algorithmChanged && audio->timePitch && algorithm != audio->appliedAlgorithm) {
			// The algorithm of a queue that was started cannot be changed: the queue stops, and the audio
			// goes on from the clock position.
			if (audio->queueStarted) {
				AudioQueueStop(audio->queue, true);
				audio->queueStarted = audio->running = false;
				audio_clear_output(engine);
			}
			audio_set_algorithm(audio, algorithm);
			audio->appliedAlgorithm = algorithm;
			pthread_mutex_lock(&engine->lock);
			const int64_t now = clock_now_locked(engine);
			arm_audio_hold_locked(engine);
			pthread_mutex_unlock(&engine->lock);
			audio_restart_at(engine, now);
		}

		// Play and pause.
		if (!playing)
			audio->startFailed = false; // try again at the next play
		if (playing && !audio->running && !audio->startFailed) {
			const OSStatus startStatus = AudioQueueStart(audio->queue, NULL);
			if (startStatus == noErr) {
				audio->running = true;
				audio->queueStarted = true;
				if (audio->recovering) {
					// The audio was missing while it failed to start: begin again from the clock position.
					audio->recovering = false;
					pthread_mutex_lock(&engine->lock);
					engine->audioActive = true;
					int64_t now = clock_now_locked(engine);
					pthread_mutex_unlock(&engine->lock);
					AVPEEngineSeek(engine, now, 0);
				}
			} else {
				audio->startFailed = true;
				audio->recovering = true;
				// No output: the clock runs on its own.
				pthread_mutex_lock(&engine->lock);
				if (engine->audioHold) {
					engine->audioHold = false;
					rebase_clock_locked(engine, clock_now_locked(engine));
				}
				engine->audioActive = false;
				pthread_cond_broadcast(&engine->wake);
				pthread_mutex_unlock(&engine->lock);
				if (engine->callbacks.started)
					engine->callbacks.started(engine->callbacks.context);
			}
		} else if (!playing && audio->running) {
			AudioQueuePause(audio->queue);
			audio->running = false;
		}
		audio_sync_clock(engine);

		if (audio->timePitch) {
			// The queue applies the rate to what it holds: nothing is read again.
			if (rate != audio->appliedQueueRate) {
				AudioQueueSetParameter(audio->queue, kAudioQueueParam_PlayRate, (float)rate);
				audio->appliedQueueRate = rate;
			}
		} else if (rateChanged && rate != audio->lastRate) {
			audio_restart_at(engine, rateChangeUs);
		}
		audio->lastRate = rate;

		if (audio->startFailed) {
			audio_discard_packets(engine);
			pthread_mutex_lock(&engine->lock);
			if (!engine->exiting && !engine->audioQueue.head)
				wait_locked(engine, 20000);
			pthread_mutex_unlock(&engine->lock);
			continue;
		}

		if (markerNext) {
			pthread_mutex_lock(&engine->lock);
			PNode *node = engine->audioQueue.head && !engine->audioQueue.head->packet ? pq_pop(&engine->audioQueue) : NULL;
			pthread_mutex_unlock(&engine->lock);
			if (node) {
				audio_begin_seek(engine, node);
				node_free(node);
			}
			continue;
		}

		if (freeIndex >= 0 && !audio->streamDone && audio_produce(engine, freeIndex, rate))
			continue;
		if (audio->streamDone && audio->prerolling) {
			audio_finish_preroll(engine, true); // a seek to the end: no audio at the target
		}

		// Nothing to do until a packet or a free buffer shows up, or something changes.
		pthread_mutex_lock(&engine->lock);
		bool work = engine->audioQueue.head && (!engine->audioQueue.head->packet || audio->streamDone == false);
		bool freeNow = false;
		for (int i = 0; i < AUDIO_BUFFERS; i++)
			freeNow |= !audio->busy[i];
		if (!engine->exiting && !(work && freeNow) && !(engine->audioQueue.head && !engine->audioQueue.head->packet) &&
		    !(engine->rateChanged || engine->volumeDirty || engine->algorithmChanged))
			wait_locked(engine, 20000);
		pthread_mutex_unlock(&engine->lock);
	}

	audio_finish_preroll(engine, false);
	audio_close(engine);
	return NULL;
}

#pragma mark - Demuxing

// Requires the lock. Whether the demuxer should wait for the decoders to catch up.
static bool queues_full_locked(AVPEEngine *engine)
{
	size_t total = engine->videoQueue.bytes + engine->audioQueue.bytes;
	if (total > QUEUE_HARD_LIMIT_BYTES)
		return true;
	// A queue is full only if the stream that is behind it has something to decode: reading on is how
	// the data of a badly interleaved file arrives.
	bool videoFull = engine->hasVideo && engine->videoQueue.bytes > VIDEO_QUEUE_BYTES;
	bool audioFull = engine->hasAudio && engine->audioQueue.bytes > AUDIO_QUEUE_BYTES;
	if (videoFull && (!engine->hasAudio || engine->audioQueue.count > 0))
		return true;
	if (audioFull && (!engine->hasVideo || engine->videoQueue.count > 0))
		return true;
	return false;
}

static void demux_seek(AVPEEngine *engine, int64_t targetUs)
{
	AVPEMedia *media = engine->media;
	int index = media->videoIndex >= 0 && engine->hasVideo ? media->videoIndex : media->audioIndex;
	if (index < 0)
		return;
	const AVStream *stream = media->format->streams[index];
	// Some demuxers (MPEG-TS) compare the target with the decode timestamps of the keyframes, which are
	// earlier than the presentation timestamps when the stream has B-frames, and then land after the
	// keyframe. Aiming a little early finds the keyframe in any case; the frames before the target
	// are decoded and dropped anyway.
	int64_t aimUs = targetUs + media->startUs - SEEK_REORDER_MARGIN_US;
	int64_t ts = av_rescale_q(aimUs < 0 ? 0 : aimUs, (AVRational){ 1, US_PER_SEC }, stream->time_base);
	if (av_seek_frame(media->format, index, ts, AVSEEK_FLAG_BACKWARD) < 0)
		av_seek_frame(media->format, index, stream->start_time == AV_NOPTS_VALUE ? 0 : stream->start_time,
			AVSEEK_FLAG_BACKWARD | AVSEEK_FLAG_ANY);
}

static void *demux_main(void *arg)
{
	AVPEEngine *engine = arg;
	AVPEMedia *media = engine->media;
	AVPacket *packet = av_packet_alloc();

	for (;;) {
		pthread_mutex_lock(&engine->lock);
		while (!engine->exiting && !engine->needSeek &&
		       !(engine->demuxEOF ? !engine->endHandled : !queues_full_locked(engine)))
			wait_locked(engine, -1);
		if (engine->exiting) {
			pthread_mutex_unlock(&engine->lock);
			break;
		}
		if (engine->needSeek) {
			int64_t target = engine->seekTargetUs;
			uint64_t token = engine->seekToken;
			engine->needSeek = false;
			rebase_clock_locked(engine, target);
			engine->gen++;
			engine->demuxEOF = false;
			engine->endHandled = false;

			// Whatever is queued is for the old position. Markers dropped here belong to seeks that the
			// threads never reached: they end unfinished.
			uint64_t dropped[16];
			size_t droppedCount = 0;
			pq_flush(&engine->videoQueue, engine->hasVideo ? dropped : NULL, &droppedCount, 16);
			pq_flush(&engine->audioQueue, !engine->hasVideo ? dropped : NULL, &droppedCount, 16);
			PNode *videoMarker = NULL, *audioMarker = NULL;
			if (engine->hasVideo) {
				videoMarker = calloc(1, sizeof(*videoMarker));
				videoMarker->targetUs = target;
				videoMarker->token = token;
				videoMarker->gen = engine->gen;
				pq_push(&engine->videoQueue, videoMarker);
			}
			if (engine->hasAudio) {
				audioMarker = calloc(1, sizeof(*audioMarker));
				audioMarker->targetUs = target;
				audioMarker->token = token;
				audioMarker->gen = engine->gen;
				pq_push(&engine->audioQueue, audioMarker);
			}
			pthread_cond_broadcast(&engine->wake);
			pthread_mutex_unlock(&engine->lock);

			if (engine->callbacks.seekDone) {
				for (size_t i = 0; i < droppedCount; i++)
					engine->callbacks.seekDone(engine->callbacks.context, dropped[i], false);
				if (!engine->hasVideo && !engine->hasAudio && token != 0)
					engine->callbacks.seekDone(engine->callbacks.context, token, true);
			}
			demux_seek(engine, target);
			continue;
		}
		bool atEnd = engine->demuxEOF;
		pthread_mutex_unlock(&engine->lock);

		if (atEnd) {
			// The media lasts until its duration, which can be later than the last frame.
			if (wait_until(engine, media->durationUs, NULL)) {
				pthread_mutex_lock(&engine->lock);
				engine->playing = false;
				engine->rate = 0;
				engine->audioHold = false;
				rebase_clock_locked(engine, media->durationUs);
				engine->endHandled = true;
				pthread_mutex_unlock(&engine->lock);
				if (engine->callbacks.ended)
					engine->callbacks.ended(engine->callbacks.context);
			}
			continue;
		}

		if (!packet || av_read_frame(media->format, packet) < 0) {
			pthread_mutex_lock(&engine->lock);
			engine->demuxEOF = true;
			pthread_cond_broadcast(&engine->wake);
			pthread_mutex_unlock(&engine->lock);
			continue;
		}

		PQueue *queue = NULL;
		if (engine->hasVideo && packet->stream_index == media->videoIndex)
			queue = &engine->videoQueue;
		else if (engine->hasAudio && packet->stream_index == media->audioIndex)
			queue = &engine->audioQueue;
		if (!queue) {
			av_packet_unref(packet);
			continue;
		}
		PNode *node = calloc(1, sizeof(*node));
		AVPacket *owned = av_packet_alloc();
		if (!node || !owned) {
			free(node);
			av_packet_free(&owned);
			av_packet_unref(packet);
			continue;
		}
		av_packet_move_ref(owned, packet);
		node->packet = owned;
		pthread_mutex_lock(&engine->lock);
		pq_push(queue, node);
		pthread_cond_broadcast(&engine->wake);
		pthread_mutex_unlock(&engine->lock);
	}

	av_packet_free(&packet);
	return NULL;
}

#pragma mark - Engine API

AVPEEngine *AVPEEngineCreate(AVPEMedia *media, AVPECallbacks callbacks)
{
	AVPEEngine *engine = calloc(1, sizeof(*engine));
	if (!engine) {
		AVPEMediaClose(media);
		return NULL;
	}
	engine->media = media;
	engine->callbacks = callbacks;
	pthread_mutex_init(&engine->lock, NULL);
	pthread_cond_init(&engine->wake, NULL);
	engine->rate = 0;
	engine->audioRate = 1;
	engine->volume = 1;
	rebase_clock_locked(engine, 0);
	// Show the first frame without playing.
	engine->needSeek = true;
	engine->seekTargetUs = 0;
	engine->seekToken = 0;

	if (media->videoIndex >= 0 && create_format_description(engine) && create_session(engine))
		engine->hasVideo = true;
	engine->hasAudio = media->audioIndex >= 0;
	engine->audio.owner = !engine->hasVideo;

	if (pthread_create(&engine->demuxThread, NULL, demux_main, engine) != 0) {
		AVPEEngineDestroy(engine);
		return NULL;
	}
	engine->demuxStarted = true;
	if (engine->hasVideo) {
		if (pthread_create(&engine->videoThread, NULL, video_main, engine) != 0) {
			AVPEEngineDestroy(engine);
			return NULL;
		}
		engine->videoStarted = true;
	}
	if (engine->hasAudio) {
		if (pthread_create(&engine->audioThread, NULL, audio_main, engine) != 0) {
			AVPEEngineDestroy(engine);
			return NULL;
		}
		engine->audioStarted = true;
	}
	return engine;
}

void AVPEEngineDestroy(AVPEEngine *engine)
{
	if (!engine)
		return;
	pthread_mutex_lock(&engine->lock);
	engine->exiting = true;
	pthread_cond_broadcast(&engine->wake);
	pthread_mutex_unlock(&engine->lock);
	if (engine->demuxStarted)
		pthread_join(engine->demuxThread, NULL);
	if (engine->videoStarted)
		pthread_join(engine->videoThread, NULL);
	if (engine->audioStarted)
		pthread_join(engine->audioThread, NULL);
	destroy_session(engine);
	release_held(engine);
	if (engine->formatDescription)
		CFRelease(engine->formatDescription);
	pq_flush(&engine->videoQueue, NULL, NULL, 0);
	pq_flush(&engine->audioQueue, NULL, NULL, 0);
	AVPEMediaClose(engine->media);
	pthread_cond_destroy(&engine->wake);
	pthread_mutex_destroy(&engine->lock);
	free(engine);
}

void AVPEEngineSetRate(AVPEEngine *engine, float rate)
{
	pthread_mutex_lock(&engine->lock);
	bool play = rate > 0;
	bool atEnd = engine->baseMediaUs >= engine->media->durationUs && engine->media->durationUs > 0 && !engine->needSeek;
	if (play && atEnd && !engine->playing)
		play = false; // nothing to play from the end: the position has to be moved first
	const bool wasPlaying = engine->playing;
	const int64_t now = clock_now_locked(engine);
	rebase_clock_locked(engine, now);
	engine->playing = play;
	engine->rate = play ? rate : 0;
	if (!play) {
		engine->audioHold = false;
	} else {
		if (engine->audioRate != rate) {
			// What the queue holds was rendered at the old rate.
			engine->audioRate = rate;
			engine->rateChanged = true;
			engine->rateChangeMediaUs = now;
			arm_audio_hold_locked(engine);
		} else if (!wasPlaying) {
			arm_audio_hold_locked(engine);
		}
	}
	pthread_cond_broadcast(&engine->wake);
	pthread_mutex_unlock(&engine->lock);
}

void AVPEEngineSetAudioAlgorithm(AVPEEngine *engine, AVPEAudioAlgorithm algorithm)
{
	pthread_mutex_lock(&engine->lock);
	if (engine->audioAlgorithm != algorithm) {
		engine->audioAlgorithm = algorithm;
		engine->algorithmChanged = true;
		pthread_cond_broadcast(&engine->wake);
	}
	pthread_mutex_unlock(&engine->lock);
}

float AVPEEngineRate(AVPEEngine *engine)
{
	pthread_mutex_lock(&engine->lock);
	float rate = engine->playing ? engine->rate : 0;
	pthread_mutex_unlock(&engine->lock);
	return rate;
}

int64_t AVPEEngineTimeUs(AVPEEngine *engine)
{
	pthread_mutex_lock(&engine->lock);
	int64_t now = clock_now_locked(engine);
	pthread_mutex_unlock(&engine->lock);
	return now;
}

void AVPEEngineSeek(AVPEEngine *engine, int64_t timeUs, uint64_t token)
{
	int64_t duration = engine->media->durationUs;
	if (timeUs < 0)
		timeUs = 0;
	if (duration > 0 && timeUs > duration)
		timeUs = duration;

	pthread_mutex_lock(&engine->lock);
	uint64_t replaced = engine->needSeek ? engine->seekToken : 0;
	engine->needSeek = true;
	engine->seekTargetUs = timeUs;
	engine->seekToken = token;
	// The clock moves at once, so the time reads back as the target while the frame is found.
	rebase_clock_locked(engine, timeUs);
	// While playing, it waits for the audio of the new position.
	arm_audio_hold_locked(engine);
	pthread_cond_broadcast(&engine->wake);
	pthread_mutex_unlock(&engine->lock);

	// A seek that was requested but not started yet never finishes.
	if (replaced != 0 && engine->callbacks.seekDone)
		engine->callbacks.seekDone(engine->callbacks.context, replaced, false);
}

int64_t AVPEEngineDurationUs(const AVPEEngine *engine)
{
	return engine->media->durationUs;
}

void AVPEEngineSetVolume(AVPEEngine *engine, float volume)
{
	pthread_mutex_lock(&engine->lock);
	engine->volume = volume < 0 ? 0 : volume;
	engine->volumeDirty = true;
	pthread_cond_broadcast(&engine->wake);
	pthread_mutex_unlock(&engine->lock);
}

bool AVPEEngineHasAudio(const AVPEEngine *engine)
{
	return engine->hasAudio;
}

bool AVPEEngineWaiting(AVPEEngine *engine)
{
	pthread_mutex_lock(&engine->lock);
	bool waiting = engine->playing && engine->audioHold;
	pthread_mutex_unlock(&engine->lock);
	return waiting;
}
