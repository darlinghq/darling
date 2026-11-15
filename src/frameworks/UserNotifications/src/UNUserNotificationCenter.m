/*
 This file is part of Darling.

 Copyright (C) 2021 Lubos Dolezel
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

#import <UserNotifications/UNUserNotificationCenter.h>
#import <UserNotifications/UNNotificationSettings.h>
#import <UserNotifications/UNNotificationRequest.h>
#import <UserNotifications/UNNotificationContent.h>
#import <UserNotifications/UNMutableNotificationContent.h>
#import <UserNotifications/UNNotification.h>
#import <UserNotifications/UNNotificationResponse.h>
#import <UserNotifications/UNNotificationAction.h>
#import <UserNotifications/UNNotificationCategory.h>
#import <UserNotifications/UNNotificationAttachment.h>
#import <UserNotifications/UNNotificationSound.h>
#import <UserNotifications/UNTimeIntervalNotificationTrigger.h>
#import <UserNotifications/UNCalendarNotificationTrigger.h>

#include <xpc/xpc.h>
#import <objc/runtime.h>
#import <objc/message.h>
#include <dlfcn.h>
#include "../../../usernoted/usernoted_protocol.h"

static const char kCategoriesKey;

static xpc_connection_t s_usernoted_conn = NULL;
static dispatch_once_t s_conn_once;

@implementation UNUserNotificationCenter

@synthesize delegate = _delegate;

static UNUserNotificationCenter *s_currentCenter = nil;

/*
 * Whether this process is the active (foreground) application. A process that never created an
 * NSApplication (a command-line tool, a daemon) is not. NSApp is read without being created.
 */
static BOOL application_is_active(void)
{
    id *appPointer = (id *) dlsym(RTLD_DEFAULT, "NSApp");
    id app = appPointer ? *appPointer : nil;
    if (app == nil || ![app respondsToSelector:@selector(isActive)])
        return NO;
    return ((BOOL (*)(id, SEL)) objc_msgSend)(app, @selector(isActive));
}

static UNNotification *notification_from_message(xpc_object_t msg)
{
    const char *ident = xpc_dictionary_get_string(msg, UN_KEY_NOTIFICATION_ID);
    const char *title = xpc_dictionary_get_string(msg, UN_KEY_TITLE);
    const char *subtitle = xpc_dictionary_get_string(msg, UN_KEY_SUBTITLE);
    const char *body = xpc_dictionary_get_string(msg, UN_KEY_BODY);
    const char *category_id = xpc_dictionary_get_string(msg, UN_KEY_CATEGORY_ID);

    UNMutableNotificationContent *content = [[[UNMutableNotificationContent alloc] init] autorelease];
    if (title) content.title = [NSString stringWithUTF8String:title];
    if (subtitle) content.subtitle = [NSString stringWithUTF8String:subtitle];
    if (body) content.body = [NSString stringWithUTF8String:body];
    if (category_id) content.categoryIdentifier = [NSString stringWithUTF8String:category_id];

    NSString *identifier = ident ? [NSString stringWithUTF8String:ident] : @"";
    UNNotificationRequest *req = [UNNotificationRequest requestWithIdentifier:identifier content:content trigger:nil];
    return [UNNotification notificationWithRequest:req date:[NSDate date]];
}

static void send_will_present_reply(xpc_object_t request, BOOL foreground, int64_t options)
{
    xpc_object_t reply = xpc_dictionary_create_reply(request);
    if (reply == NULL)
        return;
    xpc_dictionary_set_bool(reply, UN_KEY_FOREGROUND, foreground);
    xpc_dictionary_set_int64(reply, UN_KEY_PRESENT_OPTIONS, options);
    xpc_connection_send_message(s_usernoted_conn, reply);
    xpc_release(reply);
}

/*
 * The daemon is about to present a notification. In the foreground the app decides, through
 * the delegate, how it is presented; a delegate that does not implement the method silences it,
 * as the documented default does. An app that is not in the foreground gets it presented as usual.
 */
static void handle_will_present(xpc_object_t request)
{
    xpc_retain(request);
    dispatch_async(dispatch_get_main_queue(), ^{
        if (!application_is_active())
        {
            send_will_present_reply(request, NO, 0);
            xpc_release(request);
            return;
        }

        id<UNUserNotificationCenterDelegate> del = s_currentCenter.delegate;
        SEL sel = @selector(userNotificationCenter:willPresentNotification:withCompletionHandler:);
        if (![del respondsToSelector:sel])
        {
            send_will_present_reply(request, YES, 0);
            xpc_release(request);
            return;
        }

        UNNotification *notif = notification_from_message(request);
        __block BOOL replied = NO;
        [del userNotificationCenter:s_currentCenter willPresentNotification:notif
              withCompletionHandler:^(UNNotificationPresentationOptions options) {
            dispatch_async(dispatch_get_main_queue(), ^{
                if (replied)
                    return;
                replied = YES;
                send_will_present_reply(request, YES, (int64_t) options);
                xpc_release(request);
            });
        }];
    });
}

static void handle_daemon_message(xpc_object_t msg)
{
    const char *op = xpc_dictionary_get_string(msg, UN_KEY_OP);
    if (!op)
        return;

    if (strcmp(op, UN_OP_WILL_PRESENT) == 0)
    {
        handle_will_present(msg);
        return;
    }

    if (strcmp(op, UN_OP_ACTION_INVOKED) == 0)
    {
        const char *ident = xpc_dictionary_get_string(msg, UN_KEY_NOTIFICATION_ID);
        const char *action_key = xpc_dictionary_get_string(msg, UN_KEY_ACTION_KEY);
        const char *title = xpc_dictionary_get_string(msg, UN_KEY_TITLE);
        const char *subtitle = xpc_dictionary_get_string(msg, UN_KEY_SUBTITLE);
        const char *body = xpc_dictionary_get_string(msg, UN_KEY_BODY);
        const char *category_id = xpc_dictionary_get_string(msg, UN_KEY_CATEGORY_ID);

        UNMutableNotificationContent *content = [[[UNMutableNotificationContent alloc] init] autorelease];
        if (title) content.title = [NSString stringWithUTF8String:title];
        if (subtitle) content.subtitle = [NSString stringWithUTF8String:subtitle];
        if (body) content.body = [NSString stringWithUTF8String:body];
        if (category_id) content.categoryIdentifier = [NSString stringWithUTF8String:category_id];

        NSString *identifier = ident ? [NSString stringWithUTF8String:ident] : @"";
        UNNotificationRequest *req = [UNNotificationRequest requestWithIdentifier:identifier content:content trigger:nil];
        UNNotification *notif = [UNNotification notificationWithRequest:req date:[NSDate date]];

        NSString *actionId = nil;
        if (!action_key || strcmp(action_key, "default") == 0)
        {
            actionId = UNNotificationDefaultActionIdentifier;
        }
        else
        {
            actionId = [NSString stringWithUTF8String:action_key];
        }

        UNNotificationResponse *resp = [UNNotificationResponse responseWithNotification:notif actionIdentifier:actionId];

        dispatch_async(dispatch_get_main_queue(), ^{
            id<UNUserNotificationCenterDelegate> del = s_currentCenter.delegate;
            if ([del respondsToSelector:@selector(userNotificationCenter:didReceiveNotificationResponse:withCompletionHandler:)])
            {
                [del userNotificationCenter:s_currentCenter didReceiveNotificationResponse:resp withCompletionHandler:^{}];
            }
        });
    }
}

static xpc_connection_t get_usernoted_connection(void)
{
    dispatch_once(&s_conn_once, ^{
        const char *service_name = getenv(USERNOTED_SERVICE_ENV);
        if (!service_name)
            service_name = USERNOTED_MACH_SERVICE;

        s_usernoted_conn = xpc_connection_create_mach_service(service_name,
            NULL, 0);

        xpc_connection_set_event_handler(s_usernoted_conn, ^(xpc_object_t event) {
            if (xpc_get_type(event) == XPC_TYPE_DICTIONARY)
            {
                handle_daemon_message(event);
            }
        });
        xpc_connection_resume(s_usernoted_conn);

        /* Check in with daemon */
        NSBundle *bundle = [NSBundle mainBundle];
        NSString *bid = [bundle bundleIdentifier] ? [bundle bundleIdentifier] : [[NSProcessInfo processInfo] processName];
        NSString *bpath = [bundle bundlePath];
        NSString *epath = [bundle executablePath];

        xpc_object_t checkin = xpc_dictionary_create(NULL, NULL, 0);
        xpc_dictionary_set_string(checkin, UN_KEY_OP, UN_OP_CHECKIN);
        if (bid) xpc_dictionary_set_string(checkin, UN_KEY_BUNDLE_ID, [bid UTF8String]);
        if (bpath) xpc_dictionary_set_string(checkin, UN_KEY_BUNDLE_PATH, [bpath UTF8String]);
        if (epath) xpc_dictionary_set_string(checkin, UN_KEY_EXEC_PATH, [epath UTF8String]);

        xpc_connection_send_message(s_usernoted_conn, checkin);
        xpc_release(checkin);
    });

    return s_usernoted_conn;
}

+ (UNUserNotificationCenter *) currentNotificationCenter
{
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        s_currentCenter = [[UNUserNotificationCenter alloc] init];
    });
    return s_currentCenter;
}

- (instancetype) init
{
    self = [super init];
    if (self) {
        get_usernoted_connection();
    }
    return self;
}

- (BOOL) supportsContentExtensions
{
    return NO;
}

- (void) requestAuthorizationWithOptions: (UNAuthorizationOptions) options
                       completionHandler: (void (^)(BOOL granted, NSError *error)) completionHandler
{
    xpc_connection_t conn = get_usernoted_connection();
    xpc_object_t msg = xpc_dictionary_create(NULL, NULL, 0);
    xpc_dictionary_set_string(msg, UN_KEY_OP, UN_OP_REQUEST_AUTH);

    xpc_connection_send_message_with_reply(conn, msg, dispatch_get_main_queue(), ^(xpc_object_t reply) {
        if (completionHandler) {
            completionHandler(YES, nil);
        }
    });
    xpc_release(msg);
}

- (void) getNotificationSettingsWithCompletionHandler: (void (^)(UNNotificationSettings *settings)) completionHandler
{
    if (completionHandler) {
        dispatch_async(dispatch_get_main_queue(), ^{
            completionHandler([[[UNNotificationSettings alloc] init] autorelease]);
        });
    }
}

- (void) addNotificationRequest: (UNNotificationRequest *) request
           withCompletionHandler: (void (^)(NSError *error)) completionHandler
{
    if (!request) {
        if (completionHandler) {
            dispatch_async(dispatch_get_main_queue(), ^{
                completionHandler(nil);
            });
        }
        return;
    }

    xpc_connection_t conn = get_usernoted_connection();
    xpc_object_t msg = xpc_dictionary_create(NULL, NULL, 0);
    xpc_dictionary_set_string(msg, UN_KEY_OP, UN_OP_POST);

    NSBundle *bundle = [NSBundle mainBundle];
    NSString *bid = [bundle bundleIdentifier] ? [bundle bundleIdentifier] : [[NSProcessInfo processInfo] processName];
    if (bid) xpc_dictionary_set_string(msg, UN_KEY_BUNDLE_ID, [bid UTF8String]);

    if (request.identifier) xpc_dictionary_set_string(msg, UN_KEY_NOTIFICATION_ID, [request.identifier UTF8String]);

    UNNotificationContent *content = request.content;
    if (content) {
        if (content.title) xpc_dictionary_set_string(msg, UN_KEY_TITLE, [content.title UTF8String]);
        if (content.subtitle) xpc_dictionary_set_string(msg, UN_KEY_SUBTITLE, [content.subtitle UTF8String]);
        if (content.body) xpc_dictionary_set_string(msg, UN_KEY_BODY, [content.body UTF8String]);
        if (content.categoryIdentifier) xpc_dictionary_set_string(msg, UN_KEY_CATEGORY_ID, [content.categoryIdentifier UTF8String]);

        /* Interruption level / urgency */
        int urgency = 1; /* active / normal */
        if (content.interruptionLevel == UNNotificationInterruptionLevelPassive) {
            urgency = 0; /* low */
        } else if (content.interruptionLevel == UNNotificationInterruptionLevelTimeSensitive
                   || content.interruptionLevel == UNNotificationInterruptionLevelCritical) {
            urgency = 2; /* critical */
        }
        xpc_dictionary_set_int64(msg, UN_KEY_URGENCY, urgency);

        /* Image attachment hint */
        if (content.attachments.count > 0) {
            for (UNNotificationAttachment *att in content.attachments) {
                if (att.URL && [att.URL isFileURL]) {
                    xpc_dictionary_set_string(msg, UN_KEY_IMAGE_PATH, [att.URL.path UTF8String]);
                    break;
                }
            }
        }

        /* Sound hint */
        if (content.sound) {
            if (content.sound.soundName) {
                xpc_dictionary_set_string(msg, UN_KEY_SOUND_FILE, [content.sound.soundName UTF8String]);
            } else {
                xpc_dictionary_set_string(msg, UN_KEY_SOUND_FILE, "default");
            }
        }

        /* Category actions & custom dismiss */
        NSSet *cats = nil;
        @synchronized (self) {
            cats = objc_getAssociatedObject(self, &kCategoriesKey);
        }
        if (cats && content.categoryIdentifier) {
            for (UNNotificationCategory *cat in cats) {
                if ([cat.identifier isEqualToString:content.categoryIdentifier]) {
                    if (cat.options & UNNotificationCategoryOptionCustomDismissAction) {
                        xpc_dictionary_set_bool(msg, UN_KEY_CUSTOM_DISMISS, true);
                    }
                    if (cat.actions.count > 0) {
                        xpc_object_t act_arr = xpc_array_create(NULL, 0);
                        for (UNNotificationAction *act in cat.actions) {
                            xpc_array_set_string(act_arr, XPC_ARRAY_APPEND, [act.identifier UTF8String]);
                            xpc_array_set_string(act_arr, XPC_ARRAY_APPEND, [act.title UTF8String]);
                        }
                        xpc_dictionary_set_value(msg, UN_KEY_ACTIONS, act_arr);
                        xpc_release(act_arr);
                    }
                    break;
                }
            }
        }
    }

    /* Trigger */
    UNNotificationTrigger *trig = request.trigger;
    if ([trig isKindOfClass:[UNTimeIntervalNotificationTrigger class]]) {
        UNTimeIntervalNotificationTrigger *tit = (UNTimeIntervalNotificationTrigger *)trig;
        xpc_dictionary_set_int64(msg, UN_KEY_TRIGGER_TYPE, 1);
        xpc_dictionary_set_double(msg, UN_KEY_TIME_INTERVAL, tit.timeInterval);
        xpc_dictionary_set_bool(msg, UN_KEY_REPEATS, tit.repeats);
    } else if ([trig isKindOfClass:[UNCalendarNotificationTrigger class]]) {
        UNCalendarNotificationTrigger *cit = (UNCalendarNotificationTrigger *)trig;
        xpc_dictionary_set_int64(msg, UN_KEY_TRIGGER_TYPE, 2);
        xpc_dictionary_set_bool(msg, UN_KEY_REPEATS, cit.repeats);
        NSDate *nextDate = [cit nextTriggerDate];
        if (nextDate) {
            xpc_dictionary_set_double(msg, UN_KEY_FIRE_DATE, [nextDate timeIntervalSince1970]);
        }
    } else {
        xpc_dictionary_set_int64(msg, UN_KEY_TRIGGER_TYPE, 0);
    }

    xpc_dictionary_set_bool(msg, UN_KEY_IS_NSUSER, false);

    xpc_connection_send_message_with_reply(conn, msg, dispatch_get_main_queue(), ^(xpc_object_t reply) {
        if (completionHandler) {
            completionHandler(nil);
        }
    });
    xpc_release(msg);
}

- (void) getPendingNotificationRequestsWithCompletionHandler: (void (^)(NSArray<UNNotificationRequest *> *requests)) completionHandler
{
    if (!completionHandler)
        return;

    xpc_connection_t conn = get_usernoted_connection();
    xpc_object_t msg = xpc_dictionary_create(NULL, NULL, 0);
    xpc_dictionary_set_string(msg, UN_KEY_OP, UN_OP_GET_PENDING);

    NSBundle *bundle = [NSBundle mainBundle];
    NSString *bid = [bundle bundleIdentifier] ? [bundle bundleIdentifier] : [[NSProcessInfo processInfo] processName];
    if (bid) xpc_dictionary_set_string(msg, UN_KEY_BUNDLE_ID, [bid UTF8String]);

    xpc_connection_send_message_with_reply(conn, msg, dispatch_get_main_queue(), ^(xpc_object_t reply) {
        NSMutableArray<UNNotificationRequest *> *results = [NSMutableArray array];
        if (reply && xpc_get_type(reply) == XPC_TYPE_DICTIONARY) {
            xpc_object_t arr = xpc_dictionary_get_value(reply, UN_KEY_PENDING_REQUESTS);
            if (arr && xpc_get_type(arr) == XPC_TYPE_ARRAY) {
                size_t count = xpc_array_get_count(arr);
                for (size_t i = 0; i < count; i++) {
                    xpc_object_t item = xpc_array_get_value(arr, i);
                    const char *nid = xpc_dictionary_get_string(item, UN_KEY_NOTIFICATION_ID);
                    const char *title = xpc_dictionary_get_string(item, UN_KEY_TITLE);
                    const char *subtitle = xpc_dictionary_get_string(item, UN_KEY_SUBTITLE);
                    const char *body = xpc_dictionary_get_string(item, UN_KEY_BODY);
                    const char *category_id = xpc_dictionary_get_string(item, UN_KEY_CATEGORY_ID);
                    int trigger_type = (int) xpc_dictionary_get_int64(item, UN_KEY_TRIGGER_TYPE);
                    double ti = xpc_dictionary_get_double(item, UN_KEY_TIME_INTERVAL);
                    bool rep = xpc_dictionary_get_bool(item, UN_KEY_REPEATS);

                    UNMutableNotificationContent *c = [[[UNMutableNotificationContent alloc] init] autorelease];
                    if (title) c.title = [NSString stringWithUTF8String:title];
                    if (subtitle) c.subtitle = [NSString stringWithUTF8String:subtitle];
                    if (body) c.body = [NSString stringWithUTF8String:body];
                    if (category_id) c.categoryIdentifier = [NSString stringWithUTF8String:category_id];

                    UNNotificationTrigger *t = nil;
                    if (trigger_type == 1) {
                        t = [UNTimeIntervalNotificationTrigger triggerWithTimeInterval:ti repeats:rep];
                    }

                    NSString *ident = nid ? [NSString stringWithUTF8String:nid] : @"";
                    [results addObject:[UNNotificationRequest requestWithIdentifier:ident content:c trigger:t]];
                }
            }
        }
        completionHandler(results);
    });
    xpc_release(msg);
}

- (void) removePendingNotificationRequestsWithIdentifiers: (NSArray<NSString *> *) identifiers
{
    xpc_connection_t conn = get_usernoted_connection();
    xpc_object_t msg = xpc_dictionary_create(NULL, NULL, 0);
    xpc_dictionary_set_string(msg, UN_KEY_OP, UN_OP_REMOVE_PENDING);

    NSBundle *bundle = [NSBundle mainBundle];
    NSString *bid = [bundle bundleIdentifier] ? [bundle bundleIdentifier] : [[NSProcessInfo processInfo] processName];
    if (bid) xpc_dictionary_set_string(msg, UN_KEY_BUNDLE_ID, [bid UTF8String]);

    if (identifiers.count > 0) {
        xpc_object_t arr = xpc_array_create(NULL, 0);
        for (NSString *s in identifiers) {
            xpc_array_set_string(arr, XPC_ARRAY_APPEND, [s UTF8String]);
        }
        xpc_dictionary_set_value(msg, UN_KEY_IDENTIFIERS, arr);
        xpc_release(arr);
    }

    xpc_connection_send_message(conn, msg);
    xpc_release(msg);
}

- (void) removeAllPendingNotificationRequests
{
    xpc_connection_t conn = get_usernoted_connection();
    xpc_object_t msg = xpc_dictionary_create(NULL, NULL, 0);
    xpc_dictionary_set_string(msg, UN_KEY_OP, UN_OP_REMOVE_ALL_PENDING);

    NSBundle *bundle = [NSBundle mainBundle];
    NSString *bid = [bundle bundleIdentifier] ? [bundle bundleIdentifier] : [[NSProcessInfo processInfo] processName];
    if (bid) xpc_dictionary_set_string(msg, UN_KEY_BUNDLE_ID, [bid UTF8String]);

    xpc_connection_send_message(conn, msg);
    xpc_release(msg);
}

- (void) getDeliveredNotificationsWithCompletionHandler: (void (^)(NSArray<UNNotification *> *notifications)) completionHandler
{
    if (!completionHandler)
        return;

    xpc_connection_t conn = get_usernoted_connection();
    xpc_object_t msg = xpc_dictionary_create(NULL, NULL, 0);
    xpc_dictionary_set_string(msg, UN_KEY_OP, UN_OP_GET_DELIVERED);

    NSBundle *bundle = [NSBundle mainBundle];
    NSString *bid = [bundle bundleIdentifier] ? [bundle bundleIdentifier] : [[NSProcessInfo processInfo] processName];
    if (bid) xpc_dictionary_set_string(msg, UN_KEY_BUNDLE_ID, [bid UTF8String]);

    xpc_connection_send_message_with_reply(conn, msg, dispatch_get_main_queue(), ^(xpc_object_t reply) {
        NSMutableArray<UNNotification *> *results = [NSMutableArray array];
        if (reply && xpc_get_type(reply) == XPC_TYPE_DICTIONARY) {
            xpc_object_t arr = xpc_dictionary_get_value(reply, UN_KEY_DELIVERED_NOTIFICATIONS);
            if (arr && xpc_get_type(arr) == XPC_TYPE_ARRAY) {
                size_t count = xpc_array_get_count(arr);
                for (size_t i = 0; i < count; i++) {
                    xpc_object_t item = xpc_array_get_value(arr, i);
                    const char *nid = xpc_dictionary_get_string(item, UN_KEY_NOTIFICATION_ID);
                    const char *title = xpc_dictionary_get_string(item, UN_KEY_TITLE);
                    const char *subtitle = xpc_dictionary_get_string(item, UN_KEY_SUBTITLE);
                    const char *body = xpc_dictionary_get_string(item, UN_KEY_BODY);
                    const char *category_id = xpc_dictionary_get_string(item, UN_KEY_CATEGORY_ID);
                    double fire_epoch = xpc_dictionary_get_double(item, UN_KEY_FIRE_DATE);

                    UNMutableNotificationContent *c = [[[UNMutableNotificationContent alloc] init] autorelease];
                    if (title) c.title = [NSString stringWithUTF8String:title];
                    if (subtitle) c.subtitle = [NSString stringWithUTF8String:subtitle];
                    if (body) c.body = [NSString stringWithUTF8String:body];
                    if (category_id) c.categoryIdentifier = [NSString stringWithUTF8String:category_id];

                    NSString *ident = nid ? [NSString stringWithUTF8String:nid] : @"";
                    UNNotificationRequest *req = [UNNotificationRequest requestWithIdentifier:ident content:c trigger:nil];
                    NSDate *d = fire_epoch > 0 ? [NSDate dateWithTimeIntervalSince1970:fire_epoch] : [NSDate date];
                    [results addObject:[UNNotification notificationWithRequest:req date:d]];
                }
            }
        }
        completionHandler(results);
    });
    xpc_release(msg);
}

- (void) removeDeliveredNotificationsWithIdentifiers: (NSArray<NSString *> *) identifiers
{
    xpc_connection_t conn = get_usernoted_connection();
    xpc_object_t msg = xpc_dictionary_create(NULL, NULL, 0);
    xpc_dictionary_set_string(msg, UN_KEY_OP, UN_OP_REMOVE_DELIVERED);

    NSBundle *bundle = [NSBundle mainBundle];
    NSString *bid = [bundle bundleIdentifier] ? [bundle bundleIdentifier] : [[NSProcessInfo processInfo] processName];
    if (bid) xpc_dictionary_set_string(msg, UN_KEY_BUNDLE_ID, [bid UTF8String]);

    if (identifiers.count > 0) {
        xpc_object_t arr = xpc_array_create(NULL, 0);
        for (NSString *s in identifiers) {
            xpc_array_set_string(arr, XPC_ARRAY_APPEND, [s UTF8String]);
        }
        xpc_dictionary_set_value(msg, UN_KEY_IDENTIFIERS, arr);
        xpc_release(arr);
    }

    xpc_connection_send_message(conn, msg);
    xpc_release(msg);
}

- (void) removeAllDeliveredNotifications
{
    xpc_connection_t conn = get_usernoted_connection();
    xpc_object_t msg = xpc_dictionary_create(NULL, NULL, 0);
    xpc_dictionary_set_string(msg, UN_KEY_OP, UN_OP_REMOVE_ALL_DELIVERED);

    NSBundle *bundle = [NSBundle mainBundle];
    NSString *bid = [bundle bundleIdentifier] ? [bundle bundleIdentifier] : [[NSProcessInfo processInfo] processName];
    if (bid) xpc_dictionary_set_string(msg, UN_KEY_BUNDLE_ID, [bid UTF8String]);

    xpc_connection_send_message(conn, msg);
    xpc_release(msg);
}

- (void) setNotificationCategories: (NSSet<UNNotificationCategory *> *) categories
{
    @synchronized (self) {
        NSSet *copy = [categories copy];
        objc_setAssociatedObject(self, &kCategoriesKey, copy, OBJC_ASSOCIATION_RETAIN_NONATOMIC);
        [copy release];
    }
}

- (void) getNotificationCategoriesWithCompletionHandler: (void (^)(NSSet<UNNotificationCategory *> *categories)) completionHandler
{
    if (completionHandler) {
        NSSet *snapshot;
        @synchronized (self) {
            snapshot = [objc_getAssociatedObject(self, &kCategoriesKey) copy];
        }
        dispatch_async(dispatch_get_main_queue(), ^{
            completionHandler(snapshot ? snapshot : [NSSet set]);
            [snapshot release];
        });
    }
}

@end
