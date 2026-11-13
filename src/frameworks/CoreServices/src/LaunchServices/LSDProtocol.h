/*
 This file is part of Darling.

 Copyright (C) 2026 Lubos Dolezel

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

// Requests that CoreServices clients (LaunchServices, AE) send to launchservicesd over XPC.
// Every request is a dictionary whose "darling-op" string names the operation. Every reply
// carries "err" (an OSStatus, 0 on success). This is Darling's own protocol and is not meant
// to be seen by applications.
#ifndef _LSD_PROTOCOL_H
#define _LSD_PROTOCOL_H

#define LSD_SERVICE_NAME "com.apple.coreservices.launchservicesd"
#define LSD_SERVICE_NAME_OVERRIDE_ENV "__LSXPCSERVICENAME"

#define LSD_KEY_OP "darling-op"
#define LSD_KEY_ERR "err"

// Add or refresh an application bundle in the database.
// Request: "path" (string). Reply: "err".
#define LSD_OP_REGISTER "register"

// A process that can receive Apple events announces itself and the XPC endpoint they are sent to.
// The registration lasts as long as the connection the request came over.
// Request: "pid" (int64), "bundle_id" (string, optional), "endpoint" (endpoint). Reply: "err".
#define LSD_OP_AE_CHECKIN "ae-checkin"

// Find the endpoint of a process that checked in, by "pid" (int64) or by "bundle_id" (string; the
// most recent check-in wins). With "wait_ms" (int64) a pid that has not checked in yet is waited
// for, until the process exits or the time elapses.
// Reply: "err" (procNotFound when there is none), "pid" (int64), "endpoint".
#define LSD_OP_AE_LOOKUP "ae-lookup"

// Queue an Apple event for a process that was just started and has not checked in yet (or deliver
// it at once to one that did). When the process checks in, launchservicesd delivers its first event:
// the queued events in order, preceded by an open-application event unless the first queued event
// opens or prints documents. A process that checks in with nothing queued gets open-application
// (after a short grace period in which a launch may still queue its events).
// Request: "pid" (int64), "event" (data, a flattened Apple event), "event_id" (int64, the event's
// AEEventID, so that the event need not be parsed). Reply: "err".
#define LSD_OP_AE_QUEUE "ae-queue"

#define LSD_KEY_EVENT_ID "event_id"

#define LSD_KEY_PATH "path"
#define LSD_KEY_PID "pid"
#define LSD_KEY_BUNDLE_ID "bundle_id"
#define LSD_KEY_ENDPOINT "endpoint"
#define LSD_KEY_WAIT_MS "wait_ms"

// Apple event delivery, sent to the endpoint a process checked in with.
// Request: "event" (data, a flattened Apple event). Reply: "err" and "reply" (data, the flattened reply event).
#define LSD_KEY_EVENT "event"
#define LSD_KEY_REPLY "reply"

#endif
