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
#import <darling-stub-forwarding.h>
#include <xpc/xpc.h>
#include <dispatch/dispatch.h>
#include <unistd.h>
#include "locationd_protocol.h"

// How long -requestLocation waits for a fix before reporting kCLErrorLocationUnknown.
#define REQUEST_LOCATION_TIMEOUT_SECONDS 10

@class _CLLocationClient;

/*
 * Per-manager state shared with the daemon connection. The connection's queue
 * never touches a CLLocationManager directly: it goes through the token, which
 * holds a non-retained pointer that is cleared (under the token's lock) before
 * the manager is deallocated, and posts delegate callbacks to the run loop of
 * the thread the manager was created on, as documented for CLLocationManager.
 */
@interface _CLLocationClientToken : NSObject
{
@public
	NSRecursiveLock *lock;
	CLLocationManager *manager;   // not retained
	CFRunLoopRef runLoop;         // retained
	BOOL updating;
	BOOL requesting;
	CLLocationAccuracy desiredAccuracy;
	CLLocationDistance distanceFilter;
	CLLocation *location;         // last location delivered to the delegate
	dispatch_source_t timer;      // -requestLocation timeout
}
- (id)initWithManager:(CLLocationManager *)mgr;
- (void)invalidate;
- (void)performOnManager:(void (^)(CLLocationManager *mgr))work;
- (void)cancelTimerLocked;
@end

@interface CLLocationManager ()
- (void)_deliverAuthorizationChange:(BOOL)force;
- (void)_deliverError:(NSError *)error;
@end

@interface _CLLocationClient : NSObject
{
	xpc_connection_t _connection;
	dispatch_queue_t _queue;
	NSMutableArray *_tokens;
	NSLock *_stateLock;
	BOOL _enabled;
	CLAuthorizationStatus _authStatus;
	CLAccuracyAuthorization _accuracyAuth;
	BOOL _hasState;
	CLLocation *_lastLocation;
}
+ (_CLLocationClient *)sharedClient;
- (BOOL)locationServicesEnabled;
- (CLAuthorizationStatus)authorizationStatus;
- (CLAccuracyAuthorization)accuracyAuthorization;
- (void)addToken:(_CLLocationClientToken *)token;
- (void)removeToken:(_CLLocationClientToken *)token;
- (void)updateSubscription;
- (void)requestAuthorization;
- (dispatch_queue_t)queue;
- (void)failToken:(_CLLocationClientToken *)token manager:(CLLocationManager *)mgr withError:(NSError *)err;
@end

@implementation _CLLocationClientToken

- (id)initWithManager:(CLLocationManager *)mgr
{
	self = [super init];
	if (self)
	{
		lock = [[NSRecursiveLock alloc] init];
		manager = mgr;
		runLoop = (CFRunLoopRef) CFRetain(CFRunLoopGetCurrent());
		desiredAccuracy = kCLLocationAccuracyBest;
		distanceFilter = kCLDistanceFilterNone;
	}
	return self;
}

- (void)dealloc
{
	[lock release];
	[location release];
	[super dealloc];
}

- (void)cancelTimerLocked
{
	if (timer)
	{
		dispatch_source_cancel(timer);
		dispatch_release(timer);
		timer = NULL;
	}
}

- (void)invalidate
{
	[lock lock];
	manager = nil;
	[self cancelTimerLocked];
	updating = NO;
	requesting = NO;
	if (runLoop)
	{
		CFRelease(runLoop);
		runLoop = NULL;
	}
	[lock unlock];
}

- (void)performOnManager:(void (^)(CLLocationManager *mgr))work
{
	CFRunLoopRef rl = NULL;

	[lock lock];
	if (manager && runLoop)
		rl = (CFRunLoopRef) CFRetain(runLoop);
	[lock unlock];

	if (!rl)
		return;

	[self retain];
	void (^block)(void) = ^{
		[lock lock];
		CLLocationManager *mgr = manager;
		if (mgr)
			work(mgr);
		[lock unlock];
		[self release];
	};
	CFRunLoopPerformBlock(rl, kCFRunLoopCommonModes, block);
	CFRunLoopWakeUp(rl);
	CFRelease(rl);
}

@end

@implementation _CLLocationClient

+ (_CLLocationClient *)sharedClient
{
	static _CLLocationClient *s_instance = nil;
	static dispatch_once_t onceToken;
	dispatch_once(&onceToken, ^{
		s_instance = [[_CLLocationClient alloc] init];
	});
	return s_instance;
}

- (id)init
{
	self = [super init];
	if (self)
	{
		_queue = dispatch_queue_create("com.apple.corelocation.client", DISPATCH_QUEUE_SERIAL);
		_tokens = [[NSMutableArray alloc] init];
		_stateLock = [[NSLock alloc] init];
		_authStatus = kCLAuthorizationStatusRestricted;
		_accuracyAuth = CLAccuracyAuthorizationFullAccuracy;
	}
	return self;
}

- (dispatch_queue_t)queue
{
	return _queue;
}

// Returns a retained connection to locationd, creating it (and registering with the daemon) if needed.
- (xpc_connection_t)copyConnection
{
	xpc_connection_t conn = NULL;
	BOOL created = NO;

	[_stateLock lock];
	if (!_connection)
	{
		const char *serviceName = getenv(LOCATIOND_SERVICE_ENV);
		if (!serviceName || serviceName[0] == '\0')
			serviceName = LOCATIOND_MACH_SERVICE;

		_connection = xpc_connection_create_mach_service(serviceName, _queue, 0);
		if (_connection)
		{
			xpc_connection_t self_conn = _connection;
			xpc_connection_set_event_handler(_connection, ^(xpc_object_t event) {
				[self handleEvent:event fromConnection:self_conn];
			});
			xpc_connection_resume(_connection);
			created = YES;
		}
	}
	if (_connection)
		conn = (xpc_connection_t) xpc_retain(_connection);
	[_stateLock unlock];

	if (created)
		[self sendRegister:conn];
	return conn;
}

- (void)sendRegister:(xpc_connection_t)conn
{
	xpc_object_t reg = xpc_dictionary_create(NULL, NULL, 0);
	xpc_dictionary_set_string(reg, CL_KEY_OP, CL_OP_REGISTER);
	xpc_dictionary_set_int64(reg, CL_KEY_PID, (int64_t) getpid());
	NSString *bundleId = [[NSBundle mainBundle] bundleIdentifier];
	if (bundleId)
		xpc_dictionary_set_string(reg, CL_KEY_BUNDLE_ID, [bundleId UTF8String]);

	xpc_connection_send_message_with_reply(conn, reg, _queue, ^(xpc_object_t reply) {
		if (reply && xpc_get_type(reply) == XPC_TYPE_DICTIONARY)
		{
			if ([self applyState:reply])
				[self notifyAuthChange];
		}
	});
	xpc_release(reg);
}

- (void)handleEvent:(xpc_object_t)event fromConnection:(xpc_connection_t)conn
{
	if (xpc_get_type(event) == XPC_TYPE_DICTIONARY)
	{
		[self handleDaemonMessage:event];
	}
	else if (event == XPC_ERROR_CONNECTION_INTERRUPTED)
	{
		// The daemon went away; the connection stays usable and reaches a new
		// instance, which has to be told about us and the active subscriptions.
		[_stateLock lock];
		_hasState = NO;
		_enabled = NO;
		_authStatus = kCLAuthorizationStatusRestricted;
		[_stateLock unlock];
		[self notifyAuthChange];
		[self sendRegister:conn];
		[self updateSubscription];
	}
	else if (event == XPC_ERROR_CONNECTION_INVALID)
	{
		[_stateLock lock];
		BOOL current = (_connection == conn);
		if (current)
		{
			xpc_release(_connection);
			_connection = NULL;
			_hasState = NO;
			_enabled = NO;
			_authStatus = kCLAuthorizationStatusRestricted;
		}
		[_stateLock unlock];
		if (current)
			[self notifyAuthChange];
	}
}

// Stores the state fields of a register/get-state reply. Returns YES if the state changed.
- (BOOL)applyState:(xpc_object_t)reply
{
	BOOL enabled = xpc_dictionary_get_bool(reply, CL_KEY_ENABLED);
	CLAuthorizationStatus auth = (CLAuthorizationStatus) xpc_dictionary_get_int64(reply, CL_KEY_AUTH_STATUS);
	CLAccuracyAuthorization acc = (CLAccuracyAuthorization) xpc_dictionary_get_int64(reply, CL_KEY_ACCURACY_AUTH);

	[_stateLock lock];
	BOOL changed = !_hasState || enabled != _enabled || auth != _authStatus || acc != _accuracyAuth;
	_enabled = enabled;
	_authStatus = auth;
	_accuracyAuth = acc;
	_hasState = YES;
	[_stateLock unlock];
	return changed;
}

+ (CLLocation *)locationFromMessage:(xpc_object_t)msg
{
	if (!xpc_dictionary_get_value(msg, CL_KEY_LATITUDE))
		return nil;

	double ts = xpc_dictionary_get_double(msg, CL_KEY_TIMESTAMP);
	NSDate *date = (ts > 0.0) ? [NSDate dateWithTimeIntervalSince1970:ts] : [NSDate date];
	return [[[CLLocation alloc] initWithCoordinate:CLLocationCoordinate2DMake(xpc_dictionary_get_double(msg, CL_KEY_LATITUDE),
	                                                                          xpc_dictionary_get_double(msg, CL_KEY_LONGITUDE))
	                                      altitude:xpc_dictionary_get_double(msg, CL_KEY_ALTITUDE)
	                            horizontalAccuracy:xpc_dictionary_get_double(msg, CL_KEY_HORIZONTAL_ACC)
	                              verticalAccuracy:xpc_dictionary_get_double(msg, CL_KEY_VERTICAL_ACC)
	                                        course:xpc_dictionary_get_double(msg, CL_KEY_COURSE)
	                                         speed:xpc_dictionary_get_double(msg, CL_KEY_SPEED)
	                                     timestamp:date] autorelease];
}

- (NSArray *)copyTokens
{
	NSArray *copy;
	@synchronized (_tokens) {
		copy = [_tokens copy];
	}
	return copy;
}

- (void)handleDaemonMessage:(xpc_object_t)msg
{
	const char *op = xpc_dictionary_get_string(msg, CL_KEY_OP);
	if (!op)
		return;

	if (strcmp(op, CL_OP_LOCATION_UPDATE) == 0)
	{
		CLLocation *loc = [_CLLocationClient locationFromMessage:msg];
		if (!loc)
			return;

		[_stateLock lock];
		[_lastLocation release];
		_lastLocation = [loc retain];
		[_stateLock unlock];

		NSArray *tokens = [self copyTokens];
		for (_CLLocationClientToken *token in tokens)
		{
			[token performOnManager:^(CLLocationManager *mgr) {
				[self deliverLocation:loc toManager:mgr token:token];
			}];
		}
		[tokens release];
	}
	else if (strcmp(op, CL_OP_AUTH_UPDATE) == 0)
	{
		[self applyState:msg];
		[self notifyAuthChange];
	}
	else if (strcmp(op, CL_OP_ERROR) == 0)
	{
		NSError *err = [NSError errorWithDomain:kCLErrorDomain
		                                   code:(NSInteger) xpc_dictionary_get_int64(msg, CL_KEY_ERROR_CODE)
		                               userInfo:nil];
		NSArray *tokens = [self copyTokens];
		for (_CLLocationClientToken *token in tokens)
		{
			[token performOnManager:^(CLLocationManager *mgr) {
				[self failToken:token manager:mgr withError:err];
			}];
		}
		[tokens release];
	}
}

// Runs on the manager's run loop with the token's lock held.
- (void)deliverLocation:(CLLocation *)loc toManager:(CLLocationManager *)mgr token:(_CLLocationClientToken *)token
{
	if (!token->updating && !token->requesting)
		return;

	BOOL wasRequest = token->requesting;
	CLLocation *old = token->location;

	if (!wasRequest && token->distanceFilter > 0.0 && old &&
	    [old distanceFromLocation:loc] < token->distanceFilter &&
	    loc.horizontalAccuracy >= old.horizontalAccuracy)
		return;

	if (wasRequest)
	{
		token->requesting = NO;
		[token cancelTimerLocked];
	}

	token->location = [loc retain];

	id<CLLocationManagerDelegate> delegate = mgr.delegate;
	if ([delegate respondsToSelector:@selector(locationManager:didUpdateLocations:)])
		[delegate locationManager:mgr didUpdateLocations:[NSArray arrayWithObject:loc]];
	else if ([delegate respondsToSelector:@selector(locationManager:didUpdateToLocation:fromLocation:)])
		[delegate locationManager:mgr didUpdateToLocation:loc fromLocation:old];

	[old release];

	// A completed one-shot request may have been the last reason to keep the host session open.
	if (wasRequest)
		[self updateSubscription];
}

- (void)failToken:(_CLLocationClientToken *)token manager:(CLLocationManager *)mgr withError:(NSError *)err
{
	if (!token->updating && !token->requesting)
		return;

	BOOL wasRequest = token->requesting;
	if (wasRequest)
	{
		token->requesting = NO;
		[token cancelTimerLocked];
	}

	id<CLLocationManagerDelegate> delegate = mgr.delegate;
	if ([delegate respondsToSelector:@selector(locationManager:didFailWithError:)])
		[delegate locationManager:mgr didFailWithError:err];

	if (wasRequest)
		[self updateSubscription];
}

- (void)notifyAuthChange
{
	NSArray *tokens = [self copyTokens];
	for (_CLLocationClientToken *token in tokens)
	{
		[token performOnManager:^(CLLocationManager *mgr) {
			[mgr _deliverAuthorizationChange:NO];
		}];
	}
	[tokens release];
}

- (void)dealloc
{
	if (_connection)
	{
		xpc_connection_cancel(_connection);
		xpc_release(_connection);
	}
	[_tokens release];
	[_stateLock release];
	[_lastLocation release];
	dispatch_release(_queue);
	[super dealloc];
}

// Makes sure the state fields are valid, asking the daemon synchronously if necessary.
- (void)ensureState
{
	[_stateLock lock];
	BOOL have = _hasState;
	[_stateLock unlock];
	if (have)
		return;

	xpc_connection_t conn = [self copyConnection];
	if (!conn)
		return;

	xpc_object_t msg = xpc_dictionary_create(NULL, NULL, 0);
	xpc_dictionary_set_string(msg, CL_KEY_OP, CL_OP_GET_STATE);
	NSString *bundleId = [[NSBundle mainBundle] bundleIdentifier];
	if (bundleId)
		xpc_dictionary_set_string(msg, CL_KEY_BUNDLE_ID, [bundleId UTF8String]);

	xpc_object_t reply = xpc_connection_send_message_with_reply_sync(conn, msg);
	if (reply && xpc_get_type(reply) == XPC_TYPE_DICTIONARY)
		[self applyState:reply];

	xpc_release(msg);
	if (reply)
		xpc_release(reply);
	xpc_release(conn);
}

- (BOOL)locationServicesEnabled
{
	[self ensureState];
	[_stateLock lock];
	BOOL v = _enabled;
	[_stateLock unlock];
	return v;
}

- (CLAuthorizationStatus)authorizationStatus
{
	[self ensureState];
	[_stateLock lock];
	CLAuthorizationStatus v = _authStatus;
	[_stateLock unlock];
	return v;
}

- (CLAccuracyAuthorization)accuracyAuthorization
{
	[self ensureState];
	[_stateLock lock];
	CLAccuracyAuthorization v = _accuracyAuth;
	[_stateLock unlock];
	return v;
}

- (void)addToken:(_CLLocationClientToken *)token
{
	@synchronized (_tokens) {
		[_tokens addObject:token];
	}
}

- (void)removeToken:(_CLLocationClientToken *)token
{
	@synchronized (_tokens) {
		[_tokens removeObject:token];
	}
	[self updateSubscription];
}

// Sends the strictest combined requirements of all active managers (or a stop) to the daemon.
- (void)updateSubscription
{
	dispatch_async(_queue, ^{
		BOOL any = NO;
		BOOL haveAccuracy = NO, haveFilter = NO, noFilter = NO;
		double bestAccuracy = 0.0, minDistance = 0.0;

		NSArray *tokens = [self copyTokens];
		for (_CLLocationClientToken *token in tokens)
		{
			[token->lock lock];
			if (token->updating || token->requesting)
			{
				any = YES;
				// Smaller accuracy values are better (Best and BestForNavigation are negative).
				if (!haveAccuracy || token->desiredAccuracy < bestAccuracy)
					bestAccuracy = token->desiredAccuracy;
				haveAccuracy = YES;

				// A manager without a distance filter needs every update.
				if (token->distanceFilter <= 0.0)
					noFilter = YES;
				else if (!haveFilter || token->distanceFilter < minDistance)
					minDistance = token->distanceFilter;
				haveFilter = YES;
			}
			[token->lock unlock];
		}
		[tokens release];

		if (!any)
		{
			// Nothing to stop if we never connected.
			[_stateLock lock];
			BOOL connected = (_connection != NULL);
			[_stateLock unlock];
			if (!connected)
				return;
		}

		xpc_connection_t conn = [self copyConnection];
		if (!conn)
			return;

		xpc_object_t req = xpc_dictionary_create(NULL, NULL, 0);
		if (any)
		{
			xpc_dictionary_set_string(req, CL_KEY_OP, CL_OP_START_UPDATING);
			xpc_dictionary_set_double(req, CL_KEY_DESIRED_ACCURACY, bestAccuracy);
			xpc_dictionary_set_double(req, CL_KEY_DISTANCE_FILTER, noFilter ? kCLDistanceFilterNone : minDistance);
		}
		else
		{
			xpc_dictionary_set_string(req, CL_KEY_OP, CL_OP_STOP_UPDATING);
		}
		xpc_connection_send_message(conn, req);
		xpc_release(req);
		xpc_release(conn);
	});
}

- (void)requestAuthorization
{
	dispatch_async(_queue, ^{
		xpc_connection_t conn = [self copyConnection];
		if (!conn)
			return;

		xpc_object_t req = xpc_dictionary_create(NULL, NULL, 0);
		xpc_dictionary_set_string(req, CL_KEY_OP, CL_OP_REQUEST_AUTH);
		xpc_connection_send_message_with_reply(conn, req, _queue, ^(xpc_object_t reply) {
			if (reply && xpc_get_type(reply) == XPC_TYPE_DICTIONARY)
			{
				// The reply carries only the authorization fields; keep the rest.
				[_stateLock lock];
				CLAuthorizationStatus auth = (CLAuthorizationStatus) xpc_dictionary_get_int64(reply, CL_KEY_AUTH_STATUS);
				CLAccuracyAuthorization acc = (CLAccuracyAuthorization) xpc_dictionary_get_int64(reply, CL_KEY_ACCURACY_AUTH);
				BOOL changed = (auth != _authStatus || acc != _accuracyAuth);
				_authStatus = auth;
				_accuracyAuth = acc;
				[_stateLock unlock];
				if (changed)
					[self notifyAuthChange];
			}
		});
		xpc_release(req);
		xpc_release(conn);
	});
}

@end

@implementation CLLocationManager

@synthesize delegate = _delegate;
@synthesize distanceFilter = _distanceFilter;
@synthesize desiredAccuracy = _desiredAccuracy;

+ (CLAuthorizationStatus)authorizationStatus
{
	return [[_CLLocationClient sharedClient] authorizationStatus];
}

+ (BOOL)locationServicesEnabled
{
	return [[_CLLocationClient sharedClient] locationServicesEnabled];
}

- (instancetype)init
{
	self = [super init];
	if (self)
	{
		_desiredAccuracy = kCLLocationAccuracyBest;
		_distanceFilter = kCLDistanceFilterNone;

		_CLLocationClient *client = [_CLLocationClient sharedClient];
		_authorizationStatus = [client authorizationStatus];
		_accuracyAuthorization = [client accuracyAuthorization];

		_clientToken = [[_CLLocationClientToken alloc] initWithManager:self];
		[client addToken:_clientToken];
	}
	return self;
}

- (void)dealloc
{
	_CLLocationClientToken *token = _clientToken;
	[[_CLLocationClient sharedClient] removeToken:token];
	[token invalidate];
	[token release];
	[super dealloc];
}

- (void)setDelegate:(id<CLLocationManagerDelegate>)delegate
{
	_delegate = delegate;
	if (delegate)
	{
		// The initial authorization callback, delivered from the run loop and
		// not from within this call.
		[_clientToken performOnManager:^(CLLocationManager *mgr) {
			[mgr _deliverAuthorizationChange:YES];
		}];
	}
}

- (void)setDesiredAccuracy:(CLLocationAccuracy)accuracy
{
	_CLLocationClientToken *token = _clientToken;
	[token->lock lock];
	_desiredAccuracy = accuracy;
	token->desiredAccuracy = accuracy;
	BOOL active = token->updating || token->requesting;
	[token->lock unlock];
	if (active)
		[[_CLLocationClient sharedClient] updateSubscription];
}

- (void)setDistanceFilter:(CLLocationDistance)filter
{
	_CLLocationClientToken *token = _clientToken;
	[token->lock lock];
	_distanceFilter = filter;
	token->distanceFilter = filter;
	BOOL active = token->updating || token->requesting;
	[token->lock unlock];
	if (active)
		[[_CLLocationClient sharedClient] updateSubscription];
}

- (CLLocation *)location
{
	_CLLocationClientToken *token = _clientToken;
	[token->lock lock];
	CLLocation *loc = [[token->location retain] autorelease];
	[token->lock unlock];
	return loc;
}

- (CLAuthorizationStatus)authorizationStatus
{
	return [[_CLLocationClient sharedClient] authorizationStatus];
}

- (CLAccuracyAuthorization)accuracyAuthorization
{
	return [[_CLLocationClient sharedClient] accuracyAuthorization];
}

- (BOOL)locationServicesEnabled
{
	return [[_CLLocationClient sharedClient] locationServicesEnabled];
}

- (void)requestWhenInUseAuthorization
{
	[[_CLLocationClient sharedClient] requestAuthorization];
}

- (void)requestAlwaysAuthorization
{
	[[_CLLocationClient sharedClient] requestAuthorization];
}

- (void)startUpdatingLocation
{
	_CLLocationClientToken *token = _clientToken;
	[token->lock lock];
	token->updating = YES;
	[token->lock unlock];
	[[_CLLocationClient sharedClient] updateSubscription];
}

- (void)stopUpdatingLocation
{
	_CLLocationClientToken *token = _clientToken;
	[token->lock lock];
	token->updating = NO;
	token->requesting = NO;
	[token cancelTimerLocked];
	[token->lock unlock];
	[[_CLLocationClient sharedClient] updateSubscription];
}

- (void)requestLocation
{
	_CLLocationClientToken *token = _clientToken;
	_CLLocationClient *client = [_CLLocationClient sharedClient];

	CLAuthorizationStatus status = [client authorizationStatus];
	if (![client locationServicesEnabled] || status == kCLAuthorizationStatusDenied ||
	    status == kCLAuthorizationStatusRestricted)
	{
		// No location can be delivered: report it as the system does for an unauthorized client.
		NSError *err = [NSError errorWithDomain:kCLErrorDomain code:kCLErrorDenied userInfo:nil];
		[token performOnManager:^(CLLocationManager *mgr) {
			[mgr _deliverError:err];
		}];
		return;
	}

	[token->lock lock];
	if (token->requesting)
	{
		[token->lock unlock];
		return;
	}
	token->requesting = YES;

	dispatch_source_t timer = dispatch_source_create(DISPATCH_SOURCE_TYPE_TIMER, 0, 0, [client queue]);
	token->timer = timer;
	dispatch_source_set_timer(timer, dispatch_time(DISPATCH_TIME_NOW, REQUEST_LOCATION_TIMEOUT_SECONDS * NSEC_PER_SEC),
	                          DISPATCH_TIME_FOREVER, 100 * NSEC_PER_MSEC);
	// The handler block retains the token until the source is cancelled.
	dispatch_source_set_event_handler(timer, ^{
		NSError *err = [NSError errorWithDomain:kCLErrorDomain code:kCLErrorLocationUnknown userInfo:nil];
		[token performOnManager:^(CLLocationManager *mgr) {
			[client failToken:token manager:mgr withError:err];
		}];
	});
	dispatch_resume(timer);
	[token->lock unlock];

	[client updateSubscription];
}

- (void)_deliverAuthorizationChange:(BOOL)force
{
	_CLLocationClient *client = [_CLLocationClient sharedClient];
	CLAuthorizationStatus status = [client authorizationStatus];
	CLAccuracyAuthorization accuracy = [client accuracyAuthorization];

	if (!force && status == _authorizationStatus && accuracy == _accuracyAuthorization)
		return;

	_authorizationStatus = status;
	_accuracyAuthorization = accuracy;

	id<CLLocationManagerDelegate> delegate = _delegate;
	if ([delegate respondsToSelector:@selector(locationManagerDidChangeAuthorization:)])
		[delegate locationManagerDidChangeAuthorization:self];
	else if ([delegate respondsToSelector:@selector(locationManager:didChangeAuthorizationStatus:)])
		[delegate locationManager:self didChangeAuthorizationStatus:status];
}

- (void)_deliverError:(NSError *)error
{
	id<CLLocationManagerDelegate> delegate = _delegate;
	if ([delegate respondsToSelector:@selector(locationManager:didFailWithError:)])
		[delegate locationManager:self didFailWithError:error];
}

// Methods of the public API that are not implemented yet stay no-ops.
- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

@end
