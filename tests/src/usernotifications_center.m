// CFLAGS: -framework Foundation -framework UserNotifications
// Copyright (C) 2026 Lubos Dolezel
//
// UNUserNotificationCenter: the current center is a singleton, the
// authorization, settings and add-request calls run their completion
// handlers, categories set on the center are returned by the getter, and
// UNNotificationAction / UNNotificationCategory keep their properties
// across NSSecureCoding. Prints PASS or FAIL lines, then ALL PASSED or
// FAILED n.
#import <Foundation/Foundation.h>
#import <UserNotifications/UNNotificationAction.h>
#import <UserNotifications/UNNotificationCategory.h>
#import <UserNotifications/UNNotificationRequest.h>
#import <UserNotifications/UNNotificationSettings.h>
#import <UserNotifications/UNUserNotificationCenter.h>
#include <stdio.h>
#include <stdlib.h>

static int failures;
static int pending;

#define CHECK(cond, name) do { \
	if (cond) printf("PASS: %s\n", name); \
	else { printf("FAIL: %s\n", name); failures++; } \
} while (0)

static void finish(void)
{
	if (--pending > 0)
		return;
	printf(failures ? "FAILED %d\n" : "ALL PASSED\n", failures);
	exit(failures ? 1 : 0);
}

int main(void)
{
	@autoreleasepool {
		UNUserNotificationCenter *center = [UNUserNotificationCenter currentNotificationCenter];
		CHECK(center != nil, "currentNotificationCenter is non-nil");
		CHECK(center == [UNUserNotificationCenter currentNotificationCenter],
			"currentNotificationCenter is a singleton");

		UNNotificationAction *action = [UNNotificationAction actionWithIdentifier: @"reply"
			title: @"Reply"
			options: UNNotificationActionOptionForeground | UNNotificationActionOptionDestructive];
		CHECK([action.identifier isEqualToString: @"reply"], "action identifier");
		CHECK([action.title isEqualToString: @"Reply"], "action title");
		CHECK(action.options == (UNNotificationActionOptionForeground | UNNotificationActionOptionDestructive),
			"action options");

		UNNotificationCategory *category = [UNNotificationCategory categoryWithIdentifier: @"msg"
			actions: @[action]
			intentIdentifiers: @[@"intent.one"]
			options: UNNotificationCategoryOptionCustomDismissAction];
		CHECK([category.identifier isEqualToString: @"msg"], "category identifier");
		CHECK(category.actions.count == 1 && category.actions[0] == action, "category actions");
		CHECK([category.intentIdentifiers isEqualToArray: @[@"intent.one"]], "category intent identifiers");
		CHECK(category.options == UNNotificationCategoryOptionCustomDismissAction, "category options");

		NSData *data = [NSKeyedArchiver archivedDataWithRootObject: category
			requiringSecureCoding: YES error: NULL];
		UNNotificationCategory *decoded = data
			? [NSKeyedUnarchiver unarchivedObjectOfClass: [UNNotificationCategory class]
				fromData: data error: NULL]
			: nil;
		CHECK(decoded != nil, "category survives secure coding");
		CHECK([decoded.identifier isEqualToString: @"msg"]
			&& decoded.actions.count == 1
			&& [decoded.actions[0].identifier isEqualToString: @"reply"]
			&& [decoded.actions[0].title isEqualToString: @"Reply"]
			&& decoded.options == UNNotificationCategoryOptionCustomDismissAction,
			"decoded category matches the original");

		pending = 4;
		[center requestAuthorizationWithOptions: UNAuthorizationOptionAlert | UNAuthorizationOptionSound
			completionHandler: ^(BOOL granted, NSError *error) {
				CHECK(granted && error == nil, "authorization completion handler runs");
				finish();
			}];
		[center getNotificationSettingsWithCompletionHandler: ^(UNNotificationSettings *settings) {
			CHECK(settings != nil, "settings completion handler runs");
			finish();
		}];
		[center addNotificationRequest: nil withCompletionHandler: ^(NSError *error) {
			CHECK(error == nil, "add-request completion handler runs");
			finish();
		}];

		[center setNotificationCategories: [NSSet setWithObject: category]];
		[center getNotificationCategoriesWithCompletionHandler: ^(NSSet *categories) {
			CHECK(categories.count == 1 && [categories containsObject: category],
				"categories getter returns the categories that were set");
			[center setNotificationCategories: [NSSet set]];
			[center getNotificationCategoriesWithCompletionHandler: ^(NSSet *cleared) {
				CHECK(cleared != nil && cleared.count == 0, "setting an empty set clears the categories");
				finish();
			}];
		}];
	}
	dispatch_main();
}
