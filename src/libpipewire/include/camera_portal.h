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

#ifndef DARLING_CAMERA_PORTAL_H_
#define DARLING_CAMERA_PORTAL_H_

#include <darling_dbus.h>

#ifdef __cplusplus
extern "C" {
#endif

/* The subset of libdbus and the environment that the portal client needs.
 * Keeping it injectable lets the host test run against a private bus. */
struct darling_camera_portal_api
{
	const struct darling_dbus* dbus;
	char* (*getenv)(const char* name);
	int timeout_ms;
};

enum
{
	DARLING_CAMERA_PORTAL_OK = 0,
	DARLING_CAMERA_PORTAL_NOT_AVAILABLE = -1,
	DARLING_CAMERA_PORTAL_DENIED = -2,
	DARLING_CAMERA_PORTAL_FAILED = -3,
	DARLING_CAMERA_PORTAL_CANCELLED = -4,
};

/* org.freedesktop.portal.Camera.IsCameraPresent */
int darling_camera_portal_is_present(const struct darling_camera_portal_api* api);
/* org.freedesktop.portal.Camera.AccessCamera, including its Request.Response */
int darling_camera_portal_request_access(const struct darling_camera_portal_api* api);
/* org.freedesktop.portal.Camera.OpenPipeWireRemote; the returned fd is owned by the caller. */
int darling_camera_portal_open_remote(const struct darling_camera_portal_api* api, int* fd_out);

const char* darling_camera_portal_strerror(int result);

#ifdef __cplusplus
}
#endif

#endif
