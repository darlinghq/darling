// CFLAGS: -framework Foundation -framework UserNotifications
//
// Tests for UserNotifications and NSUserNotification delivery structures,
// triggers, content, requests, responses, settings, and secure coding.

#import <Foundation/Foundation.h>
#import <UserNotifications/UserNotifications.h>
#include <stdio.h>
#include <stdlib.h>

static int failures;

#define CHECK(cond, name) do { \
	if (cond) { \
		printf("PASS: %s\n", name); \
	} else { \
		printf("FAIL: %s\n", name); \
		failures++; \
	} \
} while (0)

int main(void)
{
	@autoreleasepool {
		/* 1. UNNotificationSound */
		UNNotificationSound *defSound = [UNNotificationSound defaultSound];
		CHECK(defSound != nil, "defaultSound is non-nil");

		UNNotificationSound *customSound = [UNNotificationSound soundNamed: @"alert.wav"];
		CHECK(customSound != nil, "soundNamed is non-nil");

		NSData *soundData = [NSKeyedArchiver archivedDataWithRootObject: customSound
			requiringSecureCoding: YES error: NULL];
		UNNotificationSound *decSound = soundData ? [NSKeyedUnarchiver unarchivedObjectOfClass: [UNNotificationSound class]
			fromData: soundData error: NULL] : nil;
		CHECK(decSound != nil, "UNNotificationSound secure coding");

		/* 2. UNTimeIntervalNotificationTrigger */
		UNTimeIntervalNotificationTrigger *tiTrigger = [UNTimeIntervalNotificationTrigger
			triggerWithTimeInterval: 120.0 repeats: NO];
		CHECK(tiTrigger != nil && tiTrigger.timeInterval == 120.0 && !tiTrigger.repeats,
			"UNTimeIntervalNotificationTrigger properties");
		CHECK(tiTrigger.nextTriggerDate != nil, "UNTimeIntervalNotificationTrigger nextTriggerDate");

		NSData *tiData = [NSKeyedArchiver archivedDataWithRootObject: tiTrigger
			requiringSecureCoding: YES error: NULL];
		UNTimeIntervalNotificationTrigger *decTi = tiData ? [NSKeyedUnarchiver unarchivedObjectOfClass: [UNTimeIntervalNotificationTrigger class]
			fromData: tiData error: NULL] : nil;
		CHECK(decTi != nil && decTi.timeInterval == 120.0 && !decTi.repeats,
			"UNTimeIntervalNotificationTrigger secure coding");

		/* 3. UNCalendarNotificationTrigger */
		NSDateComponents *comps = [[NSDateComponents alloc] init];
		comps.hour = 9;
		comps.minute = 15;
		UNCalendarNotificationTrigger *calTrigger = [UNCalendarNotificationTrigger
			triggerWithDateMatchingComponents: comps repeats: YES];
		CHECK(calTrigger != nil && calTrigger.repeats, "UNCalendarNotificationTrigger created");
		CHECK(calTrigger.dateComponents.hour == 9 && calTrigger.dateComponents.minute == 15,
			"UNCalendarNotificationTrigger dateComponents");
		CHECK(calTrigger.nextTriggerDate != nil, "UNCalendarNotificationTrigger nextTriggerDate");

		NSData *calData = [NSKeyedArchiver archivedDataWithRootObject: calTrigger
			requiringSecureCoding: YES error: NULL];
		UNCalendarNotificationTrigger *decCal = calData ? [NSKeyedUnarchiver unarchivedObjectOfClass: [UNCalendarNotificationTrigger class]
			fromData: calData error: NULL] : nil;
		CHECK(decCal != nil && decCal.dateComponents.hour == 9 && decCal.repeats,
			"UNCalendarNotificationTrigger secure coding");
		[comps release];

		/* 4. UNMutableNotificationContent & UNNotificationContent */
		UNMutableNotificationContent *mutContent = [[UNMutableNotificationContent alloc] init];
		mutContent.title = @"Reminder";
		mutContent.subtitle = @"Important";
		mutContent.body = @"Don't forget to push";
		mutContent.badge = @(3);
		mutContent.sound = defSound;
		mutContent.categoryIdentifier = @"MESSAGE";
		mutContent.userInfo = @{ @"key" : @"val" };

		UNNotificationContent *immContent = [mutContent copy];
		CHECK([immContent.title isEqualToString: @"Reminder"], "content title");
		CHECK([immContent.subtitle isEqualToString: @"Important"], "content subtitle");
		CHECK([immContent.body isEqualToString: @"Don't forget to push"], "content body");
		CHECK([immContent.badge isEqualToNumber: @(3)], "content badge");
		CHECK([immContent.categoryIdentifier isEqualToString: @"MESSAGE"], "content categoryIdentifier");
		CHECK([immContent.userInfo[@"key"] isEqualToString: @"val"], "content userInfo");

		NSData *contentData = [NSKeyedArchiver archivedDataWithRootObject: immContent
			requiringSecureCoding: YES error: NULL];
		UNNotificationContent *decContent = contentData ? [NSKeyedUnarchiver unarchivedObjectOfClass: [UNNotificationContent class]
			fromData: contentData error: NULL] : nil;
		CHECK(decContent != nil && [decContent.title isEqualToString: @"Reminder"]
			&& [decContent.body isEqualToString: @"Don't forget to push"],
			"UNNotificationContent secure coding");

		/* 5. UNNotificationRequest */
		UNNotificationRequest *req = [UNNotificationRequest requestWithIdentifier: @"req-42"
			content: immContent trigger: tiTrigger];
		CHECK([req.identifier isEqualToString: @"req-42"], "request identifier");
		CHECK(req.content == immContent, "request content");
		CHECK(req.trigger == tiTrigger, "request trigger");

		NSData *reqData = [NSKeyedArchiver archivedDataWithRootObject: req
			requiringSecureCoding: YES error: NULL];
		UNNotificationRequest *decReq = reqData ? [NSKeyedUnarchiver unarchivedObjectOfClass: [UNNotificationRequest class]
			fromData: reqData error: NULL] : nil;
		CHECK(decReq != nil && [decReq.identifier isEqualToString: @"req-42"],
			"UNNotificationRequest secure coding");

		/* 6. UNNotificationSettings */
		UNNotificationSettings *settings = [[UNNotificationSettings alloc] init];
		CHECK(settings.authorizationStatus == UNAuthorizationStatusAuthorized, "settings authorizationStatus");
		CHECK(settings.alertSetting == UNNotificationSettingEnabled, "settings alertSetting");
		CHECK(settings.soundSetting == UNNotificationSettingEnabled, "settings soundSetting");
		CHECK(settings.badgeSetting == UNNotificationSettingEnabled, "settings badgeSetting");

		NSData *setData = [NSKeyedArchiver archivedDataWithRootObject: settings
			requiringSecureCoding: YES error: NULL];
		UNNotificationSettings *decSet = setData ? [NSKeyedUnarchiver unarchivedObjectOfClass: [UNNotificationSettings class]
			fromData: setData error: NULL] : nil;
		CHECK(decSet != nil && decSet.authorizationStatus == UNAuthorizationStatusAuthorized,
			"UNNotificationSettings secure coding");
		[settings release];

		/* 7. UNNotificationAction & UNTextInputNotificationAction */
		UNTextInputNotificationAction *textAction = [UNTextInputNotificationAction
			actionWithIdentifier: @"reply"
			title: @"Reply"
			options: UNNotificationActionOptionForeground
			textInputButtonTitle: @"Send"
			textInputPlaceholder: @"Type message..."];
		CHECK([textAction.identifier isEqualToString: @"reply"], "text action identifier");
		CHECK([textAction.textInputButtonTitle isEqualToString: @"Send"], "text action button title");
		CHECK([textAction.textInputPlaceholder isEqualToString: @"Type message..."], "text action placeholder");

		NSData *actData = [NSKeyedArchiver archivedDataWithRootObject: textAction
			requiringSecureCoding: YES error: NULL];
		UNTextInputNotificationAction *decAct = actData ? [NSKeyedUnarchiver unarchivedObjectOfClass: [UNTextInputNotificationAction class]
			fromData: actData error: NULL] : nil;
		CHECK(decAct != nil && [decAct.textInputButtonTitle isEqualToString: @"Send"],
			"UNTextInputNotificationAction secure coding");

		/* 8. NSUserNotification & NSUserNotificationAction */
		NSUserNotification *nsNotif = [[NSUserNotification alloc] init];
		nsNotif.identifier = @"ns-1";
		nsNotif.title = @"Mail";
		nsNotif.subtitle = @"New message";
		nsNotif.informativeText = @"From: developer@darlinghq.org";
		nsNotif.hasActionButton = YES;
		nsNotif.actionButtonTitle = @"Reply";
		nsNotif.otherButtonTitle = @"Close";
		nsNotif.soundName = NSUserNotificationDefaultSoundName;

		NSUserNotificationAction *act1 = [NSUserNotificationAction actionWithIdentifier: @"archive" title: @"Archive"];
		CHECK([act1.identifier isEqualToString: @"archive"], "NSUserNotificationAction identifier");
		CHECK([act1.title isEqualToString: @"Archive"], "NSUserNotificationAction title");
		nsNotif.additionalActions = @[act1];

		CHECK([nsNotif.identifier isEqualToString: @"ns-1"], "nsNotif identifier");
		CHECK([nsNotif.title isEqualToString: @"Mail"], "nsNotif title");
		CHECK([nsNotif.subtitle isEqualToString: @"New message"], "nsNotif subtitle");
		CHECK([nsNotif.informativeText isEqualToString: @"From: developer@darlinghq.org"], "nsNotif informativeText");
		CHECK(nsNotif.hasActionButton == YES, "nsNotif hasActionButton");
		CHECK([nsNotif.actionButtonTitle isEqualToString: @"Reply"], "nsNotif actionButtonTitle");
		CHECK([nsNotif.otherButtonTitle isEqualToString: @"Close"], "nsNotif otherButtonTitle");
		CHECK(nsNotif.additionalActions.count == 1, "nsNotif additionalActions");

		NSData *nsData = [NSKeyedArchiver archivedDataWithRootObject: nsNotif
			requiringSecureCoding: YES error: NULL];
		NSUserNotification *decNs = nsData ? [NSKeyedUnarchiver unarchivedObjectOfClass: [NSUserNotification class]
			fromData: nsData error: NULL] : nil;
		CHECK(decNs != nil && [decNs.identifier isEqualToString: @"ns-1"]
			&& [decNs.title isEqualToString: @"Mail"]
			&& [decNs.informativeText isEqualToString: @"From: developer@darlinghq.org"],
			"NSUserNotification secure coding");

		/* 9. NSUserNotificationCenter */
		NSUserNotificationCenter *nsCenter = [NSUserNotificationCenter defaultUserNotificationCenter];
		CHECK(nsCenter != nil, "NSUserNotificationCenter defaultUserNotificationCenter");
		[nsCenter deliverNotification: nsNotif];
		[nsCenter removeDeliveredNotification: nsNotif];
		[nsCenter removeAllDeliveredNotifications];

		[immContent release];
		[mutContent release];
		[req release];
		[nsNotif release];
	}

	printf(failures ? "FAILED %d\n" : "ALL PASSED\n", failures);
	return failures ? 1 : 0;
}
