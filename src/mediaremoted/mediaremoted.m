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
#include <dlfcn.h>
#include <unistd.h>
#include <ctype.h>
#include "mediaremoted_protocol.h"
#include "mediaremoted_dbus.h"

#define DBUS_WRAPPER_PATH "/usr/lib/native/libdbus.dylib"

@interface MRClientSession : NSObject
@property (nonatomic, assign) pid_t pid;
@property (nonatomic, assign) xpc_connection_t connection;
@property (nonatomic, copy) NSString* bundleID;
@property (nonatomic, copy) NSString* appName;
@property (nonatomic, assign) uint64_t playbackState;
@property (nonatomic, assign) double elapsedTime;
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
	[super dealloc];
}
@end

static struct mediaremoted_dbus s_dbus_api;
static BOOL s_dbus_ready = NO;
static DBusConnection* s_dbus_conn = NULL;

static dispatch_queue_t s_daemon_queue;
static NSMutableArray<MRClientSession*>* s_sessions;
static MRClientSession* s_active_session = nil;
static NSString* s_current_bus_name = nil;

static NSString* sanitize_for_bus_name(NSString* raw)
{
	if (!raw || [raw length] == 0)
		return @"player";

	NSMutableString* s = [NSMutableString string];
	for (NSUInteger i = 0; i < [raw length]; i++)
	{
		unichar c = [raw characterAtIndex:i];
		if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '_' || c == '-')
		{
			[s appendFormat:@"%C", c];
		}
		else if (c == '.')
		{
			[s appendString:@"."];
		}
		else
		{
			[s appendString:@"_"];
		}
	}

	if ([s length] == 0)
		return @"player";

	// Bus name elements cannot begin with a digit or dash
	unichar first = [s characterAtIndex:0];
	if ((first >= '0' && first <= '9') || first == '-')
	{
		[s insertString:@"_" atIndex:0];
	}

	return s;
}

static void build_player_state(MRClientSession* s, struct mr_player_state* state)
{
	memset(state, 0, sizeof(*state));
	if (!s)
	{
		state->identity = "Darling Media Player";
		state->playback_status = "Stopped";
		state->rate = 1.0;
		return;
	}

	state->identity = s.appName ? [s.appName UTF8String] : "Darling Media Player";
	state->desktop_entry = s.appName ? [s.appName UTF8String] : "";

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

	double rate = (s.playbackRate > 0.0) ? s.playbackRate : 1.0;
	state->rate = rate;

	NSTimeInterval now = [[NSDate date] timeIntervalSince1970];
	double current_pos_s = s.elapsedTime;
	if (s.playbackState == 1 && s.lastUpdateTimestamp > 0.0)
	{
		current_pos_s += (now - s.lastUpdateTimestamp) * rate;
	}
	if (s.duration > 0.0 && current_pos_s > s.duration)
		current_pos_s = s.duration;
	if (current_pos_s < 0.0)
		current_pos_s = 0.0;

	state->position_us = (int64_t)(current_pos_s * 1000000.0);
	state->duration_us = (int64_t)(s.duration * 1000000.0);

	state->title = s.title ? [s.title UTF8String] : NULL;
	state->artist = s.artist ? [s.artist UTF8String] : NULL;
	state->album = s.album ? [s.album UTF8String] : NULL;
	state->composer = s.composer ? [s.composer UTF8String] : NULL;
	state->genre = s.genre ? [s.genre UTF8String] : NULL;
	state->art_url = s.artUrl ? [s.artUrl UTF8String] : NULL;
	state->track_number = (int32_t)s.trackNumber;
	state->disc_number = (int32_t)s.discNumber;

	uint64_t cmds = s.enabledCommands;
	state->can_play = (cmds & (MR_CMD_PLAY | MR_CMD_TOGGLE_PLAY_PAUSE)) != 0;
	state->can_pause = (cmds & (MR_CMD_PAUSE | MR_CMD_TOGGLE_PLAY_PAUSE)) != 0;
	state->can_go_next = (cmds & MR_CMD_NEXT_TRACK) != 0;
	state->can_go_previous = (cmds & MR_CMD_PREVIOUS_TRACK) != 0;
	state->can_seek = (cmds & (MR_CMD_CHANGE_PLAYBACK_POSITION | MR_CMD_SKIP_FORWARD | MR_CMD_SKIP_BACKWARD)) != 0;
	state->can_control = true;
}

static void update_active_session(void)
{
	MRClientSession* best = nil;
	for (MRClientSession* s in s_sessions)
	{
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

	if (best != s_active_session)
	{
		if (s_current_bus_name != nil && s_dbus_ready && s_dbus_conn != NULL)
		{
			_MRHostDBusReleaseName(&s_dbus_api, s_dbus_conn, [s_current_bus_name UTF8String]);
			[s_current_bus_name release];
			s_current_bus_name = nil;
		}

		s_active_session = best;

		if (s_active_session != nil && s_dbus_ready && s_dbus_conn != NULL)
		{
			NSString* namePart = s_active_session.bundleID ? s_active_session.bundleID : s_active_session.appName;
			NSString* sanitized = sanitize_for_bus_name(namePart);
			s_current_bus_name = [[NSString stringWithFormat:@"org.mpris.MediaPlayer2.darling.%@", sanitized] retain];
			_MRHostDBusClaimName(&s_dbus_api, s_dbus_conn, [s_current_bus_name UTF8String]);
		}
	}

	if (s_dbus_ready && s_dbus_conn != NULL && s_active_session != nil)
	{
		struct mr_player_state state;
		build_player_state(s_active_session, &state);
		_MRHostDBusEmitPropertiesChanged(&s_dbus_api, s_dbus_conn, &state);
	}
}

static int on_remote_command(const char* command, double position_or_offset, void* context)
{
	dispatch_async(s_daemon_queue, ^{
		if (!s_active_session || !s_active_session.connection)
			return;

		xpc_object_t msg = xpc_dictionary_create(NULL, NULL, 0);
		xpc_dictionary_set_string(msg, MR_KEY_OP, MR_OP_INVOKE_COMMAND);
		xpc_dictionary_set_string(msg, MR_KEY_COMMAND, command ? command : "");
		if (strcmp(command, "setPosition") == 0)
		{
			xpc_dictionary_set_double(msg, MR_KEY_POSITION_TIME, position_or_offset);
		}
		else if (strcmp(command, "seek") == 0)
		{
			xpc_dictionary_set_double(msg, MR_KEY_OFFSET_TIME, position_or_offset);
		}

		xpc_connection_send_message(s_active_session.connection, msg);
		xpc_release(msg);
	});
	return 0;
}

static void handle_client_message(xpc_connection_t conn, xpc_object_t msg)
{
	const char* op = xpc_dictionary_get_string(msg, MR_KEY_OP);
	if (!op)
		return;

	pid_t pid = xpc_connection_get_pid(conn);
	MRClientSession* session = nil;
	for (MRClientSession* s in s_sessions)
	{
		if (s.connection == conn)
		{
			session = s;
			break;
		}
	}

	if (!session)
	{
		session = [[[MRClientSession alloc] init] autorelease];
		session.pid = pid;
		session.connection = conn;
		[s_sessions addObject:session];
	}

	if (strcmp(op, MR_OP_CHECKIN) == 0)
	{
		const char* bid = xpc_dictionary_get_string(msg, MR_KEY_BUNDLE_ID);
		const char* app = xpc_dictionary_get_string(msg, MR_KEY_APP_NAME);
		if (bid) session.bundleID = [NSString stringWithUTF8String:bid];
		if (app) session.appName = [NSString stringWithUTF8String:app];
		update_active_session();
		return;
	}

	if (strcmp(op, MR_OP_UPDATE_STATE) == 0)
	{
		const char* bid = xpc_dictionary_get_string(msg, MR_KEY_BUNDLE_ID);
		const char* app = xpc_dictionary_get_string(msg, MR_KEY_APP_NAME);
		if (bid) session.bundleID = [NSString stringWithUTF8String:bid];
		if (app) session.appName = [NSString stringWithUTF8String:app];

		if (xpc_dictionary_get_value(msg, MR_KEY_PLAYBACK_STATE))
		{
			session.playbackState = xpc_dictionary_get_uint64(msg, MR_KEY_PLAYBACK_STATE);
			if (session.playbackState == 1)
			{
				session.lastPlayTimestamp = [[NSDate date] timeIntervalSince1970];
			}
		}

		if (xpc_dictionary_get_value(msg, MR_KEY_ELAPSED_TIME))
		{
			session.elapsedTime = xpc_dictionary_get_double(msg, MR_KEY_ELAPSED_TIME);
			session.lastUpdateTimestamp = [[NSDate date] timeIntervalSince1970];
		}

		if (xpc_dictionary_get_value(msg, MR_KEY_RATE))
		{
			session.playbackRate = xpc_dictionary_get_double(msg, MR_KEY_RATE);
		}

		if (xpc_dictionary_get_value(msg, MR_KEY_DURATION))
		{
			session.duration = xpc_dictionary_get_double(msg, MR_KEY_DURATION);
		}

		const char* title = xpc_dictionary_get_string(msg, MR_KEY_TITLE);
		if (title) session.title = [NSString stringWithUTF8String:title];

		const char* artist = xpc_dictionary_get_string(msg, MR_KEY_ARTIST);
		if (artist) session.artist = [NSString stringWithUTF8String:artist];

		const char* album = xpc_dictionary_get_string(msg, MR_KEY_ALBUM);
		if (album) session.album = [NSString stringWithUTF8String:album];

		const char* composer = xpc_dictionary_get_string(msg, MR_KEY_COMPOSER);
		if (composer) session.composer = [NSString stringWithUTF8String:composer];

		const char* genre = xpc_dictionary_get_string(msg, MR_KEY_GENRE);
		if (genre) session.genre = [NSString stringWithUTF8String:genre];

		const char* artUrl = xpc_dictionary_get_string(msg, MR_KEY_ARTWORK_URL);
		if (artUrl) session.artUrl = [NSString stringWithUTF8String:artUrl];

		if (xpc_dictionary_get_value(msg, MR_KEY_ALBUM_TRACK_NUMBER))
		{
			session.trackNumber = xpc_dictionary_get_int64(msg, MR_KEY_ALBUM_TRACK_NUMBER);
		}

		if (xpc_dictionary_get_value(msg, MR_KEY_DISC_NUMBER))
		{
			session.discNumber = xpc_dictionary_get_int64(msg, MR_KEY_DISC_NUMBER);
		}

		if (xpc_dictionary_get_value(msg, MR_KEY_ENABLED_COMMANDS))
		{
			session.enabledCommands = xpc_dictionary_get_uint64(msg, MR_KEY_ENABLED_COMMANDS);
		}

		update_active_session();
		return;
	}
}

static void setup_dbus(void)
{
#if defined(__x86_64__)
	void* lib = dlopen(DBUS_WRAPPER_PATH, RTLD_NOW | RTLD_LOCAL);
	if (lib == NULL)
		return;

	memset(&s_dbus_api, 0, sizeof(s_dbus_api));
#define BIND(field, name) \
	if ((*(void**) &s_dbus_api.field = dlsym(lib, name)) == NULL) \
		goto fail;
	BIND(connection_open_private, "dbus_connection_open_private");
	BIND(bus_register, "dbus_bus_register");
	BIND(bus_request_name, "dbus_bus_request_name");
	BIND(bus_release_name, "dbus_bus_release_name");
	BIND(connection_close, "dbus_connection_close");
	BIND(connection_unref, "dbus_connection_unref");
	BIND(connection_read_write, "dbus_connection_read_write");
	BIND(connection_pop_message, "dbus_connection_pop_message");
	BIND(connection_send, "dbus_connection_send");
	BIND(connection_flush, "dbus_connection_flush");
	BIND(connection_send_with_reply_and_block, "dbus_connection_send_with_reply_and_block");
	BIND(message_new_method_call, "dbus_message_new_method_call");
	BIND(message_new_method_return, "dbus_message_new_method_return");
	BIND(message_new_error, "dbus_message_new_error");
	BIND(message_new_signal, "dbus_message_new_signal");
	BIND(message_iter_init_append, "dbus_message_iter_init_append");
	BIND(message_iter_append_basic, "dbus_message_iter_append_basic");
	BIND(message_iter_open_container, "dbus_message_iter_open_container");
	BIND(message_iter_close_container, "dbus_message_iter_close_container");
	BIND(message_iter_init, "dbus_message_iter_init");
	BIND(message_iter_get_arg_type, "dbus_message_iter_get_arg_type");
	BIND(message_iter_get_basic, "dbus_message_iter_get_basic");
	BIND(message_iter_recurse, "dbus_message_iter_recurse");
	BIND(message_iter_next, "dbus_message_iter_next");
	BIND(message_get_type, "dbus_message_get_type");
	BIND(message_is_signal, "dbus_message_is_signal");
	BIND(message_is_method_call, "dbus_message_is_method_call");
	BIND(message_get_path, "dbus_message_get_path");
	BIND(message_get_interface, "dbus_message_get_interface");
	BIND(message_get_member, "dbus_message_get_member");
	BIND(message_get_signature, "dbus_message_get_signature");
	BIND(message_unref, "dbus_message_unref");
	BIND(error_init, "dbus_error_init");
	BIND(error_free, "dbus_error_free");
	BIND(address_escape_value, "dbus_address_escape_value");
	BIND(free, "dbus_free");
#undef BIND

	s_dbus_api.getenv = getenv;
	s_dbus_ready = YES;
	s_dbus_conn = _MRHostDBusConnect(&s_dbus_api);
	return;

fail:
	dlclose(lib);
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
			if (s_dbus_ready && s_dbus_conn != NULL)
			{
				struct mr_player_state state;
				build_player_state(s_active_session, &state);
				_MRHostDBusProcessMessages(&s_dbus_api, s_dbus_conn, &state, on_remote_command, NULL);
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
							handle_client_message(client_conn, msg);
							xpc_release(msg);
						});
					}
					else if (msg == XPC_ERROR_CONNECTION_INVALID || msg == XPC_ERROR_CONNECTION_INTERRUPTED)
					{
						dispatch_async(s_daemon_queue, ^{
							MRClientSession* found = nil;
							for (MRClientSession* s in s_sessions)
							{
								if (s.connection == client_conn)
								{
									found = s;
									break;
								}
							}
							if (found)
							{
								[found retain];
								[s_sessions removeObject:found];
								if (s_active_session == found)
								{
									s_active_session = nil;
									if (s_current_bus_name && s_dbus_ready && s_dbus_conn != NULL)
									{
										_MRHostDBusReleaseName(&s_dbus_api, s_dbus_conn, [s_current_bus_name UTF8String]);
										[s_current_bus_name release];
										s_current_bus_name = nil;
									}
									update_active_session();
								}
								[found release];
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
