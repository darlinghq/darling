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

#include "pipe_client.h"

#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

/* PipeWire's spa/utils/string.h uses the xlocale API in a static helper. The
 * Darwin SDK hides those declarations under its feature macros, and including
 * xlocale.h here would pull in SDK integer typedefs that clash with the host
 * headers used by spa. Only the declarations the header compiles against are
 * needed; the helper is not used by this backend. */
#if defined(__APPLE__) && !defined(_DARLING_PW_XLOCALE_SHIM)
#define _DARLING_PW_XLOCALE_SHIM
typedef void *locale_t;
#ifndef LC_ALL_MASK
#define LC_ALL_MASK 0
#endif
extern locale_t newlocale(int category_set, const char *locale, locale_t base);
extern locale_t uselocale(locale_t locale);
#endif

/* Newer libspa headers return Linux-only errno values (ECHRNG) from inline helpers
 * that this backend does not call; the Darwin SDK does not define them. */
#ifndef ECHRNG
#define ECHRNG 44
#endif

#include <pipewire/core.h>
#include <pipewire/keys.h>
#include <pipewire/loop.h>
#include <pipewire/node.h>
#include <pipewire/properties.h>
#include <pipewire/proxy.h>
#include <pipewire/stream.h>
#include <pipewire/thread-loop.h>
#include <pipewire/type.h>
#include <pipewire/version.h>
#include <spa/param/audio/format-utils.h>
#include <spa/param/audio/raw.h>
#include <spa/param/format-utils.h>
#include <spa/param/props.h>
#include <spa/param/video/format-utils.h>
#include <spa/param/video/raw.h>
#ifndef SPA_VIDEO_FORMAT_RGB24
#define SPA_VIDEO_FORMAT_RGB24 SPA_VIDEO_FORMAT_RGB
#endif
#ifndef SPA_VIDEO_FORMAT_BGR24
#define SPA_VIDEO_FORMAT_BGR24 SPA_VIDEO_FORMAT_BGR
#endif
#ifndef SPA_VIDEO_FORMAT_P010_10LE
#define SPA_VIDEO_FORMAT_P010_10LE SPA_VIDEO_FORMAT_UNKNOWN
#endif
#ifndef SPA_VIDEO_TRANSFER_SMPTE2084
#define SPA_VIDEO_TRANSFER_SMPTE2084 ((enum spa_video_transfer_function)16)
#endif
#ifndef SPA_VIDEO_TRANSFER_ARIB_STD_B67
#define SPA_VIDEO_TRANSFER_ARIB_STD_B67 ((enum spa_video_transfer_function)18)
#endif
#include <spa/buffer/meta.h>
#include <spa/pod/builder.h>
#include <spa/pod/parser.h>
#include <spa/utils/defs.h>
#include <spa/utils/dict.h>

struct darling_pw_stream
{
	struct pw_thread_loop* loop;
	struct pw_context* context;
	struct pw_core* core;
	struct pw_stream* stream;

	struct darling_pw_stream_config config;
	struct darling_pw_stream_events events;
	void* user_data;
	int state;
	size_t bytes_per_frame;

	pthread_mutex_t format_mutex;
	struct darling_pw_format format;
};

struct connection_probe
{
	struct pw_thread_loop* loop;
	int done;
	int connected;
};

static void probe_info(void* data, const struct pw_core_info* info)
{
	struct connection_probe* probe = data;
	(void) info;
	probe->connected = 1;
	probe->done = 1;
	pw_thread_loop_signal(probe->loop, false);
}

static void probe_failed(void* data, uint32_t id, int seq, int res, const char* message)
{
	struct connection_probe* probe = data;
	(void) id; (void) seq; (void) res; (void) message;
	probe->done = 1;
	pw_thread_loop_signal(probe->loop, false);
}

static void probe_removed(void* data, uint32_t id)
{
	struct connection_probe* probe = data;
	(void) id;
	probe->done = 1;
	pw_thread_loop_signal(probe->loop, false);
}

static const struct pw_core_events probe_events = {
	.version = PW_VERSION_CORE_EVENTS,
	.info = probe_info,
	.error = probe_failed,
	.remove_id = probe_removed,
};

/* libpipewire needs pw_init() before any loop or context is made; every entry point
 * that starts one calls this first. */
static pthread_once_t pw_init_once = PTHREAD_ONCE_INIT;

static void pw_init_run(void)
{
	pw_init(NULL, NULL);
}

static void ensure_pw_init(void)
{
	pthread_once(&pw_init_once, pw_init_run);
}

int darling_pw_can_connect(void)
{
	ensure_pw_init();
	struct pw_thread_loop* loop;
	struct pw_context* context;
	struct pw_core* core;
	struct spa_hook listener;
	struct connection_probe probe;
	int connected = 0, i;

	loop = pw_thread_loop_new("darling-pipewire-probe", NULL);
	if (!loop)
		return 0;

	context = pw_context_new(pw_thread_loop_get_loop(loop), NULL, 0);
	if (!context)
	{
		pw_thread_loop_destroy(loop);
		return 0;
	}
	core = pw_context_connect(context, NULL, 0);
	if (!core)
	{
		pw_context_destroy(context);
		pw_thread_loop_destroy(loop);
		return 0;
	}

	probe.loop = loop;
	probe.done = 0;
	probe.connected = 0;
	pw_core_add_listener(core, &listener, &probe_events, &probe);

	/* The loop thread starts only after everything is set up, and is stopped again before
	 * anything is torn down, so no libpipewire object is touched from two threads. */
	pw_thread_loop_lock(loop);
	if (pw_thread_loop_start(loop) < 0)
	{
		pw_thread_loop_unlock(loop);
		pw_core_disconnect(core);
		pw_context_destroy(context);
		pw_thread_loop_destroy(loop);
		return 0;
	}
	pw_core_sync(core, PW_ID_CORE, 1);
	for (i = 0; i < 3 && !probe.done; i++)
		pw_thread_loop_timed_wait(loop, 1);
	connected = probe.connected;
	pw_thread_loop_unlock(loop);
	pw_thread_loop_stop(loop);

	spa_hook_remove(&listener);
	pw_core_disconnect(core);
	pw_context_destroy(context);
	pw_thread_loop_destroy(loop);
	return connected;
}

struct source_probe
{
	struct pw_thread_loop* loop;
	struct darling_pw_video_source* out;
	uint32_t max;
	uint32_t count;
	int done;
};

static const char* dict_value(const struct spa_dict* dict, const char* key)
{
	const struct spa_dict_item* item = spa_dict_lookup_item(dict, key);
	return item ? item->value : NULL;
}

static void source_global(void* data, uint32_t id, uint32_t permissions,
	const char* type, uint32_t version, const struct spa_dict* props)
{
	struct source_probe* probe = data;
	const char* media_class;
	const char* name;

	(void) permissions; (void) version;
	if (!type || strcmp(type, PW_TYPE_INTERFACE_Node) != 0 || !props)
		return;
	media_class = dict_value(props, PW_KEY_MEDIA_CLASS);
	if (!media_class || strcmp(media_class, "Video/Source") != 0)
		return;
	if (probe->count >= probe->max)
		return;

	name = dict_value(props, PW_KEY_NODE_DESCRIPTION);
	if (!name)
		name = dict_value(props, PW_KEY_NODE_NAME);
	if (!name)
		name = "Camera";
	probe->out[probe->count].id = id;
	snprintf(probe->out[probe->count].name, sizeof(probe->out[probe->count].name), "%s", name);
	probe->count++;
}

static const struct pw_registry_events source_registry_events = {
	.version = PW_VERSION_REGISTRY_EVENTS,
	.global = source_global,
};

static void source_done(void* data, uint32_t id, int seq)
{
	struct source_probe* probe = data;
	(void) id; (void) seq;
	probe->done = 1;
	pw_thread_loop_signal(probe->loop, false);
}

static const struct pw_core_events source_core_events = {
	.version = PW_VERSION_CORE_EVENTS,
	.done = source_done,
};

int darling_pw_enumerate_video_sources(struct darling_pw_video_source* out, uint32_t max)
{
	struct pw_thread_loop* loop;
	struct pw_context* context;
	struct pw_core* core;
	struct pw_registry* registry;
	struct spa_hook core_listener, registry_listener;
	struct source_probe probe;
	int result = -ECONNREFUSED, i;

	if (!out || !max)
		return -EINVAL;
	ensure_pw_init();

	loop = pw_thread_loop_new("darling-pipewire-sources", NULL);
	if (!loop)
		return -ENOMEM;
	context = pw_context_new(pw_thread_loop_get_loop(loop), NULL, 0);
	if (!context)
		goto fail_thread;
	core = pw_context_connect(context, NULL, 0);
	if (!core)
		goto fail_context;

	memset(&probe, 0, sizeof(probe));
	probe.loop = loop;
	probe.out = out;
	probe.max = max;
	registry = pw_core_get_registry(core, PW_VERSION_REGISTRY, 0);
	if (!registry)
		goto fail_core;
	pw_core_add_listener(core, &core_listener, &source_core_events, &probe);
	pw_registry_add_listener(registry, &registry_listener, &source_registry_events, &probe);

	pw_thread_loop_lock(loop);
	if (pw_thread_loop_start(loop) < 0)
	{
		pw_thread_loop_unlock(loop);
		result = -ENOMEM;
		goto fail_registry;
	}
	pw_core_sync(core, PW_ID_CORE, 1);
	for (i = 0; i < 3 && !probe.done; i++)
		pw_thread_loop_timed_wait(loop, 1);
	pw_thread_loop_unlock(loop);
	pw_thread_loop_stop(loop);
	result = (int) probe.count;

fail_registry:
	spa_hook_remove(&registry_listener);
	spa_hook_remove(&core_listener);

	/* Destroy the local proxy; newer PipeWire versions also expose a remote
	 * registry.destroy method, but that is not what this cleanup needs. */
	pw_proxy_destroy((struct pw_proxy*) registry);
	pw_core_disconnect(core);
	goto fail_context;

fail_core:
	pw_core_disconnect(core);
fail_context:
	pw_context_destroy(context);
fail_thread:
	pw_thread_loop_destroy(loop);
	return result;
}

static const struct spa_pod* build_audio_param(struct spa_pod_builder* builder,
	const struct darling_pw_stream_config* config)
{
	struct spa_audio_info_raw info;
	uint32_t positions[2];

	memset(&info, 0, sizeof(info));
	info.format = SPA_AUDIO_FORMAT_F32;
	info.rate = config->rate ? config->rate : 44100;
	info.channels = config->channels ? config->channels : 2;
	if (info.channels == 1)
	{
		positions[0] = SPA_AUDIO_CHANNEL_MONO;
		info.position[0] = positions[0];
	}
	else if (info.channels == 2)
	{
		positions[0] = SPA_AUDIO_CHANNEL_FL;
		positions[1] = SPA_AUDIO_CHANNEL_FR;
		memcpy(info.position, positions, sizeof(positions));
	}
	else
		info.flags = SPA_AUDIO_FLAG_UNPOSITIONED;

	return spa_format_audio_raw_build(builder, SPA_PARAM_EnumFormat, &info);
}

/* Offer several raw formats so PipeWire can fixate on one the source actually
 * produces. All of them can be converted to the formats CoreVideo supports. */
static const uint32_t video_formats[] = {
	SPA_VIDEO_FORMAT_BGRx,
	SPA_VIDEO_FORMAT_RGBx,
	SPA_VIDEO_FORMAT_xRGB,
	SPA_VIDEO_FORMAT_xBGR,
	SPA_VIDEO_FORMAT_I420,
	SPA_VIDEO_FORMAT_NV12,
	SPA_VIDEO_FORMAT_YUY2,
	SPA_VIDEO_FORMAT_UYVY,
	SPA_VIDEO_FORMAT_RGB24,
	SPA_VIDEO_FORMAT_BGR24,
	SPA_VIDEO_FORMAT_P010_10LE,
};

#define DARLING_VIDEO_PARAM_COUNT (sizeof(video_formats) / sizeof(video_formats[0]))
#define DARLING_VIDEO_PARAM_SIZE 512

static int is_rgb_video_format(uint32_t format)
{
	switch (format)
	{
		case SPA_VIDEO_FORMAT_BGRx:
		case SPA_VIDEO_FORMAT_RGBx:
		case SPA_VIDEO_FORMAT_xRGB:
		case SPA_VIDEO_FORMAT_xBGR:
		case SPA_VIDEO_FORMAT_RGB24:
		case SPA_VIDEO_FORMAT_BGR24:
			return 1;
		default:
			return 0;
	}
}

static const struct spa_pod* build_video_param(struct spa_pod_builder* builder,
	const struct darling_pw_stream_config* config, uint32_t format)
{
	struct spa_pod_frame frame;
	struct spa_rectangle size, minimum, maximum;
	struct spa_fraction rate, min_rate, max_rate;

	size.width = config->width ? config->width : 640;
	size.height = config->height ? config->height : 480;
	minimum.width = 1;
	minimum.height = 1;
	maximum.width = 4096;
	maximum.height = 4096;
	rate.num = config->framerate ? config->framerate : 30;
	rate.denom = 1;
	min_rate.num = 0;
	min_rate.denom = 1;
	max_rate.num = config->framerate ? config->framerate * 4 : 120;
	max_rate.denom = 1;

	spa_pod_builder_push_object(builder, &frame, SPA_TYPE_OBJECT_Format, SPA_PARAM_EnumFormat);
	spa_pod_builder_add(builder,
		SPA_FORMAT_mediaType, SPA_POD_Id(SPA_MEDIA_TYPE_video),
		SPA_FORMAT_mediaSubtype, SPA_POD_Id(SPA_MEDIA_SUBTYPE_raw),
		SPA_FORMAT_VIDEO_format, SPA_POD_Id(format),
		SPA_FORMAT_VIDEO_size, SPA_POD_CHOICE_RANGE_Rectangle(&size, &minimum, &maximum),
		SPA_FORMAT_VIDEO_framerate, SPA_POD_CHOICE_RANGE_Fraction(&rate, &min_rate, &max_rate),
		0);
	if (config->allow_hdr && format == SPA_VIDEO_FORMAT_P010_10LE)
	{
		spa_pod_builder_add(builder,
			SPA_FORMAT_VIDEO_colorMatrix, SPA_POD_Id(SPA_VIDEO_COLOR_MATRIX_BT2020),
			SPA_FORMAT_VIDEO_transferFunction, SPA_POD_Id(SPA_VIDEO_TRANSFER_SMPTE2084),
			SPA_FORMAT_VIDEO_colorPrimaries, SPA_POD_Id(SPA_VIDEO_COLOR_PRIMARIES_BT2020),
			0);
	}
	return (const struct spa_pod*) spa_pod_builder_pop(builder, &frame);
}

static enum darling_pw_video_format video_format_of(uint32_t format)
{
	switch (format)
	{
		case SPA_VIDEO_FORMAT_BGRx: return DARLING_PW_VIDEO_FORMAT_BGRX;
		case SPA_VIDEO_FORMAT_RGBx: return DARLING_PW_VIDEO_FORMAT_RGBX;
		case SPA_VIDEO_FORMAT_xRGB: return DARLING_PW_VIDEO_FORMAT_XRGB;
		case SPA_VIDEO_FORMAT_xBGR: return DARLING_PW_VIDEO_FORMAT_XBGR;
		case SPA_VIDEO_FORMAT_RGB24: return DARLING_PW_VIDEO_FORMAT_RGB24;
		case SPA_VIDEO_FORMAT_BGR24: return DARLING_PW_VIDEO_FORMAT_BGR24;
		case SPA_VIDEO_FORMAT_I420: return DARLING_PW_VIDEO_FORMAT_I420;
		case SPA_VIDEO_FORMAT_NV12: return DARLING_PW_VIDEO_FORMAT_NV12;
		case SPA_VIDEO_FORMAT_YUY2: return DARLING_PW_VIDEO_FORMAT_YUY2;
		case SPA_VIDEO_FORMAT_UYVY: return DARLING_PW_VIDEO_FORMAT_UYVY;
		default:
			if (SPA_VIDEO_FORMAT_P010_10LE != SPA_VIDEO_FORMAT_UNKNOWN
				&& format == SPA_VIDEO_FORMAT_P010_10LE)
				return DARLING_PW_VIDEO_FORMAT_P010;
			return DARLING_PW_VIDEO_FORMAT_UNKNOWN;
	}
}

int darling_pw_runtime_version(uint32_t* major, uint32_t* minor, uint32_t* micro)
{
	const char* header = pw_get_headers_version();
	const char* library = pw_get_library_version();
	uint32_t maj = 0, min = 0, mic = 0;
	int have = 0;

	/* Prefer the runtime library version; fall back to headers when the
	 * runtime symbol is unavailable or returns an unexpected format. */
	if (library && sscanf(library, "%u.%u.%u", &maj, &min, &mic) == 3)
		have = 1;
	else if (header && sscanf(header, "%u.%u.%u", &maj, &min, &mic) == 3)
		have = 1;
	if (!have)
		return -ENOENT;
	if (major)
		*major = maj;
	if (minor)
		*minor = min;
	if (micro)
		*micro = mic;
	return 0;
}

int darling_pw_runtime_supports_hdr(void)
{
	uint32_t major = 0, minor = 0, micro = 0;

	/* PipeWire 1.6+ exposes the colorimetry/transfer metadata used for HDR
	 * capture streams; P010 is also needed for 10-bit HDR payloads. */
	if (SPA_VIDEO_FORMAT_P010_10LE == SPA_VIDEO_FORMAT_UNKNOWN)
		return 0;
	if (darling_pw_runtime_version(&major, &minor, &micro) != 0)
		return 0;
	return major > 1 || (major == 1 && minor >= 6);
}

static void on_param_changed(void* data, uint32_t id, const struct spa_pod* param)
{
	struct darling_pw_stream* stream = data;
	uint32_t media_type = 0, media_subtype = 0;

	if (id != SPA_PARAM_Format || !param)
		return;
	if (stream->events.param_changed)
		stream->events.param_changed(stream->user_data, id, param);
	if (spa_format_parse(param, &media_type, &media_subtype) < 0)
		return;
	if (media_type != SPA_MEDIA_TYPE_video || media_subtype != SPA_MEDIA_SUBTYPE_raw)
		return;

	struct spa_video_info_raw info;
	memset(&info, 0, sizeof(info));
	if (spa_format_video_raw_parse(param, &info) >= 0)
	{
		pthread_mutex_lock(&stream->format_mutex);
		stream->format.video_format = video_format_of(info.format);
		stream->format.width = info.size.width;
		stream->format.height = info.size.height;
		stream->format.framerate_num = info.framerate.num;
		stream->format.framerate_denom = info.framerate.denom;
		stream->format.transfer = DARLING_PW_TRANSFER_UNKNOWN;
		stream->format.primaries = DARLING_PW_PRIMARIES_UNKNOWN;
		switch (info.color_primaries)
		{
			case SPA_VIDEO_COLOR_PRIMARIES_BT709: stream->format.primaries = DARLING_PW_PRIMARIES_BT709; break;
			case SPA_VIDEO_COLOR_PRIMARIES_BT2020: stream->format.primaries = DARLING_PW_PRIMARIES_BT2020; break;
			default: break;
		}
		switch (info.transfer_function)
		{
			case SPA_VIDEO_TRANSFER_SRGB: stream->format.transfer = DARLING_PW_TRANSFER_SRGB; break;
			case SPA_VIDEO_TRANSFER_BT709: stream->format.transfer = DARLING_PW_TRANSFER_SDR_VIDEO; break;
			case SPA_VIDEO_TRANSFER_SMPTE2084: stream->format.transfer = DARLING_PW_TRANSFER_PQ; break;
			case SPA_VIDEO_TRANSFER_ARIB_STD_B67: stream->format.transfer = DARLING_PW_TRANSFER_HLG; break;
			default: break;
		}
		stream->format.hdr = stream->format.video_format == DARLING_PW_VIDEO_FORMAT_P010
			|| stream->format.transfer == DARLING_PW_TRANSFER_PQ
			|| stream->format.transfer == DARLING_PW_TRANSFER_HLG
			|| stream->format.primaries == DARLING_PW_PRIMARIES_BT2020;
		pthread_mutex_unlock(&stream->format_mutex);
	}
}

static void on_process(void* data)
{
	struct darling_pw_stream* stream = data;

	if (stream->events.process)
		stream->events.process(stream->user_data);
}

static void on_state_changed(void* data, enum pw_stream_state old_state,
	enum pw_stream_state state, const char* error)
{
	struct darling_pw_stream* stream = data;
	(void) old_state;
	(void) error;

	stream->state = (int) state;
	pw_thread_loop_signal(stream->loop, false);
	if (stream->events.state_changed)
		stream->events.state_changed(stream->user_data, (int) state);
}

static const struct pw_stream_events stream_events = {
	.version = PW_VERSION_STREAM_EVENTS,
	.process = on_process,
	.state_changed = on_state_changed,
	.param_changed = on_param_changed,
};

int darling_pw_stream_create(const struct darling_pw_stream_config* config,
	int remote_fd, const struct darling_pw_stream_events* events, void* user_data,
	struct darling_pw_stream** out)
{
	struct darling_pw_stream* stream;
	struct pw_properties* props;
	uint8_t audio_param_area[1024];
	uint8_t video_param_area[DARLING_VIDEO_PARAM_SIZE * DARLING_VIDEO_PARAM_COUNT];
	struct spa_pod_builder builder;
	const struct spa_pod* params[DARLING_VIDEO_PARAM_COUNT];
	size_t n_params = 0, i;
	uint32_t target_id;
	const char* name;
	int flags = PW_STREAM_FLAG_AUTOCONNECT | PW_STREAM_FLAG_MAP_BUFFERS;

	/* Ownership of remote_fd transfers to this function on entry: every exit
	 * path, including these early ones, is responsible for closing it. */
	if (!config || !out)
	{
		if (remote_fd >= 0)
			close(remote_fd);
		return -EINVAL;
	}
	*out = NULL;
	ensure_pw_init();

	stream = calloc(1, sizeof(*stream));
	if (!stream)
	{
		if (remote_fd >= 0)
			close(remote_fd);
		return -ENOMEM;
	}
	stream->config = *config;
	if (events)
		stream->events = *events;
	stream->user_data = user_data;
	stream->state = (int) PW_STREAM_STATE_UNCONNECTED;
	stream->format.framerate_denom = 1;
	stream->bytes_per_frame = config->media == DARLING_PW_MEDIA_AUDIO
		? (config->channels ? config->channels : 2) * sizeof(float) : 4;
	pthread_mutex_init(&stream->format_mutex, NULL);

	stream->loop = pw_thread_loop_new(config->name ? config->name : "darling-pipewire", NULL);
	if (!stream->loop)
		goto fail;

	stream->context = pw_context_new(pw_thread_loop_get_loop(stream->loop), NULL, 0);
	if (!stream->context)
		goto fail;

	stream->core = remote_fd >= 0
		? pw_context_connect_fd(stream->context, remote_fd, NULL, 0)
		: pw_context_connect(stream->context, NULL, 0);
	if (!stream->core)
		goto fail;

	/* A portal fd exposes one already-selected camera in a separate PipeWire
	 * graph. Node ids from the session registry have no meaning there. */
	target_id = remote_fd >= 0 ? 0 : config->target_id;
	remote_fd = -1;

	name = config->name ? config->name : "Darling";
	/* The stream is a client of the graph, not a device: PipeWire derives its
	 * media class (Stream/Output/Audio, Stream/Input/Video, ...) from the type,
	 * category and direction below. Declaring a device class such as Audio/Sink
	 * here would make the session manager treat the node as something to link
	 * other streams to, and nothing would ever be connected to it. */
	props = pw_properties_new(
		PW_KEY_NODE_NAME, name,
		PW_KEY_MEDIA_NAME, name,
		PW_KEY_MEDIA_TYPE, config->media == DARLING_PW_MEDIA_AUDIO ? "Audio" : "Video",
		PW_KEY_MEDIA_CATEGORY, config->capture ? "Capture" : "Playback",
		PW_KEY_MEDIA_ROLE, config->role ? config->role : (config->capture ? "capture" : "playback"),
		NULL);
	if (!props)
		goto fail;
	/* ScreenCast v6 streams are selected by object serial rather than by the
	 * node id returned in the request response. */
	if (config->target_object && config->target_object[0] != '\0')
		pw_properties_set(props, PW_KEY_TARGET_OBJECT, config->target_object);

	stream->stream = pw_stream_new_simple(pw_thread_loop_get_loop(stream->loop),
		name, props, &stream_events, stream);
	if (!stream->stream)
		goto fail;

	if (config->media == DARLING_PW_MEDIA_AUDIO)
	{
		builder = SPA_POD_BUILDER_INIT(audio_param_area, sizeof(audio_param_area));
		params[0] = build_audio_param(&builder, config);
		n_params = 1;
	}
	else
	{
		for (i = 0; i < DARLING_VIDEO_PARAM_COUNT; i++)
		{
			if (config->rgb_only && !is_rgb_video_format(video_formats[i]))
				continue;
			if (video_formats[i] == SPA_VIDEO_FORMAT_P010_10LE
				&& (!config->allow_hdr || !darling_pw_runtime_supports_hdr()))
				continue;
			builder = SPA_POD_BUILDER_INIT(video_param_area + i * DARLING_VIDEO_PARAM_SIZE,
				DARLING_VIDEO_PARAM_SIZE);
			params[n_params] = build_video_param(&builder, config, video_formats[i]);
			if (!params[n_params])
				goto fail;
			n_params++;
		}
	}
	if (!params[0])
		goto fail;

	if (pw_stream_connect(stream->stream,
		config->capture ? PW_DIRECTION_INPUT : PW_DIRECTION_OUTPUT,
		target_id ? target_id : PW_ID_ANY,
		(enum pw_stream_flags) flags, params, n_params) < 0)
		goto fail;

	/* Started last: everything above ran before the loop thread existed. */
	if (pw_thread_loop_start(stream->loop) < 0)
		goto fail;

	*out = stream;
	return 0;

fail:
	if (remote_fd >= 0)
		close(remote_fd);
	if (stream->stream)
		pw_stream_destroy(stream->stream);
	if (stream->core)
		pw_core_disconnect(stream->core);
	if (stream->context)
		pw_context_destroy(stream->context);
	if (stream->loop)
	{
		pw_thread_loop_stop(stream->loop);
		pw_thread_loop_destroy(stream->loop);
	}
	pthread_mutex_destroy(&stream->format_mutex);
	free(stream);
	return -errno;
}

void darling_pw_stream_destroy(struct darling_pw_stream* stream)
{
	if (!stream)
		return;

	pw_thread_loop_lock(stream->loop);
	pw_stream_destroy(stream->stream);
	pw_core_disconnect(stream->core);
	pw_thread_loop_unlock(stream->loop);

	pw_context_destroy(stream->context);
	pw_thread_loop_stop(stream->loop);
	pw_thread_loop_destroy(stream->loop);
	pthread_mutex_destroy(&stream->format_mutex);
	free(stream);
}

int darling_pw_stream_lock(struct darling_pw_stream* stream)
{
	if (!stream)
		return -EINVAL;
	pw_thread_loop_lock(stream->loop);
	return 0;
}

void darling_pw_stream_unlock(struct darling_pw_stream* stream)
{
	if (stream)
		pw_thread_loop_unlock(stream->loop);
}

int darling_pw_stream_wait(struct darling_pw_stream* stream, int timeout_ms)
{
	struct timespec deadline;
	int result;

	if (!stream)
		return -EINVAL;
	pw_thread_loop_lock(stream->loop);
	/* Every state change signals the loop, so the wait has to run against one
	 * deadline rather than counting wake-ups. */
	pw_thread_loop_get_time(stream->loop, &deadline,
		(int64_t) (timeout_ms > 0 ? timeout_ms : 0) * 1000000);
	while (stream->state != (int) PW_STREAM_STATE_STREAMING
		&& stream->state != (int) PW_STREAM_STATE_ERROR)
	{
		if (pw_thread_loop_timed_wait_full(stream->loop, &deadline) != 0)
			break;
	}
	result = stream->state == (int) PW_STREAM_STATE_STREAMING ? 0 : -ETIMEDOUT;
	pw_thread_loop_unlock(stream->loop);
	return result;
}

int darling_pw_stream_set_active(struct darling_pw_stream* stream, int active)
{
	int result;

	if (!stream)
		return -EINVAL;
	pw_thread_loop_lock(stream->loop);
	result = pw_stream_set_active(stream->stream, active != 0);
	pw_thread_loop_unlock(stream->loop);
	return result;
}

int darling_pw_stream_flush(struct darling_pw_stream* stream)
{
	int result;

	if (!stream)
		return -EINVAL;
	pw_thread_loop_lock(stream->loop);
	result = pw_stream_flush(stream->stream, false);
	pw_thread_loop_unlock(stream->loop);
	return result;
}

/* Some producers leave chunk.stride unset. Derive the minimum row size so the
 * converter can still address each row without touching adjacent planes. */
static uint32_t default_video_stride(const struct darling_pw_format* format, uint32_t plane)
{
	uint32_t width = format->width ? format->width : 640;
	uint32_t chroma = (width + 1) / 2;

	switch (format->video_format)
	{
		case DARLING_PW_VIDEO_FORMAT_I420:
			return plane == 0 ? width : chroma;
		case DARLING_PW_VIDEO_FORMAT_NV12:
			return plane == 0 ? width : chroma * 2;
		case DARLING_PW_VIDEO_FORMAT_P010:
			return plane == 0 ? width * 2 : chroma * 4;
		case DARLING_PW_VIDEO_FORMAT_YUY2:
		case DARLING_PW_VIDEO_FORMAT_UYVY:
			return width * 2;
		case DARLING_PW_VIDEO_FORMAT_RGB24:
		case DARLING_PW_VIDEO_FORMAT_BGR24:
			return width * 3;
		default:
			return width * 4;
	}
}

int darling_pw_stream_dequeue(struct darling_pw_stream* stream,
	struct darling_pw_buffer* out, void** cookie)
{
	struct pw_buffer* buffer;
	struct spa_data* data;
	const struct spa_meta_header* header;
	uint32_t i, planes;

	if (!stream || !out || !cookie)
		return -EINVAL;

	memset(out, 0, sizeof(*out));
	out->pts_ns = -1;

	buffer = pw_stream_dequeue_buffer(stream->stream);
	if (!buffer)
		return 0;
	data = &buffer->buffer->datas[0];
	if (!data->data)
	{
		pw_stream_queue_buffer(stream->stream, buffer);
		return 0;
	}

	planes = buffer->buffer->n_datas;
	if (planes > 4)
		planes = 4;
	for (i = 0; i < planes; i++)
	{
		struct spa_data* plane = &buffer->buffer->datas[i];
		uint32_t offset, available, size;

		if (!plane->data)
			continue;
		offset = plane->chunk ? plane->chunk->offset : 0;
		if (offset > plane->maxsize)
			offset = 0;
		available = plane->maxsize - offset;
		size = plane->chunk ? plane->chunk->size : available;
		if (size > available)
			size = available;
		out->planes[i] = (uint8_t *) plane->data + offset;
		out->plane_strides[i] = plane->chunk ? plane->chunk->stride : 0;
		if (!out->plane_strides[i] && stream->config.media == DARLING_PW_MEDIA_VIDEO)
			out->plane_strides[i] = default_video_stride(&stream->format, i);
		out->plane_sizes[i] = size;
		if (stream->config.capture)
			out->filled += size;
	}
	{
		uint32_t offset = data->chunk ? data->chunk->offset : 0;
		if (offset > data->maxsize)
			offset = 0;
		out->capacity = data->maxsize - offset;
	}
	out->data = out->planes[0];
	out->stride = out->plane_strides[0];
	out->requested_frames = buffer->requested > UINT32_MAX ? UINT32_MAX : (uint32_t) buffer->requested;

	/* Prefer the producer's presentation timestamp; fall back to the graph
	 * cycle time (also monotonic nanoseconds) when no header is enabled. */
	header = spa_buffer_find_meta_data(buffer->buffer, SPA_META_Header, sizeof(*header));
	if (header && header->pts >= 0)
	{
		out->pts_ns = header->pts;
		out->flags = header->flags;
	}
	else if (buffer->time > 0)
		out->pts_ns = (int64_t) buffer->time;

	*cookie = buffer;
	return 1;
}

int darling_pw_stream_queue(struct darling_pw_stream* stream, void* cookie, size_t used_bytes)
{
	struct pw_buffer* buffer = cookie;
	struct spa_data* data;

	if (!stream || !buffer)
		return -EINVAL;
	data = &buffer->buffer->datas[0];

	if (!stream->config.capture)
	{
		if (used_bytes > data->maxsize)
			used_bytes = data->maxsize;
		if (data->chunk)
		{
			data->chunk->offset = 0;
			data->chunk->size = used_bytes;
		}
		buffer->size = stream->bytes_per_frame ? used_bytes / stream->bytes_per_frame : 0;
	}
	else
	{
		buffer->size = 0;
	}
	return pw_stream_queue_buffer(stream->stream, buffer);
}

int darling_pw_stream_get_time(struct darling_pw_stream* stream, struct darling_pw_time* out)
{
	struct pw_time time;
	int result;

	if (!stream || !out)
		return -EINVAL;
	result = pw_stream_get_time_n(stream->stream, &time, sizeof(time));
	if (result < 0)
		return result;
	out->now_ns = time.now;
	out->ticks = time.ticks;
	out->rate_num = time.rate.num;
	out->rate_denom = time.rate.denom;
	out->delay_ticks = time.delay;
	return 0;
}

int darling_pw_stream_get_format(struct darling_pw_stream* stream, struct darling_pw_format* out)
{
	if (!stream || !out)
		return -EINVAL;
	pthread_mutex_lock(&stream->format_mutex);
	*out = stream->format;
	pthread_mutex_unlock(&stream->format_mutex);
	return 0;
}

int darling_pw_stream_state(struct darling_pw_stream* stream)
{
	return stream ? stream->state : (int) PW_STREAM_STATE_ERROR;
}
