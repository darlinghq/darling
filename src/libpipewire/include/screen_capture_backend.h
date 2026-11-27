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

#ifndef DARLING_SCREEN_CAPTURE_BACKEND_H_
#define DARLING_SCREEN_CAPTURE_BACKEND_H_

#include <stddef.h>
#include <stdint.h>

#include <pipe_client.h>

#ifdef __cplusplus
extern "C" {
#endif

struct darling_screen_capture_stream;

struct darling_screen_capture_frame
{
	int64_t pts_ns;
	uint32_t flags;
	enum darling_pw_video_format format;
	uint32_t width;
	uint32_t height;
	void* planes[4];
	uint32_t strides[4];
	size_t sizes[4];
	enum darling_pw_transfer_function transfer;
	enum darling_pw_color_primaries primaries;
	int hdr;
};

struct darling_screen_capture_callbacks
{
	/* Called on the capture pump thread with the stream lock held. The frame's
	 * memory is only valid for the duration of the callback. */
	void (*frame)(const struct darling_screen_capture_frame* frame, void* user_data);
	void (*error)(int error, void* user_data);
};

/* Non-zero when a ScreenCast portal can be reached. This does not ask for
 * permission and does not report whether permission has been granted. */
int darling_screen_capture_available(void);

/* True after this process has obtained a ScreenCast stream successfully. */
int darling_screen_capture_permitted(void);

/* Runs one portal session to obtain the user's consent, then caches the
 * session for later streams. Returns 0 or a negative errno. */
int darling_screen_capture_request_access(void);

/* Opens a stream for the display whose compositor-space rectangle best matches
 * x/y/width/height. Zero dimensions or an unknown rectangle mean "any". */
int darling_screen_capture_open(int32_t x, int32_t y, uint32_t width, uint32_t height,
	int show_cursor, int allow_hdr,
	const struct darling_screen_capture_callbacks* callbacks,
	void* user_data, struct darling_screen_capture_stream** out);
int darling_screen_capture_start(struct darling_screen_capture_stream* stream, int active);
int darling_screen_capture_wait(struct darling_screen_capture_stream* stream, int timeout_ms);
struct darling_pw_stream* darling_screen_capture_pipe(struct darling_screen_capture_stream* stream);
void darling_screen_capture_close(struct darling_screen_capture_stream* stream);

/* Converts a capture frame to BGRA with a nearest-neighbour scale. The source
 * rectangle is in frame coordinates; the destination rectangle is in the
 * caller's surface. Either rectangle may crop or scale the frame. Returns 0 on
 * success or a negative errno when the format is unsupported or an argument is
 * invalid. */
int darling_screen_capture_convert_bgra_rect(const struct darling_screen_capture_frame* frame,
	uint8_t* destination, uint32_t destination_stride,
	uint32_t surface_width, uint32_t surface_height,
	int32_t source_x, int32_t source_y, uint32_t source_width, uint32_t source_height,
	uint32_t destination_x, uint32_t destination_y,
	uint32_t destination_width, uint32_t destination_height);

int darling_screen_capture_convert_bgra(const struct darling_screen_capture_frame* frame,
	uint8_t* destination, uint32_t destination_stride,
	uint32_t destination_width, uint32_t destination_height);

/* Returns non-zero when the linked PipeWire runtime is new enough and exposes
 * 10-bit HDR formats needed for HDR screen capture. */
int darling_screen_capture_supports_hdr(void);

#ifdef __cplusplus
}
#endif

#endif
