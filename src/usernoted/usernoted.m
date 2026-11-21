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
#include <darling_dbus.h>
#include <spawn.h>
#include <sys/wait.h>
#include <unistd.h>
#include "usernoted_protocol.h"
#include "usernoted_dbus.h"


@interface UNRecord : NSObject
@property (nonatomic, copy) NSString* identifier;
@property (nonatomic, copy) NSString* bundleID;
@property (nonatomic, copy) NSString* bundlePath;
@property (nonatomic, copy) NSString* execPath;
@property (nonatomic, copy) NSString* title;
@property (nonatomic, copy) NSString* subtitle;
@property (nonatomic, copy) NSString* body;
@property (nonatomic, copy) NSString* appIcon;
@property (nonatomic, copy) NSString* categoryID;
@property (nonatomic, copy) NSString* imagePath;
@property (nonatomic, copy) NSString* soundFile;
@property (nonatomic, copy) NSArray* actions;
@property (nonatomic, strong) NSDate* date;
@property (nonatomic, assign) uint32_t hostID;
@property (nonatomic, assign) int urgency;
@property (nonatomic, assign) BOOL customDismiss;
@property (nonatomic, assign) BOOL isNSUser;
@property (nonatomic, assign) int triggerType;
@property (nonatomic, assign) double timeInterval;
@property (nonatomic, assign) BOOL repeats;
@property (nonatomic, strong) dispatch_source_t timerSource;
@end

@implementation UNRecord
- (void)dealloc
{
	if (_timerSource)
	{
		dispatch_source_cancel(_timerSource);
		dispatch_release(_timerSource);
		_timerSource = NULL;
	}
	[_identifier release];
	[_bundleID release];
	[_bundlePath release];
	[_execPath release];
	[_title release];
	[_subtitle release];
	[_body release];
	[_appIcon release];
	[_categoryID release];
	[_imagePath release];
	[_soundFile release];
	[_actions release];
	[_date release];
	[super dealloc];
}
@end

@interface UNClientSession : NSObject
@property (nonatomic, assign) pid_t pid;
@property (nonatomic, copy) NSString* bundleID;
@property (nonatomic, copy) NSString* bundlePath;
@property (nonatomic, copy) NSString* execPath;
@property (nonatomic, assign) xpc_connection_t connection;
@end

@implementation UNClientSession
- (void)dealloc
{
	[_bundleID release];
	[_bundlePath release];
	[_execPath release];
	[super dealloc];
}
@end

static struct usernoted_dbus s_dbus_api;
static BOOL s_dbus_ready = NO;
static DBusConnection* s_dbus_conn = NULL;

static dispatch_queue_t s_daemon_queue;
static NSMutableArray<UNRecord*>* s_delivered;
static NSMutableArray<UNRecord*>* s_pending;
static NSMutableArray<UNClientSession*>* s_clients;
static NSMutableDictionary<NSString*, NSMutableArray<xpc_object_t>*>* s_queued_responses;
static NSMutableDictionary<NSString*, NSSet*>* s_categories;
static uint32_t s_next_synthetic_id = 1000;

/* The checked-in session of an app (daemon queue only), or nil when the app has none. */
static UNClientSession* session_for_bundle(NSString* bundle_id)
{
	for (UNClientSession* s in s_clients)
	{
		if ([s.bundleID isEqualToString:bundle_id])
			return s;
	}
	return nil;
}

static void setup_dbus(void)
{
#if defined(__x86_64__)
	const struct darling_dbus* shared = darling_dbus_get(0);
	if (shared == NULL)
		return;

	memset(&s_dbus_api, 0, sizeof(s_dbus_api));
	s_dbus_api.connection_open_private = shared->connection_open_private;
	s_dbus_api.bus_register = shared->bus_register;
	s_dbus_api.bus_add_match = shared->bus_add_match;
	s_dbus_api.connection_close = shared->connection_close;
	s_dbus_api.connection_unref = shared->connection_unref;
	s_dbus_api.connection_read_write = shared->connection_read_write;
	s_dbus_api.connection_pop_message = shared->connection_pop_message;
	s_dbus_api.connection_send = shared->connection_send;
	s_dbus_api.connection_flush = shared->connection_flush;
	s_dbus_api.connection_send_with_reply_and_block = shared->connection_send_with_reply_and_block;
	s_dbus_api.message_new_method_call = shared->message_new_method_call;
	s_dbus_api.message_new_method_return = shared->message_new_method_return;
	s_dbus_api.message_new_error = shared->message_new_error;
	s_dbus_api.message_iter_init_append = shared->message_iter_init_append;
	s_dbus_api.message_iter_append_basic = shared->message_iter_append_basic;
	s_dbus_api.message_iter_open_container = shared->message_iter_open_container;
	s_dbus_api.message_iter_close_container = shared->message_iter_close_container;
	s_dbus_api.message_iter_init = shared->message_iter_init;
	s_dbus_api.message_iter_get_arg_type = shared->message_iter_get_arg_type;
	s_dbus_api.message_iter_get_basic = shared->message_iter_get_basic;
	s_dbus_api.message_iter_recurse = shared->message_iter_recurse;
	s_dbus_api.message_iter_next = shared->message_iter_next;
	s_dbus_api.message_is_signal = shared->message_is_signal;
	s_dbus_api.message_is_method_call = shared->message_is_method_call;
	s_dbus_api.message_get_path = shared->message_get_path;
	s_dbus_api.message_get_interface = shared->message_get_interface;
	s_dbus_api.message_get_member = shared->message_get_member;
	s_dbus_api.message_get_signature = shared->message_get_signature;
	s_dbus_api.message_unref = shared->message_unref;
	s_dbus_api.error_init = shared->error_init;
	s_dbus_api.error_free = shared->error_free;
	s_dbus_api.address_escape_value = shared->address_escape_value;
	s_dbus_api.free = shared->free;

	s_dbus_api.getenv = getenv;
	s_dbus_ready = YES;
	s_dbus_conn = _UNHostDBusConnect(&s_dbus_api);
#endif
}

static void launch_app(const char* bundle_path, const char* exec_path, const char* bundle_id)
{
	pid_t pid;
	if (bundle_path != NULL && bundle_path[0] != '\0')
	{
		char* argv[] = { "/usr/bin/open", (char*) bundle_path, NULL };
		if (posix_spawn(&pid, "/usr/bin/open", NULL, NULL, argv, NULL) == 0)
			return;
	}
	if (exec_path != NULL && exec_path[0] != '\0')
	{
		char* argv[] = { (char*) exec_path, NULL };
		if (posix_spawn(&pid, exec_path, NULL, NULL, argv, NULL) == 0)
			return;
	}
	if (bundle_id != NULL && bundle_id[0] != '\0')
	{
		char* argv[] = { "/usr/bin/open", "-b", (char*) bundle_id, NULL };
		posix_spawn(&pid, "/usr/bin/open", NULL, NULL, argv, NULL);
	}
}

static void on_action_invoked(uint32_t id, const char* action_key, void* context)
{
	dispatch_async(s_daemon_queue, ^{
		UNRecord* found = nil;
		for (UNRecord* r in s_delivered)
		{
			if (r.hostID == id)
			{
				found = r;
				break;
			}
		}
		if (!found)
			return;

		UNClientSession* active_session = nil;
		for (UNClientSession* s in s_clients)
		{
			if ([s.bundleID isEqualToString:found.bundleID])
			{
				active_session = s;
				break;
			}
		}

		xpc_object_t msg = xpc_dictionary_create(NULL, NULL, 0);
		xpc_dictionary_set_string(msg, UN_KEY_OP, UN_OP_ACTION_INVOKED);
		xpc_dictionary_set_string(msg, UN_KEY_NOTIFICATION_ID, [found.identifier UTF8String]);
		xpc_dictionary_set_string(msg, UN_KEY_ACTION_KEY, action_key ? action_key : "default");
		xpc_dictionary_set_bool(msg, UN_KEY_IS_NSUSER, found.isNSUser);
		if (found.title) xpc_dictionary_set_string(msg, UN_KEY_TITLE, [found.title UTF8String]);
		if (found.subtitle) xpc_dictionary_set_string(msg, UN_KEY_SUBTITLE, [found.subtitle UTF8String]);
		if (found.body) xpc_dictionary_set_string(msg, UN_KEY_BODY, [found.body UTF8String]);
		if (found.categoryID) xpc_dictionary_set_string(msg, UN_KEY_CATEGORY_ID, [found.categoryID UTF8String]);

		if (active_session && active_session.connection)
		{
			xpc_connection_send_message(active_session.connection, msg);
		}
		else
		{
			/* App is not running: launch it and queue the response */
			NSMutableArray<xpc_object_t>* queue = [s_queued_responses objectForKey:found.bundleID];
			if (!queue)
			{
				queue = [NSMutableArray array];
				[s_queued_responses setObject:queue forKey:found.bundleID];
			}
			[queue addObject:msg];

			launch_app([found.bundlePath UTF8String], [found.execPath UTF8String], [found.bundleID UTF8String]);
		}
		xpc_release(msg);
	});
}

static void on_notification_closed(uint32_t id, uint32_t reason, void* context)
{
	dispatch_async(s_daemon_queue, ^{
		UNRecord* found = nil;
		for (UNRecord* r in s_delivered)
		{
			if (r.hostID == id)
			{
				found = r;
				break;
			}
		}
		if (!found)
			return;

		/* If user dismissed (reason == 2) and custom dismiss is enabled, deliver dismiss response */
		if (reason == 2 && found.customDismiss)
		{
			UNClientSession* active_session = nil;
			for (UNClientSession* s in s_clients)
			{
				if ([s.bundleID isEqualToString:found.bundleID])
				{
					active_session = s;
					break;
				}
			}

			xpc_object_t msg = xpc_dictionary_create(NULL, NULL, 0);
			xpc_dictionary_set_string(msg, UN_KEY_OP, UN_OP_ACTION_INVOKED);
			xpc_dictionary_set_string(msg, UN_KEY_NOTIFICATION_ID, [found.identifier UTF8String]);
			xpc_dictionary_set_string(msg, UN_KEY_ACTION_KEY, "com.apple.UNNotificationDismissActionIdentifier");
			xpc_dictionary_set_bool(msg, UN_KEY_IS_NSUSER, found.isNSUser);

			if (active_session && active_session.connection)
			{
				xpc_connection_send_message(active_session.connection, msg);
			}
			xpc_release(msg);
		}

		[s_delivered removeObject:found];
	});
}

static void show_record_on_desktop(UNRecord* r, BOOL with_sound)
{

	/* Map title and subtitle to summary: title + subtitle -> summary */
	NSString* summary = nil;
	if (r.title && r.title.length > 0 && r.subtitle && r.subtitle.length > 0)
	{
		summary = [NSString stringWithFormat:@"%@ — %@", r.title, r.subtitle];
	}
	else if (r.title && r.title.length > 0)
	{
		summary = r.title;
	}
	else if (r.subtitle && r.subtitle.length > 0)
	{
		summary = r.subtitle;
	}
	else
	{
		summary = r.bundleID ? r.bundleID : @"";
	}

	size_t action_count = r.actions.count;
	const char** actions_c = NULL;
	if (action_count > 0)
	{
		actions_c = malloc(sizeof(char*) * action_count);
		for (size_t i = 0; i < action_count; i++)
		{
			actions_c[i] = [[r.actions objectAtIndex:i] UTF8String];
		}
	}

	uint32_t host_id = 0;
	if (s_dbus_ready && s_dbus_conn != NULL)
	{
		host_id = _UNHostDBusNotify(&s_dbus_api, s_dbus_conn,
			[r.bundleID UTF8String],
			r.hostID,
			r.appIcon ? [r.appIcon UTF8String] : NULL,
			[summary UTF8String],
			r.body ? [r.body UTF8String] : "",
			actions_c, action_count,
			r.imagePath ? [r.imagePath UTF8String] : NULL,
			(with_sound && r.soundFile) ? [r.soundFile UTF8String] : NULL,
			r.urgency, -1);

		/* If standard Notify failed, try desktop portal */
		if (host_id == 0)
		{
			_UNHostDBusPortalAddNotification(&s_dbus_api, s_dbus_conn,
				[r.identifier UTF8String],
				[summary UTF8String],
				r.body ? [r.body UTF8String] : "",
				r.appIcon ? [r.appIcon UTF8String] : NULL,
				actions_c, action_count);
		}
	}

	if (actions_c)
		free(actions_c);

	if (host_id == 0)
		host_id = s_next_synthetic_id++;

	r.hostID = host_id;
	[s_delivered addObject:r];
}

/* How long the daemon waits for the app's answer to will-present before presenting normally. */
#define WILL_PRESENT_TIMEOUT_SECONDS 3

/*
 * UNUserNotificationCenter: an app that is in the foreground is asked, through its delegate's
 * userNotificationCenter:willPresentNotification:withCompletionHandler:, how the notification
 * is to be presented (the completion handler's options). Otherwise it is presented at once.
 */
static void post_record_to_desktop(UNRecord* r)
{
	r.date = [NSDate date];

	UNClientSession* session = r.isNSUser ? nil : session_for_bundle(r.bundleID);
	if (session == nil || session.connection == NULL)
	{
		show_record_on_desktop(r, YES);
		return;
	}

	xpc_object_t request = xpc_dictionary_create(NULL, NULL, 0);
	xpc_dictionary_set_string(request, UN_KEY_OP, UN_OP_WILL_PRESENT);
	xpc_dictionary_set_string(request, UN_KEY_NOTIFICATION_ID, [r.identifier UTF8String]);
	if (r.title) xpc_dictionary_set_string(request, UN_KEY_TITLE, [r.title UTF8String]);
	if (r.subtitle) xpc_dictionary_set_string(request, UN_KEY_SUBTITLE, [r.subtitle UTF8String]);
	if (r.body) xpc_dictionary_set_string(request, UN_KEY_BODY, [r.body UTF8String]);
	if (r.categoryID) xpc_dictionary_set_string(request, UN_KEY_CATEGORY_ID, [r.categoryID UTF8String]);

	/* the reply and the timeout both run on the daemon queue, so the flag needs no lock */
	__block BOOL answered = NO;

	xpc_connection_send_message_with_reply(session.connection, request, s_daemon_queue, ^(xpc_object_t reply) {
		if (answered)
			return;
		answered = YES;

		BOOL foreground = NO;
		int64_t options = 0;
		if (xpc_get_type(reply) == XPC_TYPE_DICTIONARY)
		{
			foreground = xpc_dictionary_get_bool(reply, UN_KEY_FOREGROUND);
			options = xpc_dictionary_get_int64(reply, UN_KEY_PRESENT_OPTIONS);
		}

		if (!foreground)
		{
			show_record_on_desktop(r, YES);
		}
		else if (options & UN_PRESENT_VISIBLE)
		{
			show_record_on_desktop(r, (options & UN_PRESENT_SOUND) != 0);
		}
		/* else: the app asked for nothing to be shown, so the notification is not delivered */
	});
	xpc_release(request);

	dispatch_after(dispatch_time(DISPATCH_TIME_NOW, WILL_PRESENT_TIMEOUT_SECONDS * NSEC_PER_SEC), s_daemon_queue, ^{
		if (answered)
			return;
		answered = YES;
		show_record_on_desktop(r, YES);
	});
}

static void schedule_record(UNRecord* r, double fire_date_epoch, double time_interval, BOOL repeats)
{
	[s_pending addObject:r];

	dispatch_source_t timer = dispatch_source_create(DISPATCH_SOURCE_TYPE_TIMER, 0, 0, s_daemon_queue);
	r.timerSource = timer;

	int64_t delay_ns;
	if (time_interval > 0.0)
	{
		delay_ns = (int64_t) (time_interval * NSEC_PER_SEC);
	}
	else
	{
		double now = [[NSDate date] timeIntervalSince1970];
		double diff = fire_date_epoch - now;
		delay_ns = diff > 0.0 ? (int64_t) (diff * NSEC_PER_SEC) : 0;
	}

	int64_t interval_ns = repeats && time_interval > 0.0 ? (int64_t) (time_interval * NSEC_PER_SEC) : DISPATCH_TIME_FOREVER;
	dispatch_source_set_timer(timer, dispatch_time(DISPATCH_TIME_NOW, delay_ns), interval_ns, 100 * NSEC_PER_MSEC);

	dispatch_source_set_event_handler(timer, ^{
		[r retain];
		if (!repeats)
		{
			[s_pending removeObject:r];
			dispatch_source_cancel(timer);
			r.timerSource = NULL;
		}
		post_record_to_desktop(r);
		[r release];
	});

	dispatch_resume(timer);
}

static void handle_client_message(xpc_connection_t conn, xpc_object_t msg)
{
	const char* op = xpc_dictionary_get_string(msg, UN_KEY_OP);
	if (!op)
		return;

	xpc_object_t reply = xpc_dictionary_create_reply(msg);

	if (strcmp(op, UN_OP_CHECKIN) == 0)
	{
		pid_t pid = xpc_connection_get_pid(conn);
		const char* bundle_id = xpc_dictionary_get_string(msg, UN_KEY_BUNDLE_ID);
		const char* bundle_path = xpc_dictionary_get_string(msg, UN_KEY_BUNDLE_PATH);
		const char* exec_path = xpc_dictionary_get_string(msg, UN_KEY_EXEC_PATH);

		UNClientSession* session = [[UNClientSession alloc] init];
		session.pid = pid;
		session.connection = conn;
		if (bundle_id) session.bundleID = [NSString stringWithUTF8String:bundle_id];
		if (bundle_path) session.bundlePath = [NSString stringWithUTF8String:bundle_path];
		if (exec_path) session.execPath = [NSString stringWithUTF8String:exec_path];
		[s_clients addObject:session];
		[session release];

		/* Deliver any queued responses for this bundle */
		if (session.bundleID)
		{
			NSMutableArray<xpc_object_t>* queued = [s_queued_responses objectForKey:session.bundleID];
			if (queued && queued.count > 0)
			{
				for (xpc_object_t qm in queued)
				{
					xpc_connection_send_message(conn, qm);
				}
				[s_queued_responses removeObjectForKey:session.bundleID];
			}
		}

		if (reply)
		{
			xpc_dictionary_set_int64(reply, UN_KEY_ERR, 0);
			xpc_connection_send_message(conn, reply);
			xpc_release(reply);
		}
		return;
	}

	if (strcmp(op, UN_OP_POST) == 0)
	{
		const char* ident = xpc_dictionary_get_string(msg, UN_KEY_NOTIFICATION_ID);
		const char* bundle_id = xpc_dictionary_get_string(msg, UN_KEY_BUNDLE_ID);
		const char* bundle_path = xpc_dictionary_get_string(msg, UN_KEY_BUNDLE_PATH);
		const char* exec_path = xpc_dictionary_get_string(msg, UN_KEY_EXEC_PATH);
		const char* title = xpc_dictionary_get_string(msg, UN_KEY_TITLE);
		const char* subtitle = xpc_dictionary_get_string(msg, UN_KEY_SUBTITLE);
		const char* body = xpc_dictionary_get_string(msg, UN_KEY_BODY);
		const char* app_icon = xpc_dictionary_get_string(msg, UN_KEY_APP_ICON);
		const char* category_id = xpc_dictionary_get_string(msg, UN_KEY_CATEGORY_ID);
		const char* image_path = xpc_dictionary_get_string(msg, UN_KEY_IMAGE_PATH);
		const char* sound_file = xpc_dictionary_get_string(msg, UN_KEY_SOUND_FILE);
		int urgency = (int) xpc_dictionary_get_int64(msg, UN_KEY_URGENCY);
		BOOL custom_dismiss = xpc_dictionary_get_bool(msg, UN_KEY_CUSTOM_DISMISS);
		BOOL is_nsuser = xpc_dictionary_get_bool(msg, UN_KEY_IS_NSUSER);
		int trigger_type = (int) xpc_dictionary_get_int64(msg, UN_KEY_TRIGGER_TYPE);
		double time_interval = xpc_dictionary_get_double(msg, UN_KEY_TIME_INTERVAL);
		BOOL repeats = xpc_dictionary_get_bool(msg, UN_KEY_REPEATS);
		double fire_date = xpc_dictionary_get_double(msg, UN_KEY_FIRE_DATE);

		UNRecord* r = [[UNRecord alloc] init];
		r.identifier = ident ? [NSString stringWithUTF8String:ident] : [[NSUUID UUID] UUIDString];
		r.bundleID = bundle_id ? [NSString stringWithUTF8String:bundle_id] : @"";
		r.bundlePath = bundle_path ? [NSString stringWithUTF8String:bundle_path] : nil;
		r.execPath = exec_path ? [NSString stringWithUTF8String:exec_path] : nil;
		r.title = title ? [NSString stringWithUTF8String:title] : nil;
		r.subtitle = subtitle ? [NSString stringWithUTF8String:subtitle] : nil;
		r.body = body ? [NSString stringWithUTF8String:body] : nil;
		r.appIcon = app_icon ? [NSString stringWithUTF8String:app_icon] : nil;
		r.categoryID = category_id ? [NSString stringWithUTF8String:category_id] : nil;
		r.imagePath = image_path ? [NSString stringWithUTF8String:image_path] : nil;
		r.soundFile = sound_file ? [NSString stringWithUTF8String:sound_file] : nil;
		r.urgency = urgency;
		r.customDismiss = custom_dismiss;
		r.isNSUser = is_nsuser;
		r.triggerType = trigger_type;
		r.timeInterval = time_interval;
		r.repeats = repeats;

		xpc_object_t actions_arr = xpc_dictionary_get_value(msg, UN_KEY_ACTIONS);
		if (actions_arr && xpc_get_type(actions_arr) == XPC_TYPE_ARRAY)
		{
			size_t count = xpc_array_get_count(actions_arr);
			NSMutableArray* acts = [NSMutableArray arrayWithCapacity:count];
			for (size_t i = 0; i < count; i++)
			{
				const char* s = xpc_array_get_string(actions_arr, i);
				[acts addObject:s ? [NSString stringWithUTF8String:s] : @""];
			}
			r.actions = acts;
		}

		if (trigger_type == 0)
		{
			post_record_to_desktop(r);
		}
		else
		{
			schedule_record(r, fire_date, time_interval, repeats);
		}
		[r release];

		if (reply)
		{
			xpc_dictionary_set_int64(reply, UN_KEY_ERR, 0);
			xpc_connection_send_message(conn, reply);
			xpc_release(reply);
		}
		return;
	}

	if (strcmp(op, UN_OP_REMOVE_DELIVERED) == 0)
	{
		const char* bundle_id = xpc_dictionary_get_string(msg, UN_KEY_BUNDLE_ID);
		NSString* bid = bundle_id ? [NSString stringWithUTF8String:bundle_id] : @"";
		xpc_object_t id_arr = xpc_dictionary_get_value(msg, UN_KEY_IDENTIFIERS);

		NSMutableSet<NSString*>* to_remove_ids = [NSMutableSet set];
		if (id_arr && xpc_get_type(id_arr) == XPC_TYPE_ARRAY)
		{
			size_t count = xpc_array_get_count(id_arr);
			for (size_t i = 0; i < count; i++)
			{
				const char* s = xpc_array_get_string(id_arr, i);
				if (s) [to_remove_ids addObject:[NSString stringWithUTF8String:s]];
			}
		}

		NSMutableArray<UNRecord*>* to_remove = [NSMutableArray array];
		for (UNRecord* r in s_delivered)
		{
			if ([r.bundleID isEqualToString:bid] && [to_remove_ids containsObject:r.identifier])
			{
				[to_remove addObject:r];
				if (s_dbus_ready && s_dbus_conn && r.hostID > 0)
				{
					_UNHostDBusCloseNotification(&s_dbus_api, s_dbus_conn, r.hostID);
				}
			}
		}
		[s_delivered removeObjectsInArray:to_remove];

		if (reply)
		{
			xpc_dictionary_set_int64(reply, UN_KEY_ERR, 0);
			xpc_connection_send_message(conn, reply);
			xpc_release(reply);
		}
		return;
	}

	if (strcmp(op, UN_OP_REMOVE_ALL_DELIVERED) == 0)
	{
		const char* bundle_id = xpc_dictionary_get_string(msg, UN_KEY_BUNDLE_ID);
		NSString* bid = bundle_id ? [NSString stringWithUTF8String:bundle_id] : @"";

		NSMutableArray<UNRecord*>* to_remove = [NSMutableArray array];
		for (UNRecord* r in s_delivered)
		{
			if ([r.bundleID isEqualToString:bid])
			{
				[to_remove addObject:r];
				if (s_dbus_ready && s_dbus_conn && r.hostID > 0)
				{
					_UNHostDBusCloseNotification(&s_dbus_api, s_dbus_conn, r.hostID);
				}
			}
		}
		[s_delivered removeObjectsInArray:to_remove];

		if (reply)
		{
			xpc_dictionary_set_int64(reply, UN_KEY_ERR, 0);
			xpc_connection_send_message(conn, reply);
			xpc_release(reply);
		}
		return;
	}

	if (strcmp(op, UN_OP_REMOVE_PENDING) == 0)
	{
		const char* bundle_id = xpc_dictionary_get_string(msg, UN_KEY_BUNDLE_ID);
		NSString* bid = bundle_id ? [NSString stringWithUTF8String:bundle_id] : @"";
		xpc_object_t id_arr = xpc_dictionary_get_value(msg, UN_KEY_IDENTIFIERS);

		NSMutableSet<NSString*>* to_remove_ids = [NSMutableSet set];
		if (id_arr && xpc_get_type(id_arr) == XPC_TYPE_ARRAY)
		{
			size_t count = xpc_array_get_count(id_arr);
			for (size_t i = 0; i < count; i++)
			{
				const char* s = xpc_array_get_string(id_arr, i);
				if (s) [to_remove_ids addObject:[NSString stringWithUTF8String:s]];
			}
		}

		NSMutableArray<UNRecord*>* to_remove = [NSMutableArray array];
		for (UNRecord* r in s_pending)
		{
			if ([r.bundleID isEqualToString:bid] && [to_remove_ids containsObject:r.identifier])
			{
				[to_remove addObject:r];
			}
		}
		[s_pending removeObjectsInArray:to_remove];

		if (reply)
		{
			xpc_dictionary_set_int64(reply, UN_KEY_ERR, 0);
			xpc_connection_send_message(conn, reply);
			xpc_release(reply);
		}
		return;
	}

	if (strcmp(op, UN_OP_REMOVE_ALL_PENDING) == 0)
	{
		const char* bundle_id = xpc_dictionary_get_string(msg, UN_KEY_BUNDLE_ID);
		NSString* bid = bundle_id ? [NSString stringWithUTF8String:bundle_id] : @"";

		NSMutableArray<UNRecord*>* to_remove = [NSMutableArray array];
		for (UNRecord* r in s_pending)
		{
			if ([r.bundleID isEqualToString:bid])
			{
				[to_remove addObject:r];
			}
		}
		[s_pending removeObjectsInArray:to_remove];

		if (reply)
		{
			xpc_dictionary_set_int64(reply, UN_KEY_ERR, 0);
			xpc_connection_send_message(conn, reply);
			xpc_release(reply);
		}
		return;
	}

	if (strcmp(op, UN_OP_GET_DELIVERED) == 0)
	{
		const char* bundle_id = xpc_dictionary_get_string(msg, UN_KEY_BUNDLE_ID);
		NSString* bid = bundle_id ? [NSString stringWithUTF8String:bundle_id] : @"";

		if (reply)
		{
			xpc_object_t arr = xpc_array_create(NULL, 0);
			for (UNRecord* r in s_delivered)
			{
				if ([r.bundleID isEqualToString:bid])
				{
					xpc_object_t item = xpc_dictionary_create(NULL, NULL, 0);
					if (r.identifier) xpc_dictionary_set_string(item, UN_KEY_NOTIFICATION_ID, [r.identifier UTF8String]);
					if (r.title) xpc_dictionary_set_string(item, UN_KEY_TITLE, [r.title UTF8String]);
					if (r.subtitle) xpc_dictionary_set_string(item, UN_KEY_SUBTITLE, [r.subtitle UTF8String]);
					if (r.body) xpc_dictionary_set_string(item, UN_KEY_BODY, [r.body UTF8String]);
					if (r.categoryID) xpc_dictionary_set_string(item, UN_KEY_CATEGORY_ID, [r.categoryID UTF8String]);
					if (r.date) xpc_dictionary_set_double(item, UN_KEY_FIRE_DATE, [r.date timeIntervalSince1970]);
					xpc_dictionary_set_bool(item, UN_KEY_IS_NSUSER, r.isNSUser);
					xpc_array_append_value(arr, item);
					xpc_release(item);
				}
			}
			xpc_dictionary_set_value(reply, UN_KEY_DELIVERED_NOTIFICATIONS, arr);
			xpc_dictionary_set_int64(reply, UN_KEY_ERR, 0);
			xpc_release(arr);
			xpc_connection_send_message(conn, reply);
			xpc_release(reply);
		}
		return;
	}

	if (strcmp(op, UN_OP_GET_PENDING) == 0)
	{
		const char* bundle_id = xpc_dictionary_get_string(msg, UN_KEY_BUNDLE_ID);
		NSString* bid = bundle_id ? [NSString stringWithUTF8String:bundle_id] : @"";

		if (reply)
		{
			xpc_object_t arr = xpc_array_create(NULL, 0);
			for (UNRecord* r in s_pending)
			{
				if ([r.bundleID isEqualToString:bid])
				{
					xpc_object_t item = xpc_dictionary_create(NULL, NULL, 0);
					if (r.identifier) xpc_dictionary_set_string(item, UN_KEY_NOTIFICATION_ID, [r.identifier UTF8String]);
					if (r.title) xpc_dictionary_set_string(item, UN_KEY_TITLE, [r.title UTF8String]);
					if (r.subtitle) xpc_dictionary_set_string(item, UN_KEY_SUBTITLE, [r.subtitle UTF8String]);
					if (r.body) xpc_dictionary_set_string(item, UN_KEY_BODY, [r.body UTF8String]);
					if (r.categoryID) xpc_dictionary_set_string(item, UN_KEY_CATEGORY_ID, [r.categoryID UTF8String]);
					xpc_dictionary_set_int64(item, UN_KEY_TRIGGER_TYPE, r.triggerType);
					xpc_dictionary_set_double(item, UN_KEY_TIME_INTERVAL, r.timeInterval);
					xpc_dictionary_set_bool(item, UN_KEY_REPEATS, r.repeats);
					xpc_array_append_value(arr, item);
					xpc_release(item);
				}
			}
			xpc_dictionary_set_value(reply, UN_KEY_PENDING_REQUESTS, arr);
			xpc_dictionary_set_int64(reply, UN_KEY_ERR, 0);
			xpc_release(arr);
			xpc_connection_send_message(conn, reply);
			xpc_release(reply);
		}
		return;
	}

	if (strcmp(op, UN_OP_GET_SETTINGS) == 0 || strcmp(op, UN_OP_REQUEST_AUTH) == 0)
	{
		if (reply)
		{
			xpc_dictionary_set_int64(reply, UN_KEY_ERR, 0);
			xpc_dictionary_set_bool(reply, "granted", true);
			xpc_dictionary_set_int64(reply, "authorization-status", 2); /* Authorized */
			xpc_connection_send_message(conn, reply);
			xpc_release(reply);
		}
		return;
	}

	if (reply)
	{
		xpc_dictionary_set_int64(reply, UN_KEY_ERR, -1);
		xpc_connection_send_message(conn, reply);
		xpc_release(reply);
	}
}

int main(int argc, const char** argv)
{
	@autoreleasepool {
		s_daemon_queue = dispatch_get_main_queue();
		s_delivered = [[NSMutableArray alloc] init];
		s_pending = [[NSMutableArray alloc] init];
		s_clients = [[NSMutableArray alloc] init];
		s_queued_responses = [[NSMutableDictionary alloc] init];
		s_categories = [[NSMutableDictionary alloc] init];

		setup_dbus();

		/* Timer to periodically pump D-Bus signals */
		dispatch_source_t dbus_timer = dispatch_source_create(DISPATCH_SOURCE_TYPE_TIMER, 0, 0, s_daemon_queue);
		dispatch_source_set_timer(dbus_timer, dispatch_time(DISPATCH_TIME_NOW, 50 * NSEC_PER_MSEC),
			50 * NSEC_PER_MSEC, 10 * NSEC_PER_MSEC);
		dispatch_source_set_event_handler(dbus_timer, ^{
			if (s_dbus_ready && s_dbus_conn != NULL)
			{
				_UNHostDBusProcessMessages(&s_dbus_api, s_dbus_conn, on_action_invoked, on_notification_closed, NULL);
			}
		});
		dispatch_resume(dbus_timer);

		const char* service_name = getenv(USERNOTED_SERVICE_ENV);
		if (!service_name)
			service_name = USERNOTED_MACH_SERVICE;

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
							NSMutableArray<UNClientSession*>* to_remove = [NSMutableArray array];
							for (UNClientSession* s in s_clients)
							{
								if (s.connection == client_conn)
									[to_remove addObject:s];
							}
							[s_clients removeObjectsInArray:to_remove];
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
