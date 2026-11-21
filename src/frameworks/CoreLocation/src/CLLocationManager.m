/*
 This file is part of Darling.

 Copyright (C) 2019 Lubos Dolezel
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

#import <CoreLocation/CLLocationManager.h>
#import <CoreLocation/CLErrorDomain.h>
#include <xpc/xpc.h>
#include <dispatch/dispatch.h>
#include <unistd.h>
#include "locationd_protocol.h"

@interface CLLocationManager ()
- (void)_deliverLocations:(NSArray<CLLocation *> *)locations;
- (void)_deliverAuthChange;
- (void)_deliverError:(NSError *)error;
@property (nonatomic, assign) BOOL isUpdating;
@property (nonatomic, assign) BOOL hasRequestedLocation;
@property (nonatomic, retain) dispatch_source_t requestLocationTimer;
@end

@interface _CLLocationClientManager : NSObject
{
    xpc_connection_t _connection;
    dispatch_queue_t _queue;
    NSMutableArray<CLLocationManager *> *_managers;
    BOOL _enabled;
    CLAuthorizationStatus _authStatus;
    CLAccuracyAuthorization _accuracyAuth;
    CLLocation *_lastLocation;
    BOOL _hasState;
}

+ (instancetype)sharedManager;
- (BOOL)locationServicesEnabled;
- (CLAuthorizationStatus)authorizationStatus;
- (CLAccuracyAuthorization)accuracyAuthorization;
- (void)registerManager:(CLLocationManager *)mgr;
- (void)unregisterManager:(CLLocationManager *)mgr;
- (void)updateSubscription;
- (void)requestAuthorization;
@end

@implementation _CLLocationClientManager

+ (instancetype)sharedManager
{
    static _CLLocationClientManager *s_instance = nil;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        s_instance = [[_CLLocationClientManager alloc] init];
    });
    return s_instance;
}

- (instancetype)init
{
    self = [super init];
    if (self)
    {
        _queue = dispatch_queue_create("com.apple.corelocation.client", DISPATCH_QUEUE_SERIAL);
        _managers = [[NSMutableArray alloc] init];
        _enabled = NO;
        _authStatus = kCLAuthorizationStatusRestricted;
        _accuracyAuth = CLAccuracyAuthorizationFullAccuracy;
        [self ensureConnection];
    }
    return self;
}

- (void)dealloc
{
    if (_connection)
    {
        xpc_connection_cancel(_connection);
        xpc_release(_connection);
        _connection = NULL;
    }
    [_managers release];
    [_lastLocation release];
    dispatch_release(_queue);
    [super dealloc];
}

- (void)ensureConnection
{
    if (_connection)
        return;

    const char *serviceName = getenv(LOCATIOND_SERVICE_ENV);
    if (!serviceName || serviceName[0] == '\0')
        serviceName = LOCATIOND_MACH_SERVICE;

    _connection = xpc_connection_create_mach_service(serviceName, _queue, 0);
    if (!_connection)
        return;

    __block _CLLocationClientManager *weakSelf = self;
    xpc_connection_set_event_handler(_connection, ^(xpc_object_t event) {
        if (xpc_get_type(event) == XPC_TYPE_DICTIONARY)
        {
            [weakSelf handleDaemonMessage:event];
        }
        else if (event == XPC_ERROR_CONNECTION_INVALID || event == XPC_ERROR_CONNECTION_INTERRUPTED)
        {
            dispatch_async(weakSelf->_queue, ^{
                if (weakSelf->_connection)
                {
                    xpc_release(weakSelf->_connection);
                    weakSelf->_connection = NULL;
                }
                weakSelf->_enabled = NO;
                weakSelf->_authStatus = kCLAuthorizationStatusRestricted;
                [weakSelf notifyAuthChange];
            });
        }
    });

    xpc_connection_resume(_connection);

    // Initial checkin
    xpc_object_t reg = xpc_dictionary_create(NULL, NULL, 0);
    xpc_dictionary_set_string(reg, CL_KEY_OP, CL_OP_REGISTER);
    xpc_dictionary_set_int64(reg, CL_KEY_PID, (int64_t)getpid());
    NSString *bundleId = [[NSBundle mainBundle] bundleIdentifier];
    if (bundleId)
        xpc_dictionary_set_string(reg, CL_KEY_BUNDLE_ID, [bundleId UTF8String]);

    xpc_connection_send_message_with_reply(_connection, reg, _queue, ^(xpc_object_t reply) {
        if (reply && xpc_get_type(reply) == XPC_TYPE_DICTIONARY)
        {
            weakSelf->_enabled = xpc_dictionary_get_bool(reply, CL_KEY_ENABLED);
            weakSelf->_authStatus = (CLAuthorizationStatus)xpc_dictionary_get_int64(reply, CL_KEY_AUTH_STATUS);
            weakSelf->_accuracyAuth = (CLAccuracyAuthorization)xpc_dictionary_get_int64(reply, CL_KEY_ACCURACY_AUTH);
            weakSelf->_hasState = YES;

            if (xpc_dictionary_get_value(reply, CL_KEY_LATITUDE))
            {
                double lat = xpc_dictionary_get_double(reply, CL_KEY_LATITUDE);
                double lon = xpc_dictionary_get_double(reply, CL_KEY_LONGITUDE);
                double alt = xpc_dictionary_get_double(reply, CL_KEY_ALTITUDE);
                double hAcc = xpc_dictionary_get_double(reply, CL_KEY_HORIZONTAL_ACC);
                double vAcc = xpc_dictionary_get_double(reply, CL_KEY_VERTICAL_ACC);
                double course = xpc_dictionary_get_double(reply, CL_KEY_COURSE);
                double speed = xpc_dictionary_get_double(reply, CL_KEY_SPEED);
                double ts = xpc_dictionary_get_double(reply, CL_KEY_TIMESTAMP);

                NSDate *date = (ts > 0.0) ? [NSDate dateWithTimeIntervalSince1970:ts] : [NSDate date];
                [weakSelf->_lastLocation release];
                weakSelf->_lastLocation = [[CLLocation alloc] initWithCoordinate:CLLocationCoordinate2DMake(lat, lon)
                                                                       altitude:alt
                                                             horizontalAccuracy:hAcc
                                                               verticalAccuracy:vAcc
                                                                         course:course
                                                                          speed:speed
                                                                      timestamp:date];
            }
            [weakSelf notifyAuthChange];
        }
    });
    xpc_release(reg);
}

- (void)handleDaemonMessage:(xpc_object_t)msg
{
    const char *op = xpc_dictionary_get_string(msg, CL_KEY_OP);
    if (!op)
        return;

    if (strcmp(op, CL_OP_LOCATION_UPDATE) == 0)
    {
        double lat = xpc_dictionary_get_double(msg, CL_KEY_LATITUDE);
        double lon = xpc_dictionary_get_double(msg, CL_KEY_LONGITUDE);
        double alt = xpc_dictionary_get_double(msg, CL_KEY_ALTITUDE);
        double hAcc = xpc_dictionary_get_double(msg, CL_KEY_HORIZONTAL_ACC);
        double vAcc = xpc_dictionary_get_double(msg, CL_KEY_VERTICAL_ACC);
        double course = xpc_dictionary_get_double(msg, CL_KEY_COURSE);
        double speed = xpc_dictionary_get_double(msg, CL_KEY_SPEED);
        double ts = xpc_dictionary_get_double(msg, CL_KEY_TIMESTAMP);

        NSDate *date = (ts > 0.0) ? [NSDate dateWithTimeIntervalSince1970:ts] : [NSDate date];
        CLLocation *loc = [[[CLLocation alloc] initWithCoordinate:CLLocationCoordinate2DMake(lat, lon)
                                                         altitude:alt
                                               horizontalAccuracy:hAcc
                                                 verticalAccuracy:vAcc
                                                           course:course
                                                            speed:speed
                                                        timestamp:date] autorelease];

        [_lastLocation release];
        _lastLocation = [loc retain];

        NSArray *mgrs = nil;
        @synchronized(_managers) {
            mgrs = [_managers copy];
        }

        for (CLLocationManager *mgr in mgrs)
        {
            if (!mgr.isUpdating && !mgr.hasRequestedLocation)
                continue;

            if (mgr.distanceFilter > 0.0 && mgr.location)
            {
                double dist = [mgr.location distanceFromLocation:loc];
                if (dist < mgr.distanceFilter && loc.horizontalAccuracy >= mgr.location.horizontalAccuracy)
                {
                    if (!mgr.hasRequestedLocation)
                        continue;
                }
            }

            if (mgr.hasRequestedLocation)
            {
                mgr.hasRequestedLocation = NO;
                if (mgr.requestLocationTimer)
                {
                    dispatch_source_cancel(mgr.requestLocationTimer);
                    mgr.requestLocationTimer = NULL;
                }
            }

            [mgr _deliverLocations:@[loc]];
        }
        [mgrs release];
    }
    else if (strcmp(op, CL_OP_AUTH_UPDATE) == 0)
    {
        _authStatus = (CLAuthorizationStatus)xpc_dictionary_get_int64(msg, CL_KEY_AUTH_STATUS);
        _accuracyAuth = (CLAccuracyAuthorization)xpc_dictionary_get_int64(msg, CL_KEY_ACCURACY_AUTH);
        [self notifyAuthChange];
    }
    else if (strcmp(op, CL_OP_ERROR) == 0)
    {
        int64_t code = xpc_dictionary_get_int64(msg, CL_KEY_ERROR_CODE);
        NSError *err = [NSError errorWithDomain:kCLErrorDomain code:(NSInteger)code userInfo:nil];

        NSArray *mgrs = nil;
        @synchronized(_managers) {
            mgrs = [_managers copy];
        }

        for (CLLocationManager *mgr in mgrs)
        {
            if (mgr.isUpdating || mgr.hasRequestedLocation)
            {
                if (mgr.hasRequestedLocation)
                {
                    mgr.hasRequestedLocation = NO;
                    if (mgr.requestLocationTimer)
                    {
                        dispatch_source_cancel(mgr.requestLocationTimer);
                        mgr.requestLocationTimer = NULL;
                    }
                }
                [mgr _deliverError:err];
            }
        }
        [mgrs release];
    }
}

- (void)notifyAuthChange
{
    NSArray *mgrs = nil;
    @synchronized(_managers) {
        mgrs = [_managers copy];
    }
    for (CLLocationManager *mgr in mgrs)
    {
        [mgr _deliverAuthChange];
    }
    [mgrs release];
}

- (void)queryStateSync
{
    if (_hasState)
        return;

    const char *serviceName = getenv(LOCATIOND_SERVICE_ENV);
    if (!serviceName || serviceName[0] == '\0')
        serviceName = LOCATIOND_MACH_SERVICE;

    xpc_connection_t conn = xpc_connection_create_mach_service(serviceName, NULL, 0);
    if (!conn)
        return;

    xpc_connection_set_event_handler(conn, ^(xpc_object_t event) {});
    xpc_connection_resume(conn);

    xpc_object_t msg = xpc_dictionary_create(NULL, NULL, 0);
    xpc_dictionary_set_string(msg, CL_KEY_OP, CL_OP_GET_STATE);
    NSString *bundleId = [[NSBundle mainBundle] bundleIdentifier];
    if (bundleId)
        xpc_dictionary_set_string(msg, CL_KEY_BUNDLE_ID, [bundleId UTF8String]);

    xpc_object_t reply = xpc_connection_send_message_with_reply_sync(conn, msg);
    if (reply && xpc_get_type(reply) == XPC_TYPE_DICTIONARY)
    {
        _enabled = xpc_dictionary_get_bool(reply, CL_KEY_ENABLED);
        _authStatus = (CLAuthorizationStatus)xpc_dictionary_get_int64(reply, CL_KEY_AUTH_STATUS);
        _accuracyAuth = (CLAccuracyAuthorization)xpc_dictionary_get_int64(reply, CL_KEY_ACCURACY_AUTH);
        _hasState = YES;
    }

    xpc_release(msg);
    if (reply) xpc_release(reply);
    xpc_connection_cancel(conn);
    xpc_release(conn);
}

- (BOOL)locationServicesEnabled
{
    [self queryStateSync];
    return _enabled;
}

- (CLAuthorizationStatus)authorizationStatus
{
    [self queryStateSync];
    return _authStatus;
}

- (CLAccuracyAuthorization)accuracyAuthorization
{
    [self queryStateSync];
    return _accuracyAuth;
}

- (void)registerManager:(CLLocationManager *)mgr
{
    @synchronized(_managers) {
        if (![_managers containsObject:mgr])
            [_managers addObject:mgr];
    }
    [self ensureConnection];
}

- (void)unregisterManager:(CLLocationManager *)mgr
{
    @synchronized(_managers) {
        [_managers removeObject:mgr];
    }
    [self updateSubscription];
}

- (void)updateSubscription
{
    dispatch_async(_queue, ^{
        BOOL anyUpdating = NO;
        double bestAccuracy = -1.0;
        double minDistance = -1.0;

        NSArray *mgrs = nil;
        @synchronized(self->_managers) {
            mgrs = [self->_managers copy];
        }

        for (CLLocationManager *mgr in mgrs)
        {
            if (mgr.isUpdating || mgr.hasRequestedLocation)
            {
                anyUpdating = YES;
                if (bestAccuracy < 0.0 || (mgr.desiredAccuracy >= 0.0 && mgr.desiredAccuracy < bestAccuracy))
                    bestAccuracy = mgr.desiredAccuracy;
                if (mgr.distanceFilter >= 0.0)
                {
                    if (minDistance < 0.0 || mgr.distanceFilter < minDistance)
                        minDistance = mgr.distanceFilter;
                }
            }
        }
        [mgrs release];

        if (!self->_connection)
            [self ensureConnection];

        if (self->_connection)
        {
            if (anyUpdating)
            {
                xpc_object_t req = xpc_dictionary_create(NULL, NULL, 0);
                xpc_dictionary_set_string(req, CL_KEY_OP, CL_OP_START_UPDATING);
                xpc_dictionary_set_double(req, CL_KEY_DESIRED_ACCURACY, bestAccuracy);
                xpc_dictionary_set_double(req, CL_KEY_DISTANCE_FILTER, minDistance);
                xpc_connection_send_message(self->_connection, req);
                xpc_release(req);
            }
            else
            {
                xpc_object_t req = xpc_dictionary_create(NULL, NULL, 0);
                xpc_dictionary_set_string(req, CL_KEY_OP, CL_OP_STOP_UPDATING);
                xpc_connection_send_message(self->_connection, req);
                xpc_release(req);
            }
        }
    });
}

- (void)requestAuthorization
{
    dispatch_async(_queue, ^{
        if (!self->_connection)
            [self ensureConnection];

        if (self->_connection)
        {
            xpc_object_t req = xpc_dictionary_create(NULL, NULL, 0);
            xpc_dictionary_set_string(req, CL_KEY_OP, CL_OP_REQUEST_AUTH);
            xpc_connection_send_message_with_reply(self->_connection, req, self->_queue, ^(xpc_object_t reply) {
                if (reply && xpc_get_type(reply) == XPC_TYPE_DICTIONARY)
                {
                    self->_authStatus = (CLAuthorizationStatus)xpc_dictionary_get_int64(reply, CL_KEY_AUTH_STATUS);
                    self->_accuracyAuth = (CLAccuracyAuthorization)xpc_dictionary_get_int64(reply, CL_KEY_ACCURACY_AUTH);
                    [self notifyAuthChange];
                }
            });
            xpc_release(req);
        }
    });
}

@end

@implementation CLLocationManager

@synthesize delegate = _delegate;
@synthesize distanceFilter = _distanceFilter;
@synthesize desiredAccuracy = _desiredAccuracy;
@synthesize location = _location;
@synthesize authorizationStatus = _authorizationStatus;
@synthesize accuracyAuthorization = _accuracyAuthorization;
@synthesize isUpdating = _isUpdating;
@synthesize hasRequestedLocation = _hasRequestedLocation;
@synthesize requestLocationTimer = _requestLocationTimer;

+ (CLAuthorizationStatus)authorizationStatus
{
    return [[_CLLocationClientManager sharedManager] authorizationStatus];
}

+ (BOOL)locationServicesEnabled
{
    return [[_CLLocationClientManager sharedManager] locationServicesEnabled];
}

- (instancetype)init
{
    self = [super init];
    if (self)
    {
        _desiredAccuracy = kCLLocationAccuracyBest;
        _distanceFilter = kCLDistanceFilterNone;
        _authorizationStatus = [[_CLLocationClientManager sharedManager] authorizationStatus];
        _accuracyAuthorization = [[_CLLocationClientManager sharedManager] accuracyAuthorization];
        [[_CLLocationClientManager sharedManager] registerManager:self];
    }
    return self;
}

- (void)dealloc
{
    [[_CLLocationClientManager sharedManager] unregisterManager:self];
    if (_requestLocationTimer)
    {
        dispatch_source_cancel(_requestLocationTimer);
        _requestLocationTimer = NULL;
    }
    [_location release];
    [super dealloc];
}

- (void)setDelegate:(id<CLLocationManagerDelegate>)delegate
{
    _delegate = delegate;
    if (_delegate)
    {
        // Deliver initial authorization callback on the run loop
        CFRunLoopRef rl = CFRunLoopGetCurrent();
        CFRunLoopPerformBlock(rl, kCFRunLoopCommonModes, ^{
            [self _deliverAuthChange];
        });
        CFRunLoopWakeUp(rl);
    }
}

- (void)requestWhenInUseAuthorization
{
    [[_CLLocationClientManager sharedManager] requestAuthorization];
}

- (void)requestAlwaysAuthorization
{
    [[_CLLocationClientManager sharedManager] requestAuthorization];
}

- (void)startUpdatingLocation
{
    _isUpdating = YES;
    [[_CLLocationClientManager sharedManager] updateSubscription];
}

- (void)stopUpdatingLocation
{
    _isUpdating = NO;
    _hasRequestedLocation = NO;
    if (_requestLocationTimer)
    {
        dispatch_source_cancel(_requestLocationTimer);
        _requestLocationTimer = NULL;
    }
    [[_CLLocationClientManager sharedManager] updateSubscription];
}

- (void)requestLocation
{
    _hasRequestedLocation = YES;
    [[_CLLocationClientManager sharedManager] updateSubscription];

    if (_requestLocationTimer)
    {
        dispatch_source_cancel(_requestLocationTimer);
        _requestLocationTimer = NULL;
    }

    _requestLocationTimer = dispatch_source_create(DISPATCH_SOURCE_TYPE_TIMER, 0, 0, dispatch_get_main_queue());
    dispatch_source_set_timer(_requestLocationTimer, dispatch_time(DISPATCH_TIME_NOW, 10 * NSEC_PER_SEC),
                              DISPATCH_TIME_FOREVER, 500 * NSEC_PER_MSEC);
    __block CLLocationManager *weakSelf = self;
    dispatch_source_set_event_handler(_requestLocationTimer, ^{
        if (weakSelf.hasRequestedLocation)
        {
            weakSelf.hasRequestedLocation = NO;
            if (weakSelf.requestLocationTimer)
            {
                dispatch_source_cancel(weakSelf.requestLocationTimer);
                weakSelf.requestLocationTimer = NULL;
            }
            NSError *err = [NSError errorWithDomain:kCLErrorDomain code:kCLErrorLocationUnknown userInfo:nil];
            [weakSelf _deliverError:err];
        }
    });
    dispatch_resume(_requestLocationTimer);
}

- (void)_deliverLocations:(NSArray<CLLocation *> *)locations
{
    if ([locations count] == 0)
        return;

    CLLocation *newLoc = [locations firstObject];
    CLLocation *oldLoc = _location;
    _location = [newLoc copy];

    dispatch_async(dispatch_get_main_queue(), ^{
        if (!self->_delegate)
            return;

        if ([self->_delegate respondsToSelector:@selector(locationManager:didUpdateLocations:)])
        {
            [self->_delegate locationManager:self didUpdateLocations:locations];
        }
        else if ([self->_delegate respondsToSelector:@selector(locationManager:didUpdateToLocation:fromLocation:)])
        {
            [self->_delegate locationManager:self didUpdateToLocation:newLoc fromLocation:oldLoc];
        }
    });

    [oldLoc release];
}

- (void)_deliverAuthChange
{
    _authorizationStatus = [[_CLLocationClientManager sharedManager] authorizationStatus];
    _accuracyAuthorization = [[_CLLocationClientManager sharedManager] accuracyAuthorization];

    dispatch_async(dispatch_get_main_queue(), ^{
        if (!self->_delegate)
            return;

        if ([self->_delegate respondsToSelector:@selector(locationManagerDidChangeAuthorization:)])
        {
            [self->_delegate locationManagerDidChangeAuthorization:self];
        }
        else if ([self->_delegate respondsToSelector:@selector(locationManager:didChangeAuthorizationStatus:)])
        {
            [self->_delegate locationManager:self didChangeAuthorizationStatus:self->_authorizationStatus];
        }
    });
}

- (void)_deliverError:(NSError *)error
{
    dispatch_async(dispatch_get_main_queue(), ^{
        if (self->_delegate && [self->_delegate respondsToSelector:@selector(locationManager:didFailWithError:)])
        {
            [self->_delegate locationManager:self didFailWithError:error];
        }
    });
}

@end
