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
 * The shared screen-capture service. CoreGraphics, ScreenCaptureKit and
 * AVFoundation all reach the ScreenCast portal through this file, so a process
 * asks for consent once per cursor mode and reuses the resulting PipeWire
 * remote for every stream.
 */

#include "screen_capture_backend.h"
#include "screencast_portal.h"

#include <errno.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <darling_dbus.h>

#define PROBE_TIMEOUT_MS 1500
#define CONSENT_TIMEOUT_MS 15000

struct portal_cache
{
	struct darling_screencast_session* session;
	const struct darling_screencast_stream* streams;
	uint32_t count;
};

struct darling_screen_capture_stream
{
	struct darling_pw_stream* pipe;
	struct darling_screen_capture_callbacks callbacks;
	void* user_data;
};

static pthread_mutex_t portal_mutex = PTHREAD_MUTEX_INITIALIZER;
/* One session per cursor mode: the mode is fixed when SelectSources runs. */
static struct portal_cache portal_sessions[2];
static int capture_permitted;

static struct darling_screencast_portal_api portal_api_for(int timeout_ms)
{
	struct darling_screencast_portal_api api;

	api.dbus = darling_dbus_get(DARLING_DBUS_HOST_FALLBACK);
	api.getenv = getenv;
	api.timeout_ms = timeout_ms;
	return api;
}

int darling_screen_capture_available(void)
{
	struct darling_screencast_portal_api api = portal_api_for(PROBE_TIMEOUT_MS);
	uint32_t sources = 0;

	if (!api.dbus || !darling_pw_can_connect())
		return 0;
	return darling_screencast_portal_properties(&api, &sources, NULL, NULL)
		== DARLING_SCREENCAST_OK && sources != 0;
}

int darling_screen_capture_supports_hdr(void)
{
	return darling_pw_runtime_supports_hdr();
}

int darling_screen_capture_permitted(void)
{
	int permitted;

	pthread_mutex_lock(&portal_mutex);
	permitted = capture_permitted;
	pthread_mutex_unlock(&portal_mutex);
	return permitted;
}

static int portal_error(int result)
{
	switch (result)
	{
		case DARLING_SCREENCAST_OK: return 0;
		case DARLING_SCREENCAST_DENIED:
		case DARLING_SCREENCAST_CANCELLED: return -EACCES;
		case DARLING_SCREENCAST_NOT_AVAILABLE: return -ENODEV;
		case DARLING_SCREENCAST_NO_STREAMS: return -ENODATA;
		default: return -EIO;
	}
}

/* Called with portal_mutex held. */
static int ensure_portal_session(int show_cursor, int timeout_ms)
{
	struct darling_screencast_portal_api api = portal_api_for(timeout_ms);
	struct darling_screencast_config config;
	struct portal_cache* cache = &portal_sessions[show_cursor ? 1 : 0];
	uint32_t source_types = 0, cursor_modes = 0;
	struct darling_screencast_session* session = NULL;
	int result;

	if (cache->session)
		return 0;
	if (!api.dbus)
		return -ENODEV;
	/* Without a PipeWire daemon there is nothing to capture into; do not put a
	 * consent dialog in front of the user for a stream that cannot start. */
	if (!darling_pw_can_connect())
		return -ENODEV;

	if (darling_screencast_portal_properties(&api, &source_types, &cursor_modes, NULL)
		!= DARLING_SCREENCAST_OK || source_types == 0)
		return -ENODEV;

	memset(&config, 0, sizeof(config));
	config.source_types = source_types & DARLING_SCREENCAST_SOURCE_MONITOR;
	if (!config.source_types)
		config.source_types = source_types;
	if (show_cursor && (cursor_modes & DARLING_SCREENCAST_CURSOR_EMBEDDED))
		config.cursor_mode = DARLING_SCREENCAST_CURSOR_EMBEDDED;
	else if (cursor_modes & DARLING_SCREENCAST_CURSOR_HIDDEN)
		config.cursor_mode = DARLING_SCREENCAST_CURSOR_HIDDEN;
	config.persist_mode = 1;

	result = darling_screencast_session_create(&api, &config, &session);
	if (result != DARLING_SCREENCAST_OK)
		return portal_error(result);

	cache->session = session;
	cache->streams = darling_screencast_session_streams(session, &cache->count);
	capture_permitted = 1;
	return 0;
}

int darling_screen_capture_request_access(void)
{
	int result;

	pthread_mutex_lock(&portal_mutex);
	result = ensure_portal_session(0, CONSENT_TIMEOUT_MS);
	pthread_mutex_unlock(&portal_mutex);
	return result;
}

static int64_t rectangle_overlap(int32_t ax, int32_t ay, int32_t aw, int32_t ah,
	int32_t bx, int32_t by, int32_t bw, int32_t bh)
{
	int64_t left = ax > bx ? ax : bx;
	int64_t top = ay > by ? ay : by;
	int64_t right = (int64_t) ax + aw < (int64_t) bx + bw
		? (int64_t) ax + aw : (int64_t) bx + bw;
	int64_t bottom = (int64_t) ay + ah < (int64_t) by + bh
		? (int64_t) ay + ah : (int64_t) by + bh;

	if (right <= left || bottom <= top)
		return 0;
	return (right - left) * (bottom - top);
}

/* Returns the stream whose announced rectangle best covers the requested one.
 * A zero-sized request means "any monitor"; a request outside all announced
 * rectangles falls back to the first monitor. */
static uint32_t select_stream(const struct darling_screencast_stream* streams, uint32_t count,
	int32_t x, int32_t y, uint32_t width, uint32_t height)
{
	uint32_t best = 0, i;
	int64_t best_score = 0;

	if (!streams || count == 0)
		return 0;
	if (count == 1 || (!width && !height))
		return 0;

	for (i = 0; i < count; i++)
	{
		int32_t sw = streams[i].width > 0 ? streams[i].width : 0;
		int32_t sh = streams[i].height > 0 ? streams[i].height : 0;
		int64_t score = rectangle_overlap(x, y, (int32_t) width, (int32_t) height,
			streams[i].x, streams[i].y, sw, sh);

		if (score > best_score)
		{
			best_score = score;
			best = i;
		}
	}
	return best;
}

static void on_process(void* user_data)
{
	struct darling_screen_capture_stream* capture = user_data;
	struct darling_pw_buffer buffer;
	struct darling_screen_capture_frame frame;
	struct darling_pw_format format;
	void* cookie = NULL;
	uint32_t i;

	memset(&buffer, 0, sizeof(buffer));
	if (darling_pw_stream_dequeue(capture->pipe, &buffer, &cookie) <= 0)
		return;
	if (darling_pw_stream_get_format(capture->pipe, &format) != 0)
	{
		darling_pw_stream_queue(capture->pipe, cookie, 0);
		return;
	}

	memset(&frame, 0, sizeof(frame));
	frame.pts_ns = buffer.pts_ns;
	frame.flags = buffer.flags;
	frame.format = format.video_format;
	frame.width = format.width;
	frame.height = format.height;
	frame.transfer = format.transfer;
	frame.primaries = format.primaries;
	frame.hdr = format.hdr;
	for (i = 0; i < 4; i++)
	{
		frame.planes[i] = buffer.planes[i];
		frame.strides[i] = buffer.plane_strides[i];
		frame.sizes[i] = buffer.plane_sizes[i];
	}

	if (capture->callbacks.frame)
		capture->callbacks.frame(&frame, capture->user_data);
	darling_pw_stream_queue(capture->pipe, cookie, 0);
}

static void on_state_changed(void* user_data, int state)
{
	struct darling_screen_capture_stream* capture = user_data;

	if (state == DARLING_PW_STATE_ERROR && capture->callbacks.error)
		capture->callbacks.error(-EIO, capture->user_data);
}

static void on_param_changed(void* user_data, uint32_t id, const void* param)
{
	(void) user_data; (void) id; (void) param;
}

int darling_screen_capture_open(int32_t x, int32_t y, uint32_t width, uint32_t height,
	int show_cursor, int allow_hdr, const struct darling_screen_capture_callbacks* callbacks,
	void* user_data, struct darling_screen_capture_stream** out)
{
	static const struct darling_pw_stream_events events = {
		.version = 1,
		.process = on_process,
		.state_changed = on_state_changed,
		.param_changed = on_param_changed,
	};
	struct darling_screen_capture_stream* capture;
	struct darling_pw_stream_config config;
	struct darling_pw_stream* pipe = NULL;
	const struct darling_screencast_stream* stream;
	uint32_t index, count;
	int fd = -1, result;

	if (!out)
		return -EINVAL;
	*out = NULL;
	if (!callbacks)
		return -EINVAL;

	pthread_mutex_lock(&portal_mutex);
	result = ensure_portal_session(show_cursor, CONSENT_TIMEOUT_MS);
	if (result != 0)
	{
		pthread_mutex_unlock(&portal_mutex);
		return result;
	}
	index = select_stream(portal_sessions[show_cursor ? 1 : 0].streams,
		portal_sessions[show_cursor ? 1 : 0].count, x, y, width, height);
	stream = &portal_sessions[show_cursor ? 1 : 0].streams[index];
	count = portal_sessions[show_cursor ? 1 : 0].count;
	fd = dup(darling_screencast_session_fd(portal_sessions[show_cursor ? 1 : 0].session));
	pthread_mutex_unlock(&portal_mutex);

	if (fd < 0)
		return -errno;
	if (index >= count)
	{
		close(fd);
		return -ENODATA;
	}

	capture = calloc(1, sizeof(*capture));
	if (!capture)
	{
		close(fd);
		return -ENOMEM;
	}
	capture->callbacks = *callbacks;
	capture->user_data = user_data;

	memset(&config, 0, sizeof(config));
	config.media = DARLING_PW_MEDIA_VIDEO;
	config.capture = 1;
	config.name = "darling-screen-capture";
	config.role = "capture";
	config.target_object = stream->target_object;
	config.width = stream->width > 0 ? (uint32_t) stream->width : 0;
	config.height = stream->height > 0 ? (uint32_t) stream->height : 0;
	config.framerate = 30;
	config.rgb_only = !allow_hdr;
	config.allow_hdr = allow_hdr && darling_pw_runtime_supports_hdr();

	/* The portal fd is owned by darling_pw_stream_create on every path. */
	result = darling_pw_stream_create(&config, fd, &events, capture, &pipe);
	if (result != 0)
	{
		free(capture);
		return result;
	}
	capture->pipe = pipe;
	darling_pw_stream_set_active(pipe, 0);
	*out = capture;
	return 0;
}

int darling_screen_capture_start(struct darling_screen_capture_stream* stream, int active)
{
	if (!stream)
		return -EINVAL;
	return darling_pw_stream_set_active(stream->pipe, active);
}

int darling_screen_capture_wait(struct darling_screen_capture_stream* stream, int timeout_ms)
{
	return stream ? darling_pw_stream_wait(stream->pipe, timeout_ms) : -EINVAL;
}

struct darling_pw_stream* darling_screen_capture_pipe(struct darling_screen_capture_stream* stream)
{
	return stream ? stream->pipe : NULL;
}

void darling_screen_capture_close(struct darling_screen_capture_stream* stream)
{
	if (!stream)
		return;
	darling_pw_stream_set_active(stream->pipe, 0);
	darling_pw_stream_destroy(stream->pipe);
	free(stream);
}

static inline uint8_t clamp_u8(int v) { return (uint8_t)(v < 0 ? 0 : v > 255 ? 255 : v); }

static void yuv_to_rgb8(int y10, int cb10, int cr10, enum darling_pw_transfer_function transfer, uint8_t* red, uint8_t* green, uint8_t* blue)
{
	/* P010 carries 10-bit video-range values in the high bits. For PQ/HDR we
	 * tone-map luminance into 8-bit sRGB-ish output so existing BGRA consumers
	 * still get a viewable frame; HDR metadata is preserved on the frame. */
	double y = (y10 - 64.0) / 876.0;
	double cb = (cb10 - 512.0) / 896.0;
	double cr = (cr10 - 512.0) / 896.0;
	double r = y + 1.4746 * cr;
	double g = y - 0.16455312684366 * cb - 0.57135312684366 * cr;
	double b = y + 1.8814 * cb;
	if (r < 0) r = 0; if (g < 0) g = 0; if (b < 0) b = 0;
	if (transfer == DARLING_PW_TRANSFER_PQ || transfer == DARLING_PW_TRANSFER_HLG)
	{
		/* Simple exposure/tone map for HDR -> SDR preview. */
		double luma = 0.2627 * r + 0.6780 * g + 0.0593 * b;
		double scale = 1.0 / (1.0 + luma);
		r *= scale * 1.8; g *= scale * 1.8; b *= scale * 1.8;
	}
	*red = clamp_u8((int)(r * 255.0 + 0.5));
	*green = clamp_u8((int)(g * 255.0 + 0.5));
	*blue = clamp_u8((int)(b * 255.0 + 0.5));
}

static int read_rgb_pixel(const struct darling_screen_capture_frame* frame,
	uint32_t x, uint32_t y, uint8_t* red, uint8_t* green, uint8_t* blue)
{
	const uint8_t* pixel;
	size_t offset;
	uint32_t bytes;

	if (frame->format == DARLING_PW_VIDEO_FORMAT_P010)
	{
		const uint16_t* py;
		const uint16_t* puv;
		size_t offy, offuv;
		uint32_t cx = x / 2, cy = y / 2;
		int y10, cb10, cr10;
		if (!frame->planes[0] || !frame->planes[1])
			return -EINVAL;
		if (frame->strides[0] < frame->width * 2 || frame->strides[1] < frame->width * 2)
			return -EINVAL;
		offy = (size_t) y * frame->strides[0] + (size_t) x * 2;
		offuv = (size_t) cy * frame->strides[1] + (size_t) cx * 4;
		if ((frame->sizes[0] && offy + 2 > frame->sizes[0]) || (frame->sizes[1] && offuv + 4 > frame->sizes[1]))
			return -EINVAL;
		py = (const uint16_t*) ((const uint8_t*) frame->planes[0] + offy);
		puv = (const uint16_t*) ((const uint8_t*) frame->planes[1] + offuv);
		y10 = (int) (py[0] >> 6);
		cb10 = (int) (puv[0] >> 6);
		cr10 = (int) (puv[1] >> 6);
		yuv_to_rgb8(y10, cb10, cr10, frame->transfer, red, green, blue);
		return 0;
	}

	switch (frame->format)
	{
		case DARLING_PW_VIDEO_FORMAT_BGRX:
		case DARLING_PW_VIDEO_FORMAT_RGBX:
		case DARLING_PW_VIDEO_FORMAT_XRGB:
		case DARLING_PW_VIDEO_FORMAT_XBGR:
			bytes = 4;
			break;
		case DARLING_PW_VIDEO_FORMAT_RGB24:
		case DARLING_PW_VIDEO_FORMAT_BGR24:
			bytes = 3;
			break;
		default:
			return -EINVAL;
	}
	if (frame->strides[0] < (uint32_t) frame->width * bytes)
		return -EINVAL;
	offset = (size_t) y * frame->strides[0] + (size_t) x * bytes;
	if (frame->sizes[0] && offset + bytes > frame->sizes[0])
		return -EINVAL;
	pixel = (const uint8_t*) frame->planes[0] + offset;

	switch (frame->format)
	{
		case DARLING_PW_VIDEO_FORMAT_BGRX:
			*blue = pixel[0]; *green = pixel[1]; *red = pixel[2]; return 0;
		case DARLING_PW_VIDEO_FORMAT_RGBX:
			*red = pixel[0]; *green = pixel[1]; *blue = pixel[2]; return 0;
		case DARLING_PW_VIDEO_FORMAT_XRGB:
			*red = pixel[1]; *green = pixel[2]; *blue = pixel[3]; return 0;
		case DARLING_PW_VIDEO_FORMAT_XBGR:
			*blue = pixel[1]; *green = pixel[2]; *red = pixel[3]; return 0;
		case DARLING_PW_VIDEO_FORMAT_RGB24:
			*red = pixel[0]; *green = pixel[1]; *blue = pixel[2]; return 0;
		case DARLING_PW_VIDEO_FORMAT_BGR24:
			*blue = pixel[0]; *green = pixel[1]; *red = pixel[2]; return 0;
		default:
			return -EINVAL;
	}
}

int darling_screen_capture_convert_bgra_rect(const struct darling_screen_capture_frame* frame,
	uint8_t* destination, uint32_t destination_stride,
	uint32_t surface_width, uint32_t surface_height,
	int32_t source_x, int32_t source_y, uint32_t source_width, uint32_t source_height,
	uint32_t destination_x, uint32_t destination_y,
	uint32_t destination_width, uint32_t destination_height)
{
	uint32_t x, y;

	if (!frame || !destination || !destination_width || !destination_height
		|| !source_width || !source_height || !frame->planes[0]
		|| !frame->width || !frame->height)
		return -EINVAL;
	if (source_x < 0 || source_y < 0
		|| (int64_t) source_x + source_width > frame->width
		|| (int64_t) source_y + source_height > frame->height)
		return -EINVAL;
	if ((uint64_t) destination_x + destination_width > surface_width
		|| (uint64_t) destination_y + destination_height > surface_height
		|| destination_stride < (uint64_t) surface_width * 4)
		return -EINVAL;

	for (y = 0; y < destination_height; y++)
	{
		uint32_t sample_y = (uint32_t) source_y
			+ (uint32_t) ((uint64_t) y * source_height / destination_height);
		uint8_t* row = destination + (size_t) (destination_y + y) * destination_stride
			+ (size_t) destination_x * 4;

		for (x = 0; x < destination_width; x++)
		{
			uint32_t sample_x = (uint32_t) source_x
				+ (uint32_t) ((uint64_t) x * source_width / destination_width);
			uint8_t red = 0, green = 0, blue = 0;
			int result = read_rgb_pixel(frame, sample_x, sample_y, &red, &green, &blue);

			if (result != 0)
				return result;
			row[x * 4 + 0] = blue;
			row[x * 4 + 1] = green;
			row[x * 4 + 2] = red;
			row[x * 4 + 3] = 255;
		}
	}
	return 0;
}

int darling_screen_capture_convert_bgra(const struct darling_screen_capture_frame* frame,
	uint8_t* destination, uint32_t destination_stride,
	uint32_t destination_width, uint32_t destination_height)
{
	return darling_screen_capture_convert_bgra_rect(frame, destination, destination_stride,
		destination_width, destination_height, 0, 0,
		frame ? frame->width : 0, frame ? frame->height : 0,
		0, 0, destination_width, destination_height);
}
