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

#import <Foundation/Foundation.h>
#include <xpc/xpc.h>
#include <dispatch/dispatch.h>
#include <unistd.h>
#include <math.h>
#include <string.h>
#include "mediaremoted_protocol.h"
#include "mediaremoted_dbus.h"
#include "darling_dbus.h"

// What one client process has told the service. The position of the playback
// is kept as an anchor (the elapsed time at a moment in time) from which it
// is extrapolated while the client is playing.
@interface MRClientSession : NSObject
@property (nonatomic, assign) pid_t pid;
@property (nonatomic, assign) xpc_connection_t connection;
@property (nonatomic, copy) NSString* bundleID;
@property (nonatomic, copy) NSString* appName;
@property (nonatomic, assign) uint64_t playbackState;
@property (nonatomic, assign) BOOL hasInfo;
@property (nonatomic, assign) NSTimeInterval infoTimestamp;
@property (nonatomic, assign) BOOL hasElapsed;
@property (nonatomic, assign) double anchorElapsed;
@property (nonatomic, assign) NSTimeInterval anchorTime;
@property (nonatomic, assign) double playbackRate;
@property (nonatomic, assign) NSTimeInterval lastUpdateTimestamp;
@property (nonatomic, assign) NSTimeInterval lastPlayTimestamp;
@property (nonatomic, assign) uint64_t enabledCommands;

@property (nonatomic, copy) NSString* title;
@property (nonatomic, copy) NSString* artist;
@property (nonatomic, copy) NSString* album;
@property (nonatomic, copy) NSString* composer;
@property (nonatomic, copy) NSString* genre;
@property (nonatomic, copy) NSString* artUrl;
@property (nonatomic, assign) double duration;
@property (nonatomic, assign) int64_t trackNumber;
@property (nonatomic, assign) int64_t discNumber;

// Identifies the track for MPRIS; changes when the track changes.
@property (nonatomic, assign) uint64_t trackSerial;
@property (nonatomic, copy) NSString* trackKey;

- (double)positionAt:(NSTimeInterval)now;
- (BOOL)isEligible;
@end

@implementation MRClientSession
- (instancetype)init
{
	self = [super init];
	if (self)
	{
		_playbackRate = 1.0;
		_lastUpdateTimestamp = [[NSDate date] timeIntervalSince1970];
	}
	return self;
}

- (void)dealloc
{
	[_bundleID release];
	[_appName release];
	[_title release];
	[_artist release];
	[_album release];
	[_composer release];
	[_genre release];
	[_artUrl release];
	[_trackKey release];
	[super dealloc];
}

- (double)positionAt:(NSTimeInterval)now
{
	if (!_hasElapsed)
		return 0.0;

	double pos = _anchorElapsed;
	if (_playbackState == 1) // MPNowPlayingPlaybackStatePlaying
		pos += (now - _anchorTime) * _playbackRate;
	if (_duration > 0.0 && pos > _duration)
		pos = _duration;
	if (pos < 0.0)
		pos = 0.0;
	return pos;
}

// A client that has not published anything (it only registered command
// targets, or stopped playing and cleared its info) is not a media player the
// desktop should list.
- (BOOL)isEligible
{
	return _hasInfo || _playbackState == 1 || _playbackState == 2 || _playbackState == 4;
}
@end

static struct mediaremoted_dbus s_dbus_api;
static BOOL s_dbus_ready = NO;
static DBusConnection* s_dbus_conn = NULL;

static dispatch_queue_t s_daemon_queue;
static NSMutableArray<MRClientSession*>* s_sessions;
static MRClientSession* s_active_session = nil;
static NSString* s_current_bus_name = nil;
static NSString* s_last_signature = nil;

// The last part of the well-known name. A bus name is made of dot-separated
// elements that are non-empty, use [A-Za-z0-9_-] and do not start with a digit;
// the whole name is at most 255 bytes.
static NSString* sanitize_for_bus_name(NSString* raw)
{
	if (!raw || [raw length] == 0)
		return @"player";

	NSMutableArray<NSString*>* elements = [NSMutableArray array];
	for (NSString* part in [raw componentsSeparatedByString:@"."])
	{
		NSMutableString* e = [NSMutableString string];
		for (NSUInteger i = 0; i < [part length]; i++)
		{
			unichar c = [part characterAtIndex:i];
			if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '_' || c == '-')
				[e appendFormat:@"%C", c];
			else
				[e appendString:@"_"];
		}
		if ([e length] == 0 || ([e characterAtIndex:0] >= '0' && [e characterAtIndex:0] <= '9'))
			[e insertString:@"_" atIndex:0];
		[elements addObject:e];
	}

	NSString* name = [elements componentsJoinedByString:@"."];
	if ([name length] > 200)
		name = [name substringToIndex:200];
	return name;
}

static void build_player_state(MRClientSession* s, struct mr_player_state* state)
{
	memset(state, 0, sizeof(*state));
	state->rate = 1.0;
	state->playback_status = "Stopped";
	state->identity = "Darling Media Player";
	state->track_id = MR_NO_TRACK_ID;
	if (!s)
		return;

	if (s.appName)
		state->identity = [s.appName UTF8String];
	state->desktop_entry = "";

	switch (s.playbackState)
	{
		case 1: // MPNowPlayingPlaybackStatePlaying
			state->playback_status = "Playing";
			break;
		case 2: // MPNowPlayingPlaybackStatePaused
		case 4: // MPNowPlayingPlaybackStateInterrupted
			state->playback_status = "Paused";
			break;
		default:
			state->playback_status = "Stopped";
			break;
	}

	// MPRIS requires a rate within MinimumRate and MaximumRate (both 1.0)
	state->rate = 1.0;

	NSTimeInterval now = [[NSDate date] timeIntervalSince1970];
	state->position_us = (int64_t)([s positionAt:now] * 1000000.0);
	state->duration_us = (int64_t)(s.duration * 1000000.0);

	state->title = s.title ? [s.title UTF8String] : NULL;
	state->artist = s.artist ? [s.artist UTF8String] : NULL;
	state->album = s.album ? [s.album UTF8String] : NULL;
	state->composer = s.composer ? [s.composer UTF8String] : NULL;
	state->genre = s.genre ? [s.genre UTF8String] : NULL;
	state->art_url = s.artUrl ? [s.artUrl UTF8String] : NULL;
	state->track_number = (int32_t)s.trackNumber;
	state->disc_number = (int32_t)s.discNumber;
	if (s.hasInfo && (s.title != nil || s.duration > 0.0))
	{
		state->track_id = [[NSString stringWithFormat:@"/org/mpris/MediaPlayer2/Track/%llu",
			(unsigned long long) s.trackSerial] UTF8String];
	}

	uint64_t cmds = s.enabledCommands;
	state->can_play = (cmds & (MR_CMD_PLAY | MR_CMD_TOGGLE_PLAY_PAUSE)) != 0;
	state->can_pause = (cmds & (MR_CMD_PAUSE | MR_CMD_TOGGLE_PLAY_PAUSE)) != 0;
	state->can_go_next = (cmds & MR_CMD_NEXT_TRACK) != 0;
	state->can_go_previous = (cmds & MR_CMD_PREVIOUS_TRACK) != 0;
	state->can_seek = (cmds & (MR_CMD_CHANGE_PLAYBACK_POSITION | MR_CMD_SKIP_FORWARD | MR_CMD_SKIP_BACKWARD)) != 0;
	state->can_control = true;
}

// The properties that MPRIS announces with PropertiesChanged; the position is
// not one of them (a jump of it is announced with Seeked).
static NSString* state_signature(const struct mr_player_state* state)
{
	return [NSString stringWithFormat:@"%s|%s|%s|%s|%s|%s|%s|%s|%d|%d|%lld|%d%d%d%d%d%d",
		state->playback_status, state->track_id,
		state->title ? state->title : "", state->artist ? state->artist : "",
		state->album ? state->album : "", state->composer ? state->composer : "",
		state->genre ? state->genre : "", state->art_url ? state->art_url : "",
		state->track_number, state->disc_number, (long long) state->duration_us,
		state->can_play, state->can_pause, state->can_go_next, state->can_go_previous,
		state->can_seek, state->can_control];
}

static void set_active_session(MRClientSession* session)
{
	[session retain];
	[s_active_session release];
	s_active_session = session;
}

// Makes the best client the one that the desktop sees: the one that is
// playing, else the one that played last, else the one that was updated last.
// seeked is the session whose position jumped, if any.
static void update_active_session(MRClientSession* seeked)
{
	MRClientSession* best = nil;
	for (MRClientSession* s in s_sessions)
	{
		if (![s isEligible])
			continue;

		if (!best)
		{
			best = s;
		}
		else if (s.playbackState == 1 && best.playbackState != 1)
		{
			best = s;
		}
		else if (s.playbackState != 1 && best.playbackState == 1)
		{
			// best is playing, s is not; keep best
		}
		else if (s.lastPlayTimestamp > best.lastPlayTimestamp)
		{
			best = s;
		}
		else if (s.lastPlayTimestamp == best.lastPlayTimestamp && s.lastUpdateTimestamp > best.lastUpdateTimestamp)
		{
			best = s;
		}
	}

	BOOL have_bus = s_dbus_ready && s_dbus_conn != NULL;

	if (best != s_active_session)
	{
		if (s_current_bus_name != nil && have_bus)
		{
			_MRHostDBusReleaseName(&s_dbus_api, s_dbus_conn, [s_current_bus_name UTF8String]);
		}
		[s_current_bus_name release];
		s_current_bus_name = nil;
		[s_last_signature release];
		s_last_signature = nil;

		set_active_session(best);

		if (best != nil && have_bus)
		{
			NSString* namePart = best.bundleID ? best.bundleID : best.appName;
			s_current_bus_name = [[NSString stringWithFormat:@"org.mpris.MediaPlayer2.darling.%@",
				sanitize_for_bus_name(namePart)] retain];
			_MRHostDBusClaimName(&s_dbus_api, s_dbus_conn, [s_current_bus_name UTF8String]);
		}
	}

	if (best == nil || !have_bus)
		return;

	struct mr_player_state state;
	build_player_state(best, &state);
	NSString* signature = state_signature(&state);
	if (![signature isEqualToString:s_last_signature])
	{
		[s_last_signature release];
		s_last_signature = [signature retain];
		_MRHostDBusEmitPropertiesChanged(&s_dbus_api, s_dbus_conn, &state);
	}
	if (seeked == best)
		_MRHostDBusEmitSeeked(&s_dbus_api, s_dbus_conn, state.position_us);
}

static void send_command(const char* command, double position, double interval)
{
	xpc_object_t msg = xpc_dictionary_create(NULL, NULL, 0);
	xpc_dictionary_set_string(msg, MR_KEY_OP, MR_OP_INVOKE_COMMAND);
	xpc_dictionary_set_string(msg, MR_KEY_COMMAND, command);
	if (strcmp(command, "changePlaybackPosition") == 0)
		xpc_dictionary_set_double(msg, MR_KEY_POSITION_TIME, position);
	else if (strcmp(command, "skipForward") == 0 || strcmp(command, "skipBackward") == 0)
		xpc_dictionary_set_double(msg, MR_KEY_INTERVAL, interval);
	xpc_connection_send_message(s_active_session.connection, msg);
	xpc_release(msg);
}

// A call from the desktop. Seek (relative, in seconds) and SetPosition
// (absolute, in seconds) become the commands that MediaPlayer knows.
static int on_remote_command(const char* command, double seconds, void* context)
{
	MRClientSession* s = s_active_session;
	if (!s || !s.connection || !command)
		return -1;

	if (strcmp(command, "setPosition") == 0 || strcmp(command, "seek") == 0)
	{
		double target = seconds;
		if (strcmp(command, "seek") == 0)
		{
			target = [s positionAt:[[NSDate date] timeIntervalSince1970]] + seconds;
			if (!(s.enabledCommands & MR_CMD_CHANGE_PLAYBACK_POSITION))
			{
				// Without a position command, a skip of the requested length is the closest thing.
				if (seconds > 0.0 && (s.enabledCommands & MR_CMD_SKIP_FORWARD))
					send_command("skipForward", 0.0, seconds);
				else if (seconds < 0.0 && (s.enabledCommands & MR_CMD_SKIP_BACKWARD))
					send_command("skipBackward", 0.0, -seconds);
				return 0;
			}
			// Seeking past the end acts as Next, before the start goes to the start
			if (s.duration > 0.0 && target > s.duration)
			{
				send_command("nextTrack", 0.0, 0.0);
				return 0;
			}
			if (target < 0.0)
				target = 0.0;
		}
		else if (target < 0.0 || (s.duration > 0.0 && target > s.duration))
		{
			return 0; // SetPosition outside of the track does nothing
		}
		if (s.enabledCommands & MR_CMD_CHANGE_PLAYBACK_POSITION)
			send_command("changePlaybackPosition", target, 0.0);
		return 0;
	}

	send_command(command, 0.0, 0.0);
	return 0;
}

static const char* optional_string(xpc_object_t msg, const char* key)
{
	return xpc_dictionary_get_string(msg, key);
}

// Takes over the snapshot of a client. Returns whether the position jumped
// within the same track (a seek).
static BOOL apply_state(MRClientSession* s, xpc_object_t msg)
{
	NSTimeInterval now = [[NSDate date] timeIntervalSince1970];
	uint64_t oldState = s.playbackState;
	double oldPos = [s positionAt:now];
	BOOL hadInfo = s.hasInfo;
	NSString* oldKey = [[s.trackKey retain] autorelease];

	uint64_t newState = xpc_dictionary_get_value(msg, MR_KEY_PLAYBACK_STATE) ?
		xpc_dictionary_get_uint64(msg, MR_KEY_PLAYBACK_STATE) : 0;
	s.playbackState = newState;
	s.hasInfo = xpc_dictionary_get_bool(msg, MR_KEY_HAS_INFO);
	s.lastUpdateTimestamp = now;
	if (newState == 1 && oldState != 1)
		s.lastPlayTimestamp = now;

#define STRING_FIELD(prop, key) do { \
		const char* v_ = optional_string(msg, key); \
		s.prop = v_ ? [NSString stringWithUTF8String:v_] : nil; \
	} while (0)
	STRING_FIELD(title, MR_KEY_TITLE);
	STRING_FIELD(artist, MR_KEY_ARTIST);
	STRING_FIELD(album, MR_KEY_ALBUM);
	STRING_FIELD(composer, MR_KEY_COMPOSER);
	STRING_FIELD(genre, MR_KEY_GENRE);
	STRING_FIELD(artUrl, MR_KEY_ARTWORK_URL);
#undef STRING_FIELD

	s.duration = xpc_dictionary_get_value(msg, MR_KEY_DURATION) ?
		xpc_dictionary_get_double(msg, MR_KEY_DURATION) : 0.0;
	s.trackNumber = xpc_dictionary_get_value(msg, MR_KEY_ALBUM_TRACK_NUMBER) ?
		xpc_dictionary_get_int64(msg, MR_KEY_ALBUM_TRACK_NUMBER) : 0;
	s.discNumber = xpc_dictionary_get_value(msg, MR_KEY_DISC_NUMBER) ?
		xpc_dictionary_get_int64(msg, MR_KEY_DISC_NUMBER) : 0;
	s.playbackRate = xpc_dictionary_get_value(msg, MR_KEY_RATE) ?
		xpc_dictionary_get_double(msg, MR_KEY_RATE) : 1.0;
	s.enabledCommands = xpc_dictionary_get_uint64(msg, MR_KEY_ENABLED_COMMANDS);

	NSString* newKey = [NSString stringWithFormat:@"%@|%@|%@|%g", s.title, s.artist, s.album, s.duration];
	if (![newKey isEqualToString:oldKey])
	{
		s.trackKey = newKey;
		s.trackSerial = s.trackSerial + 1;
	}

	BOOL seeked = NO;
	NSTimeInterval infoTimestamp = xpc_dictionary_get_double(msg, MR_KEY_INFO_TIMESTAMP);
	if (infoTimestamp != s.infoTimestamp)
	{
		// New now-playing info: its elapsed time is relative to when it was set
		s.infoTimestamp = infoTimestamp;
		s.hasElapsed = xpc_dictionary_get_value(msg, MR_KEY_ELAPSED_TIME) != NULL;
		s.anchorElapsed = xpc_dictionary_get_double(msg, MR_KEY_ELAPSED_TIME);
		s.anchorTime = infoTimestamp > 0.0 ? infoTimestamp : now;
		if (hadInfo && s.hasInfo && s.hasElapsed && [newKey isEqualToString:oldKey] &&
			fabs([s positionAt:now] - oldPos) > 1.0)
		{
			seeked = YES;
		}
	}
	else if (oldState == 1 && newState != 1)
	{
		// Playback stopped advancing: freeze the position where it is
		s.anchorElapsed = oldPos;
		s.anchorTime = now;
	}
	else if (oldState != 1 && newState == 1)
	{
		s.anchorTime = now;
	}
	return seeked;
}

static MRClientSession* session_for_connection(xpc_connection_t conn)
{
	for (MRClientSession* s in s_sessions)
	{
		if (s.connection == conn)
			return s;
	}
	return nil;
}

static void handle_client_message(xpc_connection_t conn, xpc_object_t msg)
{
	const char* op = xpc_dictionary_get_string(msg, MR_KEY_OP);
	if (!op)
		return;

	MRClientSession* session = session_for_connection(conn);
	if (!session)
	{
		session = [[[MRClientSession alloc] init] autorelease];
		session.pid = xpc_connection_get_pid(conn);
		session.connection = conn;
		[s_sessions addObject:session];
	}

	const char* bid = xpc_dictionary_get_string(msg, MR_KEY_BUNDLE_ID);
	const char* app = xpc_dictionary_get_string(msg, MR_KEY_APP_NAME);
	if (bid) session.bundleID = [NSString stringWithUTF8String:bid];
	if (app) session.appName = [NSString stringWithUTF8String:app];

	BOOL seeked = NO;
	if (strcmp(op, MR_OP_UPDATE_STATE) == 0)
	{
		seeked = apply_state(session, msg);
	}
	else if (strcmp(op, MR_OP_UPDATE_COMMANDS) == 0)
	{
		session.enabledCommands = xpc_dictionary_get_uint64(msg, MR_KEY_ENABLED_COMMANDS);
	}
	else if (strcmp(op, MR_OP_CHECKIN) != 0)
	{
		return;
	}

	update_active_session(seeked ? session : nil);
}

static void setup_dbus(void)
{
#if defined(__x86_64__)
	const struct darling_dbus* shared = darling_dbus_get(0);
	if (shared == NULL)
		return;

	memset(&s_dbus_api, 0, sizeof(s_dbus_api));
#define BIND(name) s_dbus_api.name = shared->name
	BIND(connection_open_private);
	BIND(bus_register);
	BIND(bus_request_name);
	BIND(bus_release_name);
	BIND(connection_close);
	BIND(connection_unref);
	BIND(connection_read_write);
	BIND(connection_pop_message);
	BIND(connection_send);
	BIND(connection_flush);
	BIND(connection_send_with_reply_and_block);
	BIND(message_new_method_call);
	BIND(message_new_method_return);
	BIND(message_new_error);
	BIND(message_new_signal);
	BIND(message_iter_init_append);
	BIND(message_iter_append_basic);
	BIND(message_iter_open_container);
	BIND(message_iter_close_container);
	BIND(message_iter_init);
	BIND(message_iter_get_arg_type);
	BIND(message_iter_get_basic);
	BIND(message_iter_recurse);
	BIND(message_iter_next);
	BIND(message_get_type);
	BIND(message_is_signal);
	BIND(message_is_method_call);
	BIND(message_get_path);
	BIND(message_get_interface);
	BIND(message_get_member);
	BIND(message_get_signature);
	BIND(message_unref);
	BIND(error_init);
	BIND(error_free);
	BIND(address_escape_value);
	BIND(free);
#undef BIND

	s_dbus_api.getenv = getenv;
	s_dbus_ready = YES;
	s_dbus_conn = _MRHostDBusConnect(&s_dbus_api);
#endif
}

int main(int argc, const char** argv)
{
	@autoreleasepool {
		s_daemon_queue = dispatch_get_main_queue();
		s_sessions = [[NSMutableArray alloc] init];

		setup_dbus();

		// Timer to process incoming D-Bus calls and signals
		dispatch_source_t dbus_timer = dispatch_source_create(DISPATCH_SOURCE_TYPE_TIMER, 0, 0, s_daemon_queue);
		dispatch_source_set_timer(dbus_timer, dispatch_time(DISPATCH_TIME_NOW, 50 * NSEC_PER_MSEC),
			50 * NSEC_PER_MSEC, 10 * NSEC_PER_MSEC);
		dispatch_source_set_event_handler(dbus_timer, ^{
			@autoreleasepool {
				if (s_dbus_ready && s_dbus_conn != NULL)
				{
					struct mr_player_state state;
					build_player_state(s_active_session, &state);
					_MRHostDBusProcessMessages(&s_dbus_api, s_dbus_conn, &state, on_remote_command, NULL);
				}
			}
		});
		dispatch_resume(dbus_timer);

		const char* service_name = getenv(MEDIAREMOTED_SERVICE_ENV);
		if (!service_name)
			service_name = MEDIAREMOTED_MACH_SERVICE;

		xpc_connection_t listener = xpc_connection_create_mach_service(service_name,
			s_daemon_queue, XPC_CONNECTION_MACH_SERVICE_LISTENER);

		xpc_connection_set_event_handler(listener, ^(xpc_object_t peer) {
			if (xpc_get_type(peer) == XPC_TYPE_CONNECTION)
			{
				xpc_connection_t client_conn = (xpc_connection_t) peer;
				xpc_connection_set_event_handler(client_conn, ^(xpc_object_t msg) {
					if (xpc_get_type(msg) == XPC_TYPE_DICTIONARY)
					{
						xpc_retain(msg);
						dispatch_async(s_daemon_queue, ^{
							@autoreleasepool {
								handle_client_message(client_conn, msg);
							}
							xpc_release(msg);
						});
					}
					else if (msg == XPC_ERROR_CONNECTION_INVALID || msg == XPC_ERROR_CONNECTION_INTERRUPTED)
					{
						dispatch_async(s_daemon_queue, ^{
							@autoreleasepool {
								MRClientSession* found = session_for_connection(client_conn);
								if (found)
								{
									[s_sessions removeObject:found];
									update_active_session(nil);
								}
							}
						});
					}
				});
				xpc_connection_resume(client_conn);
			}
		});

		xpc_connection_resume(listener);
	}

	dispatch_main();
	return 0;
}
