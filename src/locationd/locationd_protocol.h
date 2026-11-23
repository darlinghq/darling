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

#ifndef _LOCATIOND_PROTOCOL_H_
#define _LOCATIOND_PROTOCOL_H_

#define LOCATIOND_MACH_SERVICE "com.apple.locationd.desktop.registration"
#define LOCATIOND_SERVICE_ENV "__LOCATIOND_SERVICE_NAME"
#define LOCATIOND_DBUS_ADDRESS_ENV "__LOCATIOND_DBUS_ADDRESS"
#define LOCATIOND_SYSTEM_DBUS_ADDRESS_ENV "__LOCATIOND_SYSTEM_DBUS_ADDRESS"

// Operations from client to daemon
#define CL_OP_REGISTER          "register"
#define CL_OP_START_UPDATING    "start-updating"
#define CL_OP_STOP_UPDATING     "stop-updating"
#define CL_OP_REQUEST_LOCATION  "request-location"
#define CL_OP_REQUEST_AUTH      "request-auth"
#define CL_OP_GET_STATE         "get-state"

// Operations from daemon to client
#define CL_OP_LOCATION_UPDATE   "location-update"
#define CL_OP_AUTH_UPDATE       "auth-update"
#define CL_OP_ERROR             "error"

// Protocol Keys
#define CL_KEY_OP               "op"
#define CL_KEY_BUNDLE_ID        "bundle-id"
#define CL_KEY_PID              "pid"
#define CL_KEY_ENABLED          "enabled"              // bool: locationServicesEnabled
#define CL_KEY_AUTH_STATUS      "auth-status"          // int64: CLAuthorizationStatus
#define CL_KEY_ACCURACY_AUTH    "accuracy-auth"        // int64: CLAccuracyAuthorization
#define CL_KEY_DESIRED_ACCURACY "desired-accuracy"     // double
#define CL_KEY_DISTANCE_FILTER  "distance-filter"      // double
#define CL_KEY_AUTH_TYPE        "auth-type"            // int64: 1=whenInUse, 2=always
#define CL_KEY_ERROR_CODE       "error-code"           // int64: CLError code

// Location fields in updates
#define CL_KEY_LATITUDE         "latitude"             // double
#define CL_KEY_LONGITUDE        "longitude"            // double
#define CL_KEY_ALTITUDE         "altitude"             // double
#define CL_KEY_HORIZONTAL_ACC   "horizontal-acc"       // double
#define CL_KEY_VERTICAL_ACC     "vertical-acc"         // double
#define CL_KEY_COURSE           "course"               // double
#define CL_KEY_SPEED            "speed"                // double
#define CL_KEY_TIMESTAMP        "timestamp"            // double (seconds since 1970)

#endif
