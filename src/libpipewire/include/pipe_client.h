/*
 * This file is part of Darling.
 *
 * Copyright (C) 2026 Darling developers
 *
 * Darling is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 */

/*
 * The small PipeWire client surface used by Darling's media frameworks.
 *
 * It deliberately exposes neither libpipewire nor libspa types: the frameworks
 * link to a wrapped host libpipewire, and this interface is the only shared
 * boundary between them.
 */

#ifndef DARLING_PIPEWIRE_CLIENT_H_
#define DARLING_PIPEWIRE_CLIENT_H_

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

struct darling_pw_stream;

enum darling_pw_media
{
	DARLING_PW_MEDIA_AUDIO = 0,
	DARLING_PW_MEDIA_VIDEO = 1,
};

enum darling_pw_video_format
{
	DARLING_PW_VIDEO_FORMAT_UNKNOWN = 0,
	DARLING_PW_VIDEO_FORMAT_BGRX = 1,
	DARLING_PW_VIDEO_FORMAT_RGBX = 2,
	DARLING_PW_VIDEO_FORMAT_XRGB = 3,
	DARLING_PW_VIDEO_FORMAT_XBGR = 4,
	DARLING_PW_VIDEO_FORMAT_RGB24 = 5,
	DARLING_PW_VIDEO_FORMAT_BGR24 = 6,
	DARLING_PW_VIDEO_FORMAT_I420 = 7,
	DARLING_PW_VIDEO_FORMAT_NV12 = 8,
	DARLING_PW_VIDEO_FORMAT_YUY2 = 9,
	DARLING_PW_VIDEO_FORMAT_UYVY = 10,
	DARLING_PW_VIDEO_FORMAT_P010 = 11,
};

/* Values of SPA_META_HEADER_FLAG_*. Exposed so callers can detect a break in
 * the capture timeline without depending on SPA headers. */
enum
{
	DARLING_PW_BUFFER_FLAG_DISCONT = 1 << 0,
	DARLING_PW_BUFFER_FLAG_CORRUPTED = 1 << 1,
};

struct darling_pw_video_source
{
	/* PipeWire global id, usable as darling_pw_stream_config.target_id. */
	uint32_t id;
	char name[128];
};

struct darling_pw_stream_config
{
	enum darling_pw_media media;
	/* 1 for capture (source), 0 for playback (sink). */
	int capture;
	const char* name;
	const char* role;
	/* Node id from darling_pw_enumerate_video_sources for a direct PipeWire
	 * connection; 0 means "any". A portal fd selects its own source, so this
	 * value is ignored when remote_fd >= 0. */
	uint32_t target_id;
	/* Portal stream target (PW_KEY_TARGET_OBJECT), for example the serial
	 * from a ScreenCast stream. A portal fd may expose several streams. */
	const char* target_object;
	/* Audio: frames per second and channels. */
	uint32_t rate;
	uint32_t channels;
	/* Video: preferred dimensions and maximum framerate. Zero means "any". */
	uint32_t width;
	uint32_t height;
	uint32_t framerate;
	/* Restrict format negotiation to RGB-family formats. Screen capture uses
	 * this because its consumers expose an 8-bit RGB surface. */
	int rgb_only;
	/* Allow HDR-capable formats such as P010 when the runtime supports them.
	 * The default is SDR-only to preserve existing consumers. */
	int allow_hdr;
};

struct darling_pw_stream_events
{
	uint32_t version;
	/* Called on the stream thread with the stream lock held. */
	void (*process)(void* user_data);
	void (*state_changed)(void* user_data, int state);
	/* param is a libspa spa_pod; only useful to internal callers. */
	void (*param_changed)(void* user_data, uint32_t id, const void* param);
};

struct darling_pw_buffer
{
	/* Plane 0. For audio this is the only plane. */
	void* data;
	size_t capacity;
	/* Filled bytes on capture; zero on playback. For planar video this is the
	 * sum of the filled bytes of all planes. */
	size_t filled;
	/* Row stride for video, or bytes per frame for audio. */
	uint32_t stride;
	/* Frames PipeWire would like the caller to provide, for playback. */
	uint32_t requested_frames;

	/* All planes of the buffer. Audio and packed video only use plane 0;
	 * I420 uses planes 0/1/2 and NV12 uses planes 0/1. NULL for unused planes. */
	void* planes[4];
	uint32_t plane_strides[4];
	size_t plane_sizes[4];

	/* CLOCK_MONOTONIC presentation time in nanoseconds, taken from the buffer
	 * header when present and from the graph cycle time otherwise. -1 means
	 * the source did not provide a usable timestamp. */
	int64_t pts_ns;
	/* DARLING_PW_BUFFER_FLAG_* for this buffer. */
	uint32_t flags;
};

struct darling_pw_time
{
	int64_t now_ns;
	uint64_t ticks;
	uint32_t rate_num;
	uint32_t rate_denom;
	int64_t delay_ticks;
};

enum darling_pw_transfer_function
{
	DARLING_PW_TRANSFER_UNKNOWN = 0,
	DARLING_PW_TRANSFER_SRGB = 1,
	DARLING_PW_TRANSFER_SDR_VIDEO = 2,
	DARLING_PW_TRANSFER_PQ = 3,
	DARLING_PW_TRANSFER_HLG = 4,
};

enum darling_pw_color_primaries
{
	DARLING_PW_PRIMARIES_UNKNOWN = 0,
	DARLING_PW_PRIMARIES_BT709 = 1,
	DARLING_PW_PRIMARIES_BT2020 = 2,
};

struct darling_pw_format
{
	enum darling_pw_video_format video_format;
	uint32_t width;
	uint32_t height;
	uint32_t framerate_num;
	uint32_t framerate_denom;
	enum darling_pw_transfer_function transfer;
	enum darling_pw_color_primaries primaries;
	int hdr;
};

/* PipeWire runtime version/capabilities. These read the linked libpipewire
 * version, so callers can gate optional features such as HDR screen capture. */
int darling_pw_runtime_version(uint32_t* major, uint32_t* minor, uint32_t* micro);
int darling_pw_runtime_supports_hdr(void);

/* Returns non-zero when a PipeWire server can be reached on the usual socket. */
int darling_pw_can_connect(void);

/* Lists video capture nodes currently published by PipeWire. Returns the
 * number of sources found, or a negative errno on failure. */
int darling_pw_enumerate_video_sources(struct darling_pw_video_source* out, uint32_t max);

/* remote_fd >= 0 is a Camera portal fd; on success the stream owns it. */
int darling_pw_stream_create(const struct darling_pw_stream_config* config,
	int remote_fd, const struct darling_pw_stream_events* events, void* user_data,
	struct darling_pw_stream** out);
void darling_pw_stream_destroy(struct darling_pw_stream* stream);

int darling_pw_stream_lock(struct darling_pw_stream* stream);
void darling_pw_stream_unlock(struct darling_pw_stream* stream);
/* Waits for STREAMING or ERROR. Returns 0 on success, negative on error. */
int darling_pw_stream_wait(struct darling_pw_stream* stream, int timeout_ms);
int darling_pw_stream_set_active(struct darling_pw_stream* stream, int active);
int darling_pw_stream_flush(struct darling_pw_stream* stream);

/* These must be called with the stream lock held, normally from process(). */
int darling_pw_stream_dequeue(struct darling_pw_stream* stream,
	struct darling_pw_buffer* out, void** cookie);
/* used_bytes is bytes for video and frames multiplied by bytes-per-frame for audio. */
int darling_pw_stream_queue(struct darling_pw_stream* stream, void* cookie, size_t used_bytes);

int darling_pw_stream_get_time(struct darling_pw_stream* stream, struct darling_pw_time* out);
int darling_pw_stream_get_format(struct darling_pw_stream* stream, struct darling_pw_format* out);
int darling_pw_stream_state(struct darling_pw_stream* stream);

/* PipeWire stream states, copied so callers need no PipeWire headers. */
enum
{
	DARLING_PW_STATE_ERROR = -1,
	DARLING_PW_STATE_UNCONNECTED = 0,
	DARLING_PW_STATE_CONNECTING = 1,
	DARLING_PW_STATE_PAUSED = 2,
	DARLING_PW_STATE_STREAMING = 3,
};

#ifdef __cplusplus
}
#endif

#endif
