/*
 * Copyright (C) 2026 Darling developers
 *
 * Darling is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Darling is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Darling.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef SETTINGSD_DBUS_H
#define SETTINGSD_DBUS_H

#ifdef __cplusplus
extern "C" {
#endif

// Callback for appearance changes received from portal signal.
// has_dark: 1 if dark mode value provided, 0 otherwise. dark: 1 if dark, 0 if light.
// has_contrast: 1 if contrast value provided, 0 otherwise. contrast: 1 if higher contrast, 0 if normal.
// has_accent: 1 if accent color provided, 0 otherwise. r, g, b: normalized sRGB [0.0, 1.0].
typedef void (*settingsd_change_cb)(int has_dark, int dark,
                                    int has_contrast, int contrast,
                                    int has_accent, double r, double g, double b);

// Initializes D-Bus and attempts connection to session bus.
// Returns 0 on success, negative error code on failure.
int settingsd_dbus_init(void);

// Returns the underlying Unix file descriptor of the D-Bus connection.
// Returns 0 on success, negative error code on failure.
int settingsd_dbus_get_fd(int *out_fd);

// Queries the current appearance settings from the portal.
// Returns 0 on success (portal responded), negative error code on failure.
int settingsd_dbus_read_appearance(int *out_has_dark, int *out_dark,
                                   int *out_has_contrast, int *out_contrast,
                                   int *out_has_accent, double *out_r, double *out_g, double *out_b);

// Processes pending D-Bus messages / signals with timeout in ms.
// Returns number of processed messages on success, or -1 on disconnect/fatal error.
int settingsd_dbus_process_messages(int timeout_ms, settingsd_change_cb callback);

// Closes D-Bus connection and resets connection state.
void settingsd_dbus_close(void);

#ifdef __cplusplus
}
#endif

#endif // SETTINGSD_DBUS_H
