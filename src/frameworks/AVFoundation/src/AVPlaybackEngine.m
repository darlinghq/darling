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
		if (stream->duration != AV_NOPTS_VALUE && stream->duration > 0)
			track->durationUs = av_rescale_q(stream->duration, stream->time_base, (AVRational){1, US_PER_SEC});
	}

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

struct AVPEEngine {
	AVPEMedia *media;
	AVPECallbacks callbacks;

	pthread_t thread;
	bool threadStarted;

	// The playback clock and the requests from other threads. Guarded by lock.
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

	// Only touched by the engine thread.
	VTDecompressionSessionRef session;
	CMVideoFormatDescriptionRef formatDescription;
	bool prerolling; // decoding up to the target of a seek
	int64_t prerollTargetUs;
	uint64_t prerollToken;
	CVPixelBufferRef held; // the last frame before the seek target
	int64_t heldUs;
	bool atEnd; // the demuxer is exhausted
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
	if (engine->playing)
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

static bool interrupted(AVPEEngine *engine)
{
	pthread_mutex_lock(&engine->lock);
	bool result = engine->exiting || engine->needSeek;
	pthread_mutex_unlock(&engine->lock);
	return result;
}

// Blocks until the playback clock reaches the media time, while playing. Returns false if
// playback is shutting down or a seek replaced the position.
static bool wait_until(AVPEEngine *engine, int64_t mediaUs)
{
	pthread_mutex_lock(&engine->lock);
	for (;;) {
		if (engine->exiting || engine->needSeek) {
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
		int64_t remaining = (int64_t)((double)(mediaUs - now) / engine->rate);
		wait_locked(engine, remaining < 1000 ? 1000 : remaining);
	}
	pthread_mutex_unlock(&engine->lock);
	return true;
}

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

// Runs on the engine's thread, inside VTDecompressionSessionDecodeFrame or ...FinishDelayedFrames:
// frames arrive in presentation order.
static void decoded_frame(void *refCon, void *sourceFrameRefCon, OSStatus status, VTDecodeInfoFlags flags,
                          CVImageBufferRef imageBuffer, CMTime pts, CMTime duration)
{
	AVPEEngine *engine = refCon;
	if (status != noErr || !imageBuffer || !CMTIME_IS_VALID(pts))
		return;
	int64_t ptsUs = pts.timescale == US_PER_SEC ? pts.value : av_rescale(pts.value, US_PER_SEC, pts.timescale);
	if (ptsUs < 0 || interrupted(engine))
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

	if (wait_until(engine, ptsUs))
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

// Repositions the demuxer and the decoder; frames before the target are decoded but not shown.
static void perform_seek(AVPEEngine *engine, int64_t targetUs, uint64_t token)
{
	AVPEMedia *media = engine->media;
	finish_preroll(engine, false);

	if (media->videoIndex >= 0) {
		const AVStream *stream = media->format->streams[media->videoIndex];
		// Some demuxers (MPEG-TS) compare the target with the decode timestamps of the keyframes, which are
		// earlier than the presentation timestamps when the stream has B-frames, and then land after the
		// keyframe. Aiming a little early finds the keyframe in any case; the frames before the target
		// are decoded and dropped anyway.
		int64_t aimUs = targetUs + media->startUs - SEEK_REORDER_MARGIN_US;
		int64_t ts = av_rescale_q(aimUs < 0 ? 0 : aimUs, (AVRational){1, US_PER_SEC}, stream->time_base);
		if (av_seek_frame(media->format, media->videoIndex, ts, AVSEEK_FLAG_BACKWARD) < 0)
			av_seek_frame(media->format, media->videoIndex, stream->start_time == AV_NOPTS_VALUE ? 0 : stream->start_time,
				AVSEEK_FLAG_BACKWARD | AVSEEK_FLAG_ANY);
		// The decoder's reference frames and queued output belong to the old position.
		create_session(engine);
	}
	engine->atEnd = false;
	engine->prerolling = true;
	engine->prerollTargetUs = targetUs;
	engine->prerollToken = token;
}

static void reach_end(AVPEEngine *engine)
{
	flush_decoder(engine);
	if (engine->prerolling) {
		// Seeking past the last frame shows the last frame.
		if (engine->held && !interrupted(engine))
			present(engine, engine->held);
		finish_preroll(engine, true);
	}
	engine->atEnd = true;

	// The media lasts until its duration, which can be later than the last frame.
	if (wait_until(engine, engine->media->durationUs)) {
		pthread_mutex_lock(&engine->lock);
		engine->playing = false;
		engine->rate = 0;
		rebase_clock_locked(engine, engine->media->durationUs);
		pthread_mutex_unlock(&engine->lock);
		if (engine->callbacks.ended)
			engine->callbacks.ended(engine->callbacks.context);
	}
}

static void *engine_main(void *arg)
{
	AVPEEngine *engine = arg;
	AVPEMedia *media = engine->media;
	AVPacket *packet = av_packet_alloc();

	for (;;) {
		pthread_mutex_lock(&engine->lock);
		while (!engine->exiting && !engine->needSeek && !((engine->playing || engine->prerolling) && !engine->atEnd))
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
			pthread_mutex_unlock(&engine->lock);
			perform_seek(engine, target, token);
			continue;
		}
		pthread_mutex_unlock(&engine->lock);

		if (media->videoIndex < 0 || !packet) {
			reach_end(engine);
			continue;
		}
		int result = av_read_frame(media->format, packet);
		if (result < 0) {
			reach_end(engine);
			continue;
		}
		if (packet->stream_index == media->videoIndex)
			decode_packet(engine, packet);
		av_packet_unref(packet);
	}

	finish_preroll(engine, false);
	av_packet_free(&packet);
	return NULL;
}

AVPEEngine *AVPEEngineCreate(AVPEMedia *media, AVPECallbacks callbacks)
{
	AVPEEngine *engine = calloc(1, sizeof(*engine));
	if (!engine)
		return NULL;
	engine->media = media;
	engine->callbacks = callbacks;
	pthread_mutex_init(&engine->lock, NULL);
	pthread_cond_init(&engine->wake, NULL);
	engine->rate = 0;
	rebase_clock_locked(engine, 0);
	// Show the first frame without playing.
	engine->needSeek = true;
	engine->seekTargetUs = 0;
	engine->seekToken = 0;

	if (media->videoIndex >= 0 && create_format_description(engine))
		create_session(engine);

	if (pthread_create(&engine->thread, NULL, engine_main, engine) != 0) {
		AVPEEngineDestroy(engine);
		return NULL;
	}
	engine->threadStarted = true;
	return engine;
}

void AVPEEngineDestroy(AVPEEngine *engine)
{
	if (!engine)
		return;
	if (engine->threadStarted) {
		pthread_mutex_lock(&engine->lock);
		engine->exiting = true;
		pthread_cond_broadcast(&engine->wake);
		pthread_mutex_unlock(&engine->lock);
		pthread_join(engine->thread, NULL);
	}
	destroy_session(engine);
	release_held(engine);
	if (engine->formatDescription)
		CFRelease(engine->formatDescription);
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
	rebase_clock_locked(engine, clock_now_locked(engine));
	engine->playing = play;
	engine->rate = play ? rate : 0;
	pthread_cond_broadcast(&engine->wake);
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
