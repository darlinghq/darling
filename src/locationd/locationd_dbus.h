/*
 This file is part of Darling.

 Copyright (C) 2026 Darling developers

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

#ifndef _LOCATIOND_DBUS_H_
#define _LOCATIOND_DBUS_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

struct locationd_fix {
    double latitude;
    double longitude;
    double altitude;
    double horizontal_accuracy;
    double vertical_accuracy;
    double course;
    double speed;
    double timestamp; // Unix timestamp in seconds
    int has_altitude;
    int has_course;
    int has_speed;
};

enum locationd_backend_type {
    LOCATIOND_BACKEND_NONE = 0,
    LOCATIOND_BACKEND_PORTAL = 1,
    LOCATIOND_BACKEND_GEOCLUE = 2,
};

enum locationd_auth_state {
    LOCATIOND_AUTH_NOT_DETERMINED = 0,
    LOCATIOND_AUTH_RESTRICTED = 1,
    LOCATIOND_AUTH_DENIED = 2,
    LOCATIOND_AUTH_AUTHORIZED = 3,
};

typedef void (*locationd_on_fix_fn)(const struct locationd_fix* fix, void* context);
typedef void (*locationd_on_auth_fn)(enum locationd_auth_state status, void* context);
typedef void (*locationd_on_error_fn)(int error_code, void* context);

int locationd_dbus_init(const char* session_bus_address, const char* system_bus_address);
int locationd_dbus_is_available(void);
enum locationd_backend_type locationd_dbus_get_backend(void);
int locationd_dbus_start(double desired_accuracy, double distance_threshold,
                         locationd_on_fix_fn on_fix, locationd_on_auth_fn on_auth,
                         locationd_on_error_fn on_error, void* context);
void locationd_dbus_stop(void);
int locationd_dbus_update_parameters(double desired_accuracy, double distance_threshold);
int locationd_dbus_get_fd(void);
int locationd_dbus_process_messages(int timeout_ms);
void locationd_dbus_close(void);

#ifdef __cplusplus
}
#endif

#endif
