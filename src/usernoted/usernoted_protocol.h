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

#ifndef _USERNOTED_PROTOCOL_H_
#define _USERNOTED_PROTOCOL_H_

#define USERNOTED_MACH_SERVICE "com.apple.usernoted"
#define USERNOTED_SERVICE_ENV "__USERNOTED_SERVICE_NAME"

// XPC protocol keys
#define UN_KEY_OP "darling-op"
#define UN_KEY_ERR "err"
#define UN_KEY_BUNDLE_ID "bundle-id"
#define UN_KEY_BUNDLE_PATH "bundle-path"
#define UN_KEY_EXEC_PATH "exec-path"
#define UN_KEY_PID "pid"
#define UN_KEY_NOTIFICATION_ID "notification-id"
#define UN_KEY_REPLACES_ID "replaces-id"
#define UN_KEY_TITLE "title"
#define UN_KEY_SUBTITLE "subtitle"
#define UN_KEY_BODY "body"
#define UN_KEY_APP_ICON "app-icon"
#define UN_KEY_ACTIONS "actions"              // array of string pairs: [id, title, id, title...]
#define UN_KEY_IMAGE_PATH "image-path"
#define UN_KEY_SOUND_FILE "sound-file"
#define UN_KEY_URGENCY "urgency"              // int64 (0: low, 1: normal, 2: critical)
#define UN_KEY_CATEGORY_ID "category-id"
#define UN_KEY_CUSTOM_DISMISS "custom-dismiss" // bool
#define UN_KEY_IS_NSUSER "is-nsuser"          // bool (NSUserNotification vs UserNotifications)
#define UN_KEY_USER_INFO "user-info"          // serialized data
#define UN_KEY_TRIGGER_TYPE "trigger-type"    // 0: immediate, 1: time-interval, 2: calendar
#define UN_KEY_TIME_INTERVAL "time-interval"  // double
#define UN_KEY_REPEATS "repeats"              // bool
#define UN_KEY_FIRE_DATE "fire-date"          // double (epoch seconds)
#define UN_KEY_DELIVERED_NOTIFICATIONS "delivered-notifications" // array of dicts
#define UN_KEY_PENDING_REQUESTS "pending-requests"               // array of dicts
#define UN_KEY_ACTION_KEY "action-key"        // string ("default" or action identifier)
#define UN_KEY_REASON "reason"                // int64 (close reason)
#define UN_KEY_IDENTIFIERS "identifiers"      // array of string identifiers
#define UN_KEY_FOREGROUND "foreground"        // bool (will-present reply: the app is the active application)
#define UN_KEY_PRESENT_OPTIONS "present-options" // int64 (will-present reply: UNNotificationPresentationOptions)

// Operations from client to daemon:
#define UN_OP_CHECKIN "checkin"
#define UN_OP_POST "post"
#define UN_OP_REMOVE_DELIVERED "remove-delivered"
#define UN_OP_REMOVE_ALL_DELIVERED "remove-all-delivered"
#define UN_OP_REMOVE_PENDING "remove-pending"
#define UN_OP_REMOVE_ALL_PENDING "remove-all-pending"
#define UN_OP_GET_DELIVERED "get-delivered"
#define UN_OP_GET_PENDING "get-pending"
#define UN_OP_GET_SETTINGS "get-settings"
#define UN_OP_REQUEST_AUTH "request-auth"
#define UN_OP_SET_CATEGORIES "set-categories"
#define UN_OP_GET_CATEGORIES "get-categories"

// Operations from daemon to client:
#define UN_OP_ACTION_INVOKED "action-invoked"
#define UN_OP_NOTIFICATION_CLOSED "notification-closed"
// The daemon is about to present a notification of a running app and asks the app
// (with a reply) whether it is in the foreground and, if so, how to present it.
#define UN_OP_WILL_PRESENT "will-present"

// UNNotificationPresentationOptions bits, as in the public UserNotifications API
#define UN_PRESENT_BADGE  (1 << 0)
#define UN_PRESENT_SOUND  (1 << 1)
#define UN_PRESENT_ALERT  (1 << 2)
#define UN_PRESENT_LIST   (1 << 3)
#define UN_PRESENT_BANNER (1 << 4)
// the options that put something on screen (as opposed to badge and sound only)
#define UN_PRESENT_VISIBLE (UN_PRESENT_ALERT | UN_PRESENT_LIST | UN_PRESENT_BANNER)

#endif
