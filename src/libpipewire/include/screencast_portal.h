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

#ifndef DARLING_SCREENCAST_PORTAL_H_
#define DARLING_SCREENCAST_PORTAL_H_

#include <stdint.h>

#include <darling_dbus.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Values of org.freedesktop.portal.ScreenCast.AvailableSourceTypes. */
enum
{
	DARLING_SCREENCAST_SOURCE_MONITOR = 1u << 0,
	DARLING_SCREENCAST_SOURCE_WINDOW = 1u << 1,
	DARLING_SCREENCAST_SOURCE_VIRTUAL = 1u << 2,
};

/* Values of org.freedesktop.portal.ScreenCast.AvailableCursorModes. */
enum
{
	DARLING_SCREENCAST_CURSOR_HIDDEN = 1u << 0,
	DARLING_SCREENCAST_CURSOR_EMBEDDED = 1u << 1,
	DARLING_SCREENCAST_CURSOR_METADATA = 1u << 2,
};

enum
{
	DARLING_SCREENCAST_OK = 0,
	DARLING_SCREENCAST_NOT_AVAILABLE = -1,
	DARLING_SCREENCAST_DENIED = -2,
	DARLING_SCREENCAST_FAILED = -3,
	DARLING_SCREENCAST_CANCELLED = -4,
	DARLING_SCREENCAST_NO_STREAMS = -5,
};

/* The injected libdbus table and environment, laid out like the other portal
 * clients so callers can share one setup function. */
struct darling_screencast_portal_api
{
	const struct darling_dbus* dbus;
	char* (*getenv)(const char* name);
	int timeout_ms;
};

struct darling_screencast_config
{
	/* Bitmask of DARLING_SCREENCAST_SOURCE_*. Zero means MONITOR. */
	uint32_t source_types;
	/* One of DARLING_SCREENCAST_CURSOR_*. Zero means HIDDEN. */
	uint32_t cursor_mode;
	/* Allow the user to select more than one stream. */
	int multiple;
	/* Portal persist_mode: 0 never, 1 while the caller runs, 2 until revoked. */
	uint32_t persist_mode;
	/* Optional single-use token from a previous session. */
	const char* restore_token;
};

struct darling_screencast_stream
{
	uint32_t node_id;
	uint64_t serial;
	uint32_t source_type;
	int32_t x;
	int32_t y;
	int32_t width;
	int32_t height;
	char id[128];
	/* String form of serial (preferred) or node_id, for PW_KEY_TARGET_OBJECT. */
	char target_object[48];
};

struct darling_screencast_session;

/* Reads the read-only properties used to choose legal SelectSources options. */
int darling_screencast_portal_properties(const struct darling_screencast_portal_api* api,
	uint32_t* source_types, uint32_t* cursor_modes, uint32_t* version);

/* Runs CreateSession, SelectSources, Start and OpenPipeWireRemote. The session
 * owns the PipeWire remote fd and keeps the portal connection alive. */
int darling_screencast_session_create(const struct darling_screencast_portal_api* api,
	const struct darling_screencast_config* config, struct darling_screencast_session** out);

/* Returns the PipeWire remote fd owned by the session, or -1. */
int darling_screencast_session_fd(const struct darling_screencast_session* session);

/* Returns the streams announced by Start(). The returned array is owned by the
 * session and stays valid until it is destroyed. */
const struct darling_screencast_stream* darling_screencast_session_streams(
	const struct darling_screencast_session* session, uint32_t* count);

/* Calls Session.Close and releases all local resources. */
void darling_screencast_session_destroy(struct darling_screencast_session* session);

const char* darling_screencast_strerror(int result);

#ifdef __cplusplus
}
#endif

#endif
