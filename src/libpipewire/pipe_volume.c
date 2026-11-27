/*
 * This file is part of Darling.
 *
 * Copyright (C) 2026 Lubos Dolezel
 *
 * Darling is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 */

#include "pipe_volume.h"

#include <errno.h>
#include <math.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* See pipe_client.c for why these shims are needed. */
#if defined(__APPLE__) && !defined(_DARLING_PW_XLOCALE_SHIM)
#define _DARLING_PW_XLOCALE_SHIM
typedef void *locale_t;
#ifndef LC_ALL_MASK
#define LC_ALL_MASK 0
#endif
extern locale_t newlocale(int category_set, const char *locale, locale_t base);
extern locale_t uselocale(locale_t locale);
#endif
#ifndef ECHRNG
#define ECHRNG 44
#endif

#include <pipewire/core.h>
#include <pipewire/extensions/metadata.h>
#include <pipewire/keys.h>
#include <pipewire/node.h>
#include <pipewire/pipewire.h>
#include <pipewire/properties.h>
#include <pipewire/thread-loop.h>
#include <pipewire/type.h>
#include <spa/param/props.h>
#include <spa/pod/builder.h>
#include <spa/pod/iter.h>
#include <spa/utils/dict.h>

#define MAX_NODES 64
#define MAX_CHANNELS 32

struct volume_node
{
	uint32_t id;
	char name[256];
};

struct darling_pw_volume
{
	struct pw_thread_loop* loop;
	struct pw_context* context;
	struct pw_core* core;
	struct pw_registry* registry;
	struct spa_hook core_listener;
	struct spa_hook registry_listener;
	struct pw_metadata* metadata;
	struct spa_hook metadata_listener;
	struct pw_node* node;
	struct spa_hook node_listener;

	const char* media_class;
	const char* default_key;
	struct volume_node nodes[MAX_NODES];
	uint32_t node_count;
	char default_name[256];
	uint32_t bound_id;
	int done;

	/* Values as last published by the server (guarded by lock). */
	pthread_mutex_t lock;
	int have;
	float linear;
	int mute;
	uint32_t channels;

	struct darling_pw_volume_events events;
	void* user_data;
};

static pthread_once_t volume_init_once = PTHREAD_ONCE_INIT;

static void volume_init_run(void)
{
	pw_init(NULL, NULL);
}

static void notify_changed(struct darling_pw_volume* volume)
{
	if (volume->events.changed)
		volume->events.changed(volume->user_data);
}

static void node_param(void* data, int seq, uint32_t id, uint32_t index, uint32_t next,
	const struct spa_pod* param)
{
	struct darling_pw_volume* volume = data;
	const struct spa_pod_prop* prop;
	float volumes[MAX_CHANNELS];
	uint32_t count = 0;
	int have_volume = 0, have_mute = 0, mute = 0, changed = 0;
	float linear = 0;

	(void) seq; (void) index; (void) next;
	if (id != SPA_PARAM_Props || !param || !spa_pod_is_object(param))
		return;
	SPA_POD_OBJECT_FOREACH((const struct spa_pod_object*) param, prop)
	{
		bool flag;
		switch (prop->key)
		{
			case SPA_PROP_channelVolumes:
				count = spa_pod_copy_array(&prop->value, SPA_TYPE_Float, volumes, MAX_CHANNELS);
				have_volume = count > 0;
				break;
			case SPA_PROP_mute:
				if (spa_pod_get_bool(&prop->value, &flag) == 0)
				{
					mute = flag ? 1 : 0;
					have_mute = 1;
				}
				break;
		}
	}
	if (have_volume)
	{
		float sum = 0;
		for (uint32_t i = 0; i < count; i++)
			sum += volumes[i];
		linear = sum / (float) count;
	}

	pthread_mutex_lock(&volume->lock);
	if (have_volume && (!volume->have || fabsf(volume->linear - linear) > 1e-5f))
	{
		volume->linear = linear;
		changed = 1;
	}
	if (have_volume)
		volume->channels = count;
	if (have_mute && (!volume->have || volume->mute != mute))
	{
		volume->mute = mute;
		changed = 1;
	}
	if (have_volume || have_mute)
		volume->have = 1;
	pthread_mutex_unlock(&volume->lock);
	if (changed)
		notify_changed(volume);
}

static const struct pw_node_events node_events = {
	.version = PW_VERSION_NODE_EVENTS,
	.param = node_param,
};

static void unbind_node(struct darling_pw_volume* volume)
{
	if (volume->node)
	{
		spa_hook_remove(&volume->node_listener);
		pw_proxy_destroy((struct pw_proxy*) volume->node);
		volume->node = NULL;
	}
	volume->bound_id = 0;
	pthread_mutex_lock(&volume->lock);
	volume->have = 0;
	pthread_mutex_unlock(&volume->lock);
}

/* Follows the server's default node of the media class; without a default
 * (no session manager) the first node of the class is used. */
static void reselect(struct darling_pw_volume* volume)
{
	uint32_t wanted = 0;

	for (uint32_t i = 0; i < volume->node_count; i++)
	{
		if (volume->default_name[0] && strcmp(volume->nodes[i].name, volume->default_name) == 0)
		{
			wanted = volume->nodes[i].id;
			break;
		}
	}
	if (!wanted && !volume->default_name[0] && volume->node_count > 0)
		wanted = volume->nodes[0].id;
	if (wanted == volume->bound_id)
		return;

	unbind_node(volume);
	if (!wanted)
		return;
	volume->node = pw_registry_bind(volume->registry, wanted, PW_TYPE_INTERFACE_Node,
		PW_VERSION_NODE, 0);
	if (!volume->node)
		return;
	volume->bound_id = wanted;
	pw_node_add_listener(volume->node, &volume->node_listener, &node_events, volume);
	{
		uint32_t ids[1] = { SPA_PARAM_Props };
		pw_node_subscribe_params(volume->node, ids, 1);
	}
}

/* The default.* metadata value is a JSON object such as { "name": "sink0" }. */
static void parse_default_name(const char* value, char* out, size_t size)
{
	const char* p;
	const char* end;

	out[0] = 0;
	if (!value)
		return;
	p = strstr(value, "\"name\"");
	if (!p)
		return;
	p = strchr(p + 6, ':');
	if (!p)
		return;
	p = strchr(p, '"');
	if (!p)
		return;
	p++;
	end = strchr(p, '"');
	if (!end || (size_t) (end - p) >= size)
		return;
	memcpy(out, p, (size_t) (end - p));
	out[end - p] = 0;
}

static int metadata_property(void* data, uint32_t subject, const char* key, const char* type,
	const char* value)
{
	struct darling_pw_volume* volume = data;

	(void) type;
	if (subject == PW_ID_CORE && key && strcmp(key, volume->default_key) == 0)
	{
		parse_default_name(value, volume->default_name, sizeof(volume->default_name));
		reselect(volume);
	}
	return 0;
}

static const struct pw_metadata_events metadata_events = {
	.version = PW_VERSION_METADATA_EVENTS,
	.property = metadata_property,
};

static const char* dict_value(const struct spa_dict* dict, const char* key)
{
	const struct spa_dict_item* item = spa_dict_lookup_item(dict, key);
	return item ? item->value : NULL;
}

static void registry_global(void* data, uint32_t id, uint32_t permissions,
	const char* type, uint32_t version, const struct spa_dict* props)
{
	struct darling_pw_volume* volume = data;
	const char* media_class;
	const char* name;

	(void) permissions; (void) version;
	if (!type || !props)
		return;
	if (strcmp(type, PW_TYPE_INTERFACE_Node) == 0)
	{
		media_class = dict_value(props, PW_KEY_MEDIA_CLASS);
		name = dict_value(props, PW_KEY_NODE_NAME);
		if (!media_class || strcmp(media_class, volume->media_class) != 0 || !name
			|| volume->node_count >= MAX_NODES)
			return;
		volume->nodes[volume->node_count].id = id;
		snprintf(volume->nodes[volume->node_count].name, sizeof(volume->nodes[0].name), "%s", name);
		volume->node_count++;
		reselect(volume);
	}
	else if (strcmp(type, PW_TYPE_INTERFACE_Metadata) == 0 && !volume->metadata)
	{
		name = dict_value(props, PW_KEY_METADATA_NAME);
		if (!name || strcmp(name, "default") != 0)
			return;
		volume->metadata = pw_registry_bind(volume->registry, id, type, PW_VERSION_METADATA, 0);
		if (volume->metadata)
			pw_metadata_add_listener(volume->metadata, &volume->metadata_listener,
				&metadata_events, volume);
	}
}

static void registry_global_remove(void* data, uint32_t id)
{
	struct darling_pw_volume* volume = data;

	for (uint32_t i = 0; i < volume->node_count; i++)
	{
		if (volume->nodes[i].id == id)
		{
			volume->nodes[i] = volume->nodes[--volume->node_count];
			reselect(volume);
			return;
		}
	}
}

static const struct pw_registry_events registry_events = {
	.version = PW_VERSION_REGISTRY_EVENTS,
	.global = registry_global,
	.global_remove = registry_global_remove,
};

static void core_done(void* data, uint32_t id, int seq)
{
	struct darling_pw_volume* volume = data;

	(void) id; (void) seq;
	volume->done = 1;
	pw_thread_loop_signal(volume->loop, false);
}

static const struct pw_core_events core_events = {
	.version = PW_VERSION_CORE_EVENTS,
	.done = core_done,
};

/* Called with the loop locked. */
static void roundtrip(struct darling_pw_volume* volume)
{
	volume->done = 0;
	pw_core_sync(volume->core, PW_ID_CORE, 0);
	for (int i = 0; i < 3 && !volume->done; i++)
		pw_thread_loop_timed_wait(volume->loop, 1);
}

int darling_pw_volume_open(int output, const struct darling_pw_volume_events* events,
	void* user_data, struct darling_pw_volume** out)
{
	struct darling_pw_volume* volume;

	if (!out)
		return -EINVAL;
	*out = NULL;
	pthread_once(&volume_init_once, volume_init_run);
	volume = calloc(1, sizeof(*volume));
	if (!volume)
		return -ENOMEM;
	pthread_mutex_init(&volume->lock, NULL);
	volume->media_class = output ? "Audio/Sink" : "Audio/Source";
	volume->default_key = output ? "default.audio.sink" : "default.audio.source";
	volume->channels = 2;
	if (events)
		volume->events = *events;
	volume->user_data = user_data;

	volume->loop = pw_thread_loop_new("darling-pipewire-volume", NULL);
	if (!volume->loop)
		goto fail;
	volume->context = pw_context_new(pw_thread_loop_get_loop(volume->loop), NULL, 0);
	if (!volume->context)
		goto fail;
	volume->core = pw_context_connect(volume->context, NULL, 0);
	if (!volume->core)
		goto fail;
	pw_core_add_listener(volume->core, &volume->core_listener, &core_events, volume);
	volume->registry = pw_core_get_registry(volume->core, PW_VERSION_REGISTRY, 0);
	if (!volume->registry)
		goto fail;
	pw_registry_add_listener(volume->registry, &volume->registry_listener,
		&registry_events, volume);

	pw_thread_loop_lock(volume->loop);
	if (pw_thread_loop_start(volume->loop) < 0)
	{
		pw_thread_loop_unlock(volume->loop);
		goto fail;
	}
	/* One round trip lists the nodes and the default metadata, the second one
	 * delivers the Props of the node that was selected from them. */
	roundtrip(volume);
	roundtrip(volume);
	pw_thread_loop_unlock(volume->loop);
	*out = volume;
	return 0;

fail:
	if (volume->core)
		pw_core_disconnect(volume->core);
	if (volume->context)
		pw_context_destroy(volume->context);
	if (volume->loop)
		pw_thread_loop_destroy(volume->loop);
	pthread_mutex_destroy(&volume->lock);
	free(volume);
	return -ECONNREFUSED;
}

void darling_pw_volume_close(struct darling_pw_volume* volume)
{
	if (!volume)
		return;
	pw_thread_loop_lock(volume->loop);
	volume->events.changed = NULL;
	unbind_node(volume);
	if (volume->metadata)
	{
		spa_hook_remove(&volume->metadata_listener);
		pw_proxy_destroy((struct pw_proxy*) volume->metadata);
	}
	spa_hook_remove(&volume->registry_listener);
	pw_proxy_destroy((struct pw_proxy*) volume->registry);
	spa_hook_remove(&volume->core_listener);
	pw_core_disconnect(volume->core);
	pw_thread_loop_unlock(volume->loop);
	pw_context_destroy(volume->context);
	pw_thread_loop_stop(volume->loop);
	pw_thread_loop_destroy(volume->loop);
	pthread_mutex_destroy(&volume->lock);
	free(volume);
}

int darling_pw_volume_get(struct darling_pw_volume* volume, float* linear, int* mute)
{
	int have;

	if (!volume)
		return -EINVAL;
	pthread_mutex_lock(&volume->lock);
	have = volume->have;
	if (have)
	{
		if (linear)
			*linear = volume->linear;
		if (mute)
			*mute = volume->mute;
	}
	pthread_mutex_unlock(&volume->lock);
	return have ? 0 : -ENODEV;
}

static int set_props(struct darling_pw_volume* volume, int set_volume, float linear, int mute)
{
	uint8_t buffer[512];
	struct spa_pod_builder builder = SPA_POD_BUILDER_INIT(buffer, sizeof(buffer));
	float volumes[MAX_CHANNELS];
	uint32_t channels;
	struct spa_pod* pod;
	int result = -ENODEV;

	pthread_mutex_lock(&volume->lock);
	channels = volume->channels ? volume->channels : 2;
	pthread_mutex_unlock(&volume->lock);
	if (channels > MAX_CHANNELS)
		channels = MAX_CHANNELS;
	for (uint32_t i = 0; i < channels; i++)
		volumes[i] = linear;

	if (set_volume)
		pod = spa_pod_builder_add_object(&builder, SPA_TYPE_OBJECT_Props, SPA_PARAM_Props,
			SPA_PROP_channelVolumes, SPA_POD_Array(sizeof(float), SPA_TYPE_Float, channels, volumes));
	else
		pod = spa_pod_builder_add_object(&builder, SPA_TYPE_OBJECT_Props, SPA_PARAM_Props,
			SPA_PROP_mute, SPA_POD_Bool(mute != 0));

	pw_thread_loop_lock(volume->loop);
	if (volume->node)
	{
		pw_node_set_param(volume->node, SPA_PARAM_Props, 0, pod);
		result = 0;
	}
	pw_thread_loop_unlock(volume->loop);
	if (result == 0)
	{
		/* The server echoes the change, which is then not new. */
		pthread_mutex_lock(&volume->lock);
		if (set_volume)
			volume->linear = linear;
		else
			volume->mute = mute != 0;
		pthread_mutex_unlock(&volume->lock);
	}
	return result;
}

int darling_pw_volume_set_linear(struct darling_pw_volume* volume, float linear)
{
	if (!volume)
		return -EINVAL;
	return set_props(volume, 1, linear < 0 ? 0 : linear, 0);
}

int darling_pw_volume_set_mute(struct darling_pw_volume* volume, int mute)
{
	if (!volume)
		return -EINVAL;
	return set_props(volume, 0, 0, mute);
}
