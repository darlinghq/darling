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
// A complete snapshot of the client's now-playing state: it replaces what the
// daemon knows (a key that is absent means the value is not set), and it
// carries the enabled-commands mask too.
#define MR_OP_UPDATE_STATE "update-state"
// Only the enabled-commands mask changed.
#define MR_OP_UPDATE_COMMANDS "update-commands"

// Operations from daemon to client
#define MR_OP_INVOKE_COMMAND "invoke-command"

// Keys
#define MR_KEY_OP "op"
#define MR_KEY_BUNDLE_ID "bundle-id"
#define MR_KEY_APP_NAME "app-name"
#define MR_KEY_PLAYBACK_STATE "playback-state" // uint64, MPNowPlayingPlaybackState
#define MR_KEY_HAS_INFO "has-info" // bool, the client's nowPlayingInfo is not nil
#define MR_KEY_INFO_TIMESTAMP "info-timestamp" // double, seconds since 1970 when nowPlayingInfo was set
#define MR_KEY_TITLE "title"
#define MR_KEY_ARTIST "artist" // string
#define MR_KEY_ALBUM "album"
#define MR_KEY_ALBUM_TRACK_NUMBER "album-track-number" // int64
#define MR_KEY_COMPOSER "composer" // string
#define MR_KEY_DISC_NUMBER "disc-number" // int64
#define MR_KEY_GENRE "genre" // string
#define MR_KEY_DURATION "duration" // double (seconds)
#define MR_KEY_ELAPSED_TIME "elapsed-time" // double (seconds), valid at info-timestamp
#define MR_KEY_RATE "rate" // double
#define MR_KEY_ARTWORK_URL "artwork-url" // string, a file: URL on the host
#define MR_KEY_ENABLED_COMMANDS "enabled-commands" // uint64 bitmask

// Command invocation keys. Commands: "play", "pause", "togglePlayPause",
// "stop", "nextTrack", "previousTrack", "skipForward", "skipBackward" and
// "changePlaybackPosition".
#define MR_KEY_COMMAND "command" // string
#define MR_KEY_POSITION_TIME "position-time" // double (seconds), for changePlaybackPosition
#define MR_KEY_INTERVAL "interval" // double (seconds), for skipForward and skipBackward

// The MPRIS track id that stands for "no track"
#define MR_NO_TRACK_ID "/org/mpris/MediaPlayer2/TrackList/NoTrack"

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
