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
#include "locationd_protocol.h"
#include "locationd_dbus.h"

#define WGS84_A 6378137.0
#define WGS84_F (1.0 / 298.257223563)
#define WGS84_B ((1.0 - WGS84_F) * WGS84_A)
#define DEG_TO_RAD (M_PI / 180.0)

static double calculateDistance(double lat1, double lon1, double lat2, double lon2)
{
    if (lat1 == lat2 && lon1 == lon2)
        return 0.0;

    double dlat = (lat2 - lat1) * DEG_TO_RAD;
    double dlon = (lon2 - lon1) * DEG_TO_RAD;
    double phi1 = lat1 * DEG_TO_RAD;
    double phi2 = lat2 * DEG_TO_RAD;

    double a = sin(dlat / 2.0) * sin(dlat / 2.0) +
               cos(phi1) * cos(phi2) * sin(dlon / 2.0) * sin(dlon / 2.0);
    double c = 2.0 * atan2(sqrt(a), sqrt(1.0 - a));
    return 6371009.0 * c;
}

@interface LocationClientSession : NSObject
@property (nonatomic, assign) pid_t pid;
@property (nonatomic, assign) xpc_connection_t connection;
@property (nonatomic, copy) NSString* bundleID;
@property (nonatomic, assign) BOOL isUpdating;
@property (nonatomic, assign) BOOL hasRequestedLocation;
@property (nonatomic, assign) double desiredAccuracy;
@property (nonatomic, assign) double distanceFilter;
@property (nonatomic, assign) struct locationd_fix lastFix;
@property (nonatomic, assign) BOOL hasLastFix;
@end

@implementation LocationClientSession
- (instancetype)init
{
    self = [super init];
    if (self)
    {
        _desiredAccuracy = -1.0;
        _distanceFilter = -1.0;
    }
    return self;
}

- (void)dealloc
{
    [_bundleID release];
    [super dealloc];
}
@end

static dispatch_queue_t s_main_queue;
static NSMutableArray<LocationClientSession*>* s_sessions;

static struct locationd_fix s_cached_fix;
static BOOL s_has_cached_fix = NO;
static NSTimeInterval s_cached_fix_timestamp = 0.0;

static dispatch_source_t s_dbus_source = NULL;
static dispatch_source_t s_poll_timer = NULL;
static BOOL s_host_available = NO;
static BOOL s_dbus_active = NO;
static BOOL s_host_denied = NO;
static NSTimeInterval s_last_host_init = 0.0;

static void update_host_session(void);
static void broadcast_auth_status(void);

// The host bus or location service may not have been there when locationd started.
static void retry_host_init(void)
{
    if (s_host_available)
        return;

    NSTimeInterval now = [[NSDate date] timeIntervalSince1970];
    if (now - s_last_host_init < 5.0)
        return;
    s_last_host_init = now;

    s_host_available = (locationd_dbus_init(NULL, NULL) != LOCATIOND_BACKEND_NONE);
    if (s_host_available)
    {
        s_host_denied = NO;
        broadcast_auth_status();
    }
}

// The host service decides: locationd is a single client of it, so the
// authorization it reports is the same for every application. Nothing is
// enforced here when the host allows access.
static int current_auth_status(void)
{
    if (!s_host_available)
        return 1; // kCLAuthorizationStatusRestricted
    if (s_host_denied)
        return 2; // kCLAuthorizationStatusDenied
    return 3; // kCLAuthorizationStatusAuthorizedAlways
}

static void broadcast_auth_status(void)
{
    int status = current_auth_status();
    for (LocationClientSession* s in s_sessions)
    {
        xpc_object_t notif = xpc_dictionary_create(NULL, NULL, 0);
        xpc_dictionary_set_string(notif, CL_KEY_OP, CL_OP_AUTH_UPDATE);
        xpc_dictionary_set_int64(notif, CL_KEY_AUTH_STATUS, status);
        xpc_dictionary_set_int64(notif, CL_KEY_ACCURACY_AUTH, 0); // FullAccuracy
        xpc_dictionary_set_bool(notif, CL_KEY_ENABLED, s_host_available);
        xpc_connection_send_message(s.connection, notif);
        xpc_release(notif);
    }
}

static void broadcast_error(int error_code)
{
    for (LocationClientSession* s in s_sessions)
    {
        if (s.isUpdating || s.hasRequestedLocation)
        {
            xpc_object_t notif = xpc_dictionary_create(NULL, NULL, 0);
            xpc_dictionary_set_string(notif, CL_KEY_OP, CL_OP_ERROR);
            xpc_dictionary_set_int64(notif, CL_KEY_ERROR_CODE, error_code);
            xpc_connection_send_message(s.connection, notif);
            xpc_release(notif);
        }
    }
}

static void on_host_fix(const struct locationd_fix* fix, void* context)
{
    s_cached_fix = *fix;
    s_has_cached_fix = YES;
    s_cached_fix_timestamp = [[NSDate date] timeIntervalSince1970];

    for (LocationClientSession* s in s_sessions)
    {
        if (!s.isUpdating && !s.hasRequestedLocation)
            continue;

        // Check distance filter
        if (s.distanceFilter > 0.0 && s.hasLastFix)
        {
            double dist = calculateDistance(s.lastFix.latitude, s.lastFix.longitude,
                                            fix->latitude, fix->longitude);
            if (dist < s.distanceFilter && fix->horizontal_accuracy >= s.lastFix.horizontal_accuracy)
            {
                // Movement below threshold and accuracy not improved: skip
                if (!s.hasRequestedLocation)
                    continue;
            }
        }

        s.lastFix = *fix;
        s.hasLastFix = YES;

        xpc_object_t update = xpc_dictionary_create(NULL, NULL, 0);
        xpc_dictionary_set_string(update, CL_KEY_OP, CL_OP_LOCATION_UPDATE);
        xpc_dictionary_set_double(update, CL_KEY_LATITUDE, fix->latitude);
        xpc_dictionary_set_double(update, CL_KEY_LONGITUDE, fix->longitude);
        xpc_dictionary_set_double(update, CL_KEY_ALTITUDE, fix->altitude);
        xpc_dictionary_set_double(update, CL_KEY_HORIZONTAL_ACC, fix->horizontal_accuracy);
        xpc_dictionary_set_double(update, CL_KEY_VERTICAL_ACC, fix->vertical_accuracy);
        xpc_dictionary_set_double(update, CL_KEY_COURSE, fix->course);
        xpc_dictionary_set_double(update, CL_KEY_SPEED, fix->speed);
        xpc_dictionary_set_double(update, CL_KEY_TIMESTAMP, fix->timestamp);
        xpc_connection_send_message(s.connection, update);
        xpc_release(update);

        if (s.hasRequestedLocation)
            s.hasRequestedLocation = NO;
    }

    update_host_session();
}

static void on_host_auth(enum locationd_auth_state status, void* context)
{
    BOOL denied = (status == LOCATIOND_AUTH_DENIED);
    if (denied == s_host_denied)
        return;

    s_host_denied = denied;
    broadcast_auth_status();
}

static void on_host_error(int error_code, void* context)
{
    broadcast_error(error_code);
}

static void setup_dbus_monitoring(void)
{
    int fd = locationd_dbus_get_fd();
    if (fd >= 0 && !s_dbus_source)
    {
        s_dbus_source = dispatch_source_create(DISPATCH_SOURCE_TYPE_READ, fd, 0, s_main_queue);
        dispatch_source_set_event_handler(s_dbus_source, ^{
            int rc = locationd_dbus_process_messages(0);
            if (rc < 0)
            {
                // D-Bus disconnected
                if (s_dbus_source) {
                    dispatch_source_cancel(s_dbus_source);
                    dispatch_release(s_dbus_source);
                    s_dbus_source = NULL;
                }
                s_host_available = NO;
                s_dbus_active = NO;
                broadcast_auth_status();
            }
        });
        dispatch_resume(s_dbus_source);
    }

    if (!s_poll_timer)
    {
        s_poll_timer = dispatch_source_create(DISPATCH_SOURCE_TYPE_TIMER, 0, 0, s_main_queue);
        dispatch_source_set_timer(s_poll_timer, dispatch_time(DISPATCH_TIME_NOW, 100 * NSEC_PER_MSEC),
                                  100 * NSEC_PER_MSEC, 20 * NSEC_PER_MSEC);
        dispatch_source_set_event_handler(s_poll_timer, ^{
            if (s_dbus_active)
            {
                int rc = locationd_dbus_process_messages(0);
                if (rc < 0)
                {
                    s_host_available = NO;
                    s_dbus_active = NO;
                    broadcast_auth_status();
                }
            }
        });
        dispatch_resume(s_poll_timer);
    }
}

static void update_host_session(void)
{
    BOOL any_updating = NO;
    BOOL have_accuracy = NO, have_filter = NO, no_filter = NO;
    double best_accuracy = -1.0;
    double min_distance = -1.0;

    for (LocationClientSession* s in s_sessions)
    {
        if (s.isUpdating || s.hasRequestedLocation)
        {
            any_updating = YES;

            // Smaller desired accuracy values are better (Best is -1, BestForNavigation -2).
            if (!have_accuracy || s.desiredAccuracy < best_accuracy)
                best_accuracy = s.desiredAccuracy;
            have_accuracy = YES;

            // A client without a distance filter needs every update.
            if (s.distanceFilter <= 0.0)
                no_filter = YES;
            else if (!have_filter || s.distanceFilter < min_distance)
                min_distance = s.distanceFilter;
            have_filter = YES;
        }
    }
    if (no_filter)
        min_distance = -1.0;

    if (any_updating)
    {
        if (!s_dbus_active)
        {
            retry_host_init();
            int rc = s_host_available ? locationd_dbus_start(best_accuracy, min_distance,
                                         on_host_fix, on_host_auth, on_host_error, NULL) : -1;
            if (rc == 0)
            {
                s_dbus_active = YES;
                setup_dbus_monitoring();
            }
            else if (!s_host_denied)
            {
                // The host could not start a session (and did not say it was denied):
                // tell the clients no location is available.
                broadcast_error(0); // kCLErrorLocationUnknown
            }
        }
        else
        {
            locationd_dbus_update_parameters(best_accuracy, min_distance);
        }
    }
    else
    {
        if (s_dbus_active)
        {
            locationd_dbus_stop();
            s_dbus_active = NO;
        }
    }
}

static void handle_client_message(LocationClientSession* session, xpc_object_t msg)
{
    const char* op = xpc_dictionary_get_string(msg, CL_KEY_OP);
    if (!op)
        return;

    if (strcmp(op, CL_OP_REGISTER) == 0)
    {
        const char* bid = xpc_dictionary_get_string(msg, CL_KEY_BUNDLE_ID);
        if (bid)
            session.bundleID = [NSString stringWithUTF8String:bid];
        session.pid = (pid_t)xpc_dictionary_get_int64(msg, CL_KEY_PID);

        retry_host_init();
        int auth = current_auth_status();

        xpc_object_t reply = xpc_dictionary_create_reply(msg);
        if (reply)
        {
            xpc_dictionary_set_bool(reply, CL_KEY_ENABLED, s_host_available);
            xpc_dictionary_set_int64(reply, CL_KEY_AUTH_STATUS, auth);
            xpc_dictionary_set_int64(reply, CL_KEY_ACCURACY_AUTH, 0); // FullAccuracy

            if (s_has_cached_fix)
            {
                xpc_dictionary_set_double(reply, CL_KEY_LATITUDE, s_cached_fix.latitude);
                xpc_dictionary_set_double(reply, CL_KEY_LONGITUDE, s_cached_fix.longitude);
                xpc_dictionary_set_double(reply, CL_KEY_ALTITUDE, s_cached_fix.altitude);
                xpc_dictionary_set_double(reply, CL_KEY_HORIZONTAL_ACC, s_cached_fix.horizontal_accuracy);
                xpc_dictionary_set_double(reply, CL_KEY_VERTICAL_ACC, s_cached_fix.vertical_accuracy);
                xpc_dictionary_set_double(reply, CL_KEY_COURSE, s_cached_fix.course);
                xpc_dictionary_set_double(reply, CL_KEY_SPEED, s_cached_fix.speed);
                xpc_dictionary_set_double(reply, CL_KEY_TIMESTAMP, s_cached_fix.timestamp);
            }

            xpc_connection_send_message(session.connection, reply);
            xpc_release(reply);
        }
    }
    else if (strcmp(op, CL_OP_GET_STATE) == 0)
    {
        retry_host_init();
        int auth = current_auth_status();

        xpc_object_t reply = xpc_dictionary_create_reply(msg);
        if (reply)
        {
            xpc_dictionary_set_bool(reply, CL_KEY_ENABLED, s_host_available);
            xpc_dictionary_set_int64(reply, CL_KEY_AUTH_STATUS, auth);
            xpc_dictionary_set_int64(reply, CL_KEY_ACCURACY_AUTH, 0);

            if (s_has_cached_fix)
            {
                xpc_dictionary_set_double(reply, CL_KEY_LATITUDE, s_cached_fix.latitude);
                xpc_dictionary_set_double(reply, CL_KEY_LONGITUDE, s_cached_fix.longitude);
                xpc_dictionary_set_double(reply, CL_KEY_ALTITUDE, s_cached_fix.altitude);
                xpc_dictionary_set_double(reply, CL_KEY_HORIZONTAL_ACC, s_cached_fix.horizontal_accuracy);
                xpc_dictionary_set_double(reply, CL_KEY_VERTICAL_ACC, s_cached_fix.vertical_accuracy);
                xpc_dictionary_set_double(reply, CL_KEY_COURSE, s_cached_fix.course);
                xpc_dictionary_set_double(reply, CL_KEY_SPEED, s_cached_fix.speed);
                xpc_dictionary_set_double(reply, CL_KEY_TIMESTAMP, s_cached_fix.timestamp);
            }

            xpc_connection_send_message(session.connection, reply);
            xpc_release(reply);
        }
    }
    else if (strcmp(op, CL_OP_REQUEST_AUTH) == 0)
    {
        retry_host_init();
        int auth = current_auth_status();

        xpc_object_t reply = xpc_dictionary_create_reply(msg);
        if (reply)
        {
            xpc_dictionary_set_int64(reply, CL_KEY_AUTH_STATUS, auth);
            xpc_dictionary_set_int64(reply, CL_KEY_ACCURACY_AUTH, 0);
            xpc_connection_send_message(session.connection, reply);
            xpc_release(reply);
        }
    }
    else if (strcmp(op, CL_OP_START_UPDATING) == 0)
    {
        session.isUpdating = YES;
        session.desiredAccuracy = xpc_dictionary_get_double(msg, CL_KEY_DESIRED_ACCURACY);
        session.distanceFilter = xpc_dictionary_get_double(msg, CL_KEY_DISTANCE_FILTER);

        // If we already have a recent cached fix, send it immediately to this client
        NSTimeInterval now = [[NSDate date] timeIntervalSince1970];
        if (s_has_cached_fix && (now - s_cached_fix_timestamp) < 15.0)
        {
            session.lastFix = s_cached_fix;
            session.hasLastFix = YES;

            xpc_object_t update = xpc_dictionary_create(NULL, NULL, 0);
            xpc_dictionary_set_string(update, CL_KEY_OP, CL_OP_LOCATION_UPDATE);
            xpc_dictionary_set_double(update, CL_KEY_LATITUDE, s_cached_fix.latitude);
            xpc_dictionary_set_double(update, CL_KEY_LONGITUDE, s_cached_fix.longitude);
            xpc_dictionary_set_double(update, CL_KEY_ALTITUDE, s_cached_fix.altitude);
            xpc_dictionary_set_double(update, CL_KEY_HORIZONTAL_ACC, s_cached_fix.horizontal_accuracy);
            xpc_dictionary_set_double(update, CL_KEY_VERTICAL_ACC, s_cached_fix.vertical_accuracy);
            xpc_dictionary_set_double(update, CL_KEY_COURSE, s_cached_fix.course);
            xpc_dictionary_set_double(update, CL_KEY_SPEED, s_cached_fix.speed);
            xpc_dictionary_set_double(update, CL_KEY_TIMESTAMP, s_cached_fix.timestamp);
            xpc_connection_send_message(session.connection, update);
            xpc_release(update);
        }

        update_host_session();
    }
    else if (strcmp(op, CL_OP_STOP_UPDATING) == 0)
    {
        session.isUpdating = NO;
        update_host_session();
    }
    else if (strcmp(op, CL_OP_REQUEST_LOCATION) == 0)
    {
        session.hasRequestedLocation = YES;
        session.desiredAccuracy = xpc_dictionary_get_double(msg, CL_KEY_DESIRED_ACCURACY);

        update_host_session();
    }
}

static void setup_xpc_service(const char* name)
{
    xpc_connection_t listener = xpc_connection_create_mach_service(name,
        s_main_queue, XPC_CONNECTION_MACH_SERVICE_LISTENER);
    if (!listener)
        return;

    xpc_connection_set_event_handler(listener, ^(xpc_object_t peer) {
        if (xpc_get_type(peer) == XPC_TYPE_CONNECTION)
        {
            xpc_connection_t client_conn = (xpc_connection_t)peer;
            LocationClientSession* session = [[[LocationClientSession alloc] init] autorelease];
            session.connection = client_conn;
            [s_sessions addObject:session];

            xpc_connection_set_event_handler(client_conn, ^(xpc_object_t event) {
                if (xpc_get_type(event) == XPC_TYPE_DICTIONARY)
                {
                    xpc_retain(event);
                    dispatch_async(s_main_queue, ^{
                        handle_client_message(session, event);
                        xpc_release(event);
                    });
                }
                else if (event == XPC_ERROR_CONNECTION_INVALID || event == XPC_ERROR_CONNECTION_INTERRUPTED)
                {
                    dispatch_async(s_main_queue, ^{
                        [s_sessions removeObject:session];
                        update_host_session();
                    });
                }
            });
            xpc_connection_resume(client_conn);
        }
    });

    xpc_connection_resume(listener);
}

int main(int argc, const char* argv[])
{
    @autoreleasepool {
        s_main_queue = dispatch_get_main_queue();
        s_sessions = [[NSMutableArray alloc] init];

        // Connect to host D-Bus
        int backend = locationd_dbus_init(NULL, NULL);
        s_host_available = (backend != LOCATIOND_BACKEND_NONE);
        s_last_host_init = [[NSDate date] timeIntervalSince1970];

        const char* custom_service = getenv(LOCATIOND_SERVICE_ENV);
        if (custom_service && custom_service[0] != '\0')
        {
            setup_xpc_service(custom_service);
        }
        else
        {
            setup_xpc_service(LOCATIOND_MACH_SERVICE);
        }
    }

    dispatch_main();
    return 0;
}
