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

#ifndef _MEDIAREMOTED_PROTOCOL_H_
#define _MEDIAREMOTED_PROTOCOL_H_

#define MEDIAREMOTED_MACH_SERVICE "com.apple.mediaremoted"
#define MEDIAREMOTED_SERVICE_ENV "__MEDIAREMOTED_SERVICE_NAME"
#define MEDIAREMOTED_DBUS_ADDRESS_ENV "__MEDIAREMOTED_DBUS_ADDRESS"

// Operations from client to daemon
#define MR_OP_CHECKIN "checkin"
#define MR_OP_UPDATE_STATE "update-state"

// Operations from daemon to client
#define MR_OP_INVOKE_COMMAND "invoke-command"

// Keys
#define MR_KEY_OP "op"
#define MR_KEY_ERR "err"
#define MR_KEY_BUNDLE_ID "bundle-id"
#define MR_KEY_APP_NAME "app-name"
#define MR_KEY_PLAYBACK_STATE "playback-state" // uint64
#define MR_KEY_TITLE "title"
#define MR_KEY_ARTIST "artist" // string
#define MR_KEY_ALBUM "album"
#define MR_KEY_ALBUM_TRACK_COUNT "album-track-count" // int64
#define MR_KEY_ALBUM_TRACK_NUMBER "album-track-number" // int64
#define MR_KEY_COMPOSER "composer" // string
#define MR_KEY_DISC_COUNT "disc-count" // int64
#define MR_KEY_DISC_NUMBER "disc-number" // int64
#define MR_KEY_GENRE "genre" // string
#define MR_KEY_DURATION "duration" // double (seconds)
#define MR_KEY_ELAPSED_TIME "elapsed-time" // double (seconds)
#define MR_KEY_RATE "rate" // double
#define MR_KEY_ARTWORK_URL "artwork-url" // string (e.g. file:///tmp/...)
#define MR_KEY_ENABLED_COMMANDS "enabled-commands" // uint64 bitmask

// Command invocation keys
#define MR_KEY_COMMAND "command" // string: "play", "pause", "togglePlayPause", "stop", "nextTrack", "previousTrack", "skipForward", "skipBackward", "changePlaybackPosition"
#define MR_KEY_POSITION_TIME "position-time" // double (seconds)
#define MR_KEY_OFFSET_TIME "offset-time" // double (seconds)
#define MR_KEY_STATUS "status" // int64 (MPRemoteCommandHandlerStatus)

// Command bitmask flags (which commands are enabled AND have targets)
#define MR_CMD_PLAY                      (1ULL << 0)
#define MR_CMD_PAUSE                     (1ULL << 1)
#define MR_CMD_STOP                      (1ULL << 2)
#define MR_CMD_TOGGLE_PLAY_PAUSE         (1ULL << 3)
#define MR_CMD_NEXT_TRACK                (1ULL << 4)
#define MR_CMD_PREVIOUS_TRACK            (1ULL << 5)
#define MR_CMD_CHANGE_PLAYBACK_RATE      (1ULL << 6)
#define MR_CMD_CHANGE_REPEAT_MODE        (1ULL << 7)
#define MR_CMD_CHANGE_SHUFFLE_MODE       (1ULL << 8)
#define MR_CMD_SKIP_FORWARD              (1ULL << 9)
#define MR_CMD_SKIP_BACKWARD             (1ULL << 10)
#define MR_CMD_CHANGE_PLAYBACK_POSITION  (1ULL << 11)
#define MR_CMD_RATING                    (1ULL << 12)
#define MR_CMD_LIKE                      (1ULL << 13)
#define MR_CMD_DISLIKE                   (1ULL << 14)
#define MR_CMD_BOOKMARK                  (1ULL << 15)

#endif
