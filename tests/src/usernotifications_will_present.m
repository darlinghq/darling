// CFLAGS: -framework Foundation -framework AppKit -framework UserNotifications
// Copyright (C) 2026 Lubos Dolezel
//
// UNUserNotificationCenterDelegate userNotificationCenter:willPresentNotification:
// withCompletionHandler: is called only for an app that is in the foreground,
// and the options passed to the completion handler decide whether the
// notification is presented (UNNotificationPresentationOptionNone presents
// nothing, so the notification is not delivered).
//
//  1. A process that is not the active application (no NSApplication yet) does
//     not get the call, and its notification is delivered.
//  2. After the process has become the active application: with the options
//     None the notification is not delivered; with Banner it is. This part is
//     skipped when the session does not make the process active.
//
// Prints PASS / FAIL / SKIP lines, then ALL PASSED or FAILED n.
#import <AppKit/AppKit.h>
#import <Foundation/Foundation.h>
#import <UserNotifications/UserNotifications.h>
#include <stdio.h>
#include <stdlib.h>

static int failures;

#define CHECK(cond, name) do { \
	if (cond) printf("PASS: %s\n", name); \
	else { printf("FAIL: %s\n", name); failures++; } \
} while (0)

@interface Delegate : NSObject <UNUserNotificationCenterDelegate>
@property (assign) int calls;
@property (assign) UNNotificationPresentationOptions answer;
@property (copy) NSString *lastIdentifier;
@end

@implementation Delegate
- (void) userNotificationCenter: (UNUserNotificationCenter *) center
        willPresentNotification: (UNNotification *) notification
          withCompletionHandler: (void (^)(UNNotificationPresentationOptions)) completionHandler
{
	self.calls++;
	self.lastIdentifier = notification.request.identifier;
	completionHandler(self.answer);
}
@end

static void spin(NSTimeInterval seconds)
{
	[[NSRunLoop currentRunLoop] runUntilDate: [NSDate dateWithTimeIntervalSinceNow: seconds]];
}

static void post(UNUserNotificationCenter *center, NSString *identifier)
{
	UNMutableNotificationContent *content = [[UNMutableNotificationContent alloc] init];
	content.title = @"willPresent test";
	content.body = identifier;
	UNNotificationRequest *request = [UNNotificationRequest requestWithIdentifier: identifier content: content trigger: nil];
	__block BOOL done = NO;
	[center addNotificationRequest: request withCompletionHandler: ^(NSError *error) { done = YES; }];
	for (int i = 0; i < 50 && !done; i++)
		spin(0.1);
}

static BOOL delivered(UNUserNotificationCenter *center, NSString *identifier)
{
	__block BOOL done = NO, found = NO;
	[center getDeliveredNotificationsWithCompletionHandler: ^(NSArray<UNNotification *> *list) {
		for (UNNotification *n in list)
			if ([n.request.identifier isEqualToString: identifier])
				found = YES;
		done = YES;
	}];
	for (int i = 0; i < 50 && !done; i++)
		spin(0.1);
	return found;
}

int main(void)
{
	@autoreleasepool {
		UNUserNotificationCenter *center = [UNUserNotificationCenter currentNotificationCenter];
		Delegate *delegate = [[Delegate alloc] init];
		center.delegate = delegate;

		__block BOOL authorized = NO, asked = NO;
		[center requestAuthorizationWithOptions: UNAuthorizationOptionAlert
			completionHandler: ^(BOOL granted, NSError *error) { authorized = granted; asked = YES; }];
		for (int i = 0; i < 50 && !asked; i++)
			spin(0.1);
		if (!authorized) {
			printf("SKIP: notifications are not authorized for this process\n");
			return 0;
		}
		[center removeAllDeliveredNotifications];

		// 1. not the active application
		delegate.answer = UNNotificationPresentationOptionNone;
		post(center, @"wp-background");
		spin(1.0);
		CHECK(delegate.calls == 0, "willPresent is not called when the app is not in the foreground");
		CHECK(delivered(center, @"wp-background"), "the notification of a background app is delivered");

		// 2. the active application
		[NSApplication sharedApplication];
		[NSApp activateIgnoringOtherApps: YES];
		for (int i = 0; i < 30 && ![NSApp isActive]; i++)
			spin(0.1);
		if (![NSApp isActive]) {
			printf("SKIP: this session does not make the process the active application\n");
		} else {
			delegate.answer = UNNotificationPresentationOptionNone;
			post(center, @"wp-none");
			spin(1.0);
			CHECK(delegate.calls == 1 && [delegate.lastIdentifier isEqualToString: @"wp-none"],
				"willPresent is called with the notification when the app is in the foreground");
			CHECK(!delivered(center, @"wp-none"), "options None: the notification is not delivered");

			delegate.answer = UNNotificationPresentationOptionBanner;
			post(center, @"wp-banner");
			spin(1.0);
			CHECK(delegate.calls == 2, "willPresent is called for the second notification");
			CHECK(delivered(center, @"wp-banner"), "options Banner: the notification is delivered");
		}

		[center removeAllDeliveredNotifications];
	}
	printf(failures ? "FAILED %d\n" : "ALL PASSED\n", failures);
	return failures ? 1 : 0;
}
