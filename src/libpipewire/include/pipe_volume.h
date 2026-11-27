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

/*
 * Volume and mute of the default audio output or input node of the PipeWire
 * server, read from and written to the node's Props (channelVolumes, mute).
 * Like pipe_client.h this exposes no libpipewire or libspa types.
 */

#ifndef DARLING_PIPEWIRE_VOLUME_H_
#define DARLING_PIPEWIRE_VOLUME_H_

#ifdef __cplusplus
extern "C" {
#endif

struct darling_pw_volume;

struct darling_pw_volume_events
{
	/* Called on the PipeWire thread after the server reported a volume or mute
	 * value that differs from the one last seen. */
	void (*changed)(void* user_data);
};

/* output != 0 follows the default Audio/Sink node, otherwise the default
 * Audio/Source node. Returns 0, or a negative errno when no server can be
 * reached. */
int darling_pw_volume_open(int output, const struct darling_pw_volume_events* events,
	void* user_data, struct darling_pw_volume** out);
void darling_pw_volume_close(struct darling_pw_volume* volume);

/* Linear gain (the mean of the channel volumes) and mute of the followed node,
 * as last published by the server. Returns -ENODEV while no node is known. */
int darling_pw_volume_get(struct darling_pw_volume* volume, float* linear, int* mute);
int darling_pw_volume_set_linear(struct darling_pw_volume* volume, float linear);
int darling_pw_volume_set_mute(struct darling_pw_volume* volume, int mute);

#ifdef __cplusplus
}
#endif

#endif
