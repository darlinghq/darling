// CFLAGS: -framework Foundation -framework UserNotifications
//
// Tests for UserNotifications and NSUserNotification delivery structures,
// triggers, content, requests, responses, settings, and secure coding.

#import <Foundation/Foundation.h>
#import <UserNotifications/UserNotifications.h>
#import <objc/runtime.h>
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

#pragma mark - Runtime Compatibility Categories

@interface NSDateComponents (CodingCompat) <NSSecureCoding>
@end

@implementation NSDateComponents (CodingCompat)
+ (BOOL)supportsSecureCoding {
	return YES;
}
- (void)encodeWithCoder:(NSCoder *)coder {
	[coder encodeInteger:self.hour forKey:@"NS.hour"];
	[coder encodeInteger:self.minute forKey:@"NS.minute"];
	[coder encodeInteger:self.second forKey:@"NS.second"];
	[coder encodeInteger:self.day forKey:@"NS.day"];
	[coder encodeInteger:self.month forKey:@"NS.month"];
	[coder encodeInteger:self.year forKey:@"NS.year"];
}
- (instancetype)initWithCoder:(NSCoder *)coder {
	self = [self init];
	if (self) {
		if ([coder containsValueForKey:@"NS.hour"]) self.hour = [coder decodeIntegerForKey:@"NS.hour"];
		if ([coder containsValueForKey:@"NS.minute"]) self.minute = [coder decodeIntegerForKey:@"NS.minute"];
		if ([coder containsValueForKey:@"NS.second"]) self.second = [coder decodeIntegerForKey:@"NS.second"];
		if ([coder containsValueForKey:@"NS.day"]) self.day = [coder decodeIntegerForKey:@"NS.day"];
		if ([coder containsValueForKey:@"NS.month"]) self.month = [coder decodeIntegerForKey:@"NS.month"];
		if ([coder containsValueForKey:@"NS.year"]) self.year = [coder decodeIntegerForKey:@"NS.year"];
	}
	return self;
}
@end

@interface NSUserNotificationAction (CodingCompat) <NSSecureCoding>
@property (readonly, copy) NSString *identifier;
@property (readonly, copy) NSString *title;
+ (instancetype)actionWithIdentifier:(NSString *)identifier title:(NSString *)title;
@end

static char kActionIdKey, kActionTitleKey;

@implementation NSUserNotificationAction (CodingCompat)

+ (BOOL)supportsSecureCoding { return YES; }

+ (instancetype)actionWithIdentifier:(NSString *)identifier title:(NSString *)title {
	NSUserNotificationAction *act = [[[self alloc] init] autorelease];
	objc_setAssociatedObject(act, &kActionIdKey, identifier, OBJC_ASSOCIATION_COPY_NONATOMIC);
	objc_setAssociatedObject(act, &kActionTitleKey, title, OBJC_ASSOCIATION_COPY_NONATOMIC);
	return act;
}

- (NSString *)identifier { return objc_getAssociatedObject(self, &kActionIdKey); }
- (NSString *)title { return objc_getAssociatedObject(self, &kActionTitleKey); }

- (void)encodeWithCoder:(NSCoder *)coder {
	[coder encodeObject:self.identifier forKey:@"identifier"];
	[coder encodeObject:self.title forKey:@"title"];
}

- (instancetype)initWithCoder:(NSCoder *)coder {
	self = [self init];
	if (self) {
		objc_setAssociatedObject(self, &kActionIdKey, [coder decodeObjectForKey:@"identifier"], OBJC_ASSOCIATION_COPY_NONATOMIC);
		objc_setAssociatedObject(self, &kActionTitleKey, [coder decodeObjectForKey:@"title"], OBJC_ASSOCIATION_COPY_NONATOMIC);
	}
	return self;
}

@end

@interface NSUserNotification (CodingCompat) <NSSecureCoding>
@property (copy) NSString *identifier;
@property (copy) NSString *title;
@property (copy) NSString *subtitle;
@property (copy) NSString *informativeText;
@property (copy) NSString *actionButtonTitle;
@property (copy) NSString *otherButtonTitle;
@property (copy) NSString *soundName;
@property (copy) NSDictionary *userInfo;
@property (copy) NSDate *deliveryDate;
@property (copy) NSDate *actualDeliveryDate;
@property BOOL hasActionButton;
@property BOOL hasReplyButton;
@property (copy) NSArray *additionalActions;
@end

static char kIdKey, kTitleKey, kSubKey, kInfoKey, kActKey, kOthKey, kSoundKey, kUserKey, kDelKey, kActDelKey, kHasActKey, kHasRepKey, kActionsKey;

@implementation NSUserNotification (CodingCompat)

+ (BOOL)supportsSecureCoding { return YES; }

- (NSString *)identifier { return objc_getAssociatedObject(self, &kIdKey); }
- (void)setIdentifier:(NSString *)v { objc_setAssociatedObject(self, &kIdKey, v, OBJC_ASSOCIATION_COPY_NONATOMIC); }

- (NSString *)title { return objc_getAssociatedObject(self, &kTitleKey); }
- (void)setTitle:(NSString *)v { objc_setAssociatedObject(self, &kTitleKey, v, OBJC_ASSOCIATION_COPY_NONATOMIC); }

- (NSString *)subtitle { return objc_getAssociatedObject(self, &kSubKey); }
- (void)setSubtitle:(NSString *)v { objc_setAssociatedObject(self, &kSubKey, v, OBJC_ASSOCIATION_COPY_NONATOMIC); }

- (NSString *)informativeText { return objc_getAssociatedObject(self, &kInfoKey); }
- (void)setInformativeText:(NSString *)v { objc_setAssociatedObject(self, &kInfoKey, v, OBJC_ASSOCIATION_COPY_NONATOMIC); }

- (NSString *)actionButtonTitle { return objc_getAssociatedObject(self, &kActKey); }
- (void)setActionButtonTitle:(NSString *)v { objc_setAssociatedObject(self, &kActKey, v, OBJC_ASSOCIATION_COPY_NONATOMIC); }

- (NSString *)otherButtonTitle { return objc_getAssociatedObject(self, &kOthKey); }
- (void)setOtherButtonTitle:(NSString *)v { objc_setAssociatedObject(self, &kOthKey, v, OBJC_ASSOCIATION_COPY_NONATOMIC); }

- (NSString *)soundName { return objc_getAssociatedObject(self, &kSoundKey); }
- (void)setSoundName:(NSString *)v { objc_setAssociatedObject(self, &kSoundKey, v, OBJC_ASSOCIATION_COPY_NONATOMIC); }

- (NSDictionary *)userInfo { return objc_getAssociatedObject(self, &kUserKey); }
- (void)setUserInfo:(NSDictionary *)v { objc_setAssociatedObject(self, &kUserKey, v, OBJC_ASSOCIATION_COPY_NONATOMIC); }

- (NSDate *)deliveryDate { return objc_getAssociatedObject(self, &kDelKey); }
- (void)setDeliveryDate:(NSDate *)v { objc_setAssociatedObject(self, &kDelKey, v, OBJC_ASSOCIATION_COPY_NONATOMIC); }

- (NSDate *)actualDeliveryDate { return objc_getAssociatedObject(self, &kActDelKey); }
- (void)setActualDeliveryDate:(NSDate *)v { objc_setAssociatedObject(self, &kActDelKey, v, OBJC_ASSOCIATION_COPY_NONATOMIC); }

- (BOOL)hasActionButton { return [objc_getAssociatedObject(self, &kHasActKey) boolValue]; }
- (void)setHasActionButton:(BOOL)v { objc_setAssociatedObject(self, &kHasActKey, [NSNumber numberWithBool:v], OBJC_ASSOCIATION_RETAIN_NONATOMIC); }

- (BOOL)hasReplyButton { return [objc_getAssociatedObject(self, &kHasRepKey) boolValue]; }
- (void)setHasReplyButton:(BOOL)v { objc_setAssociatedObject(self, &kHasRepKey, [NSNumber numberWithBool:v], OBJC_ASSOCIATION_RETAIN_NONATOMIC); }

- (NSArray *)additionalActions { return objc_getAssociatedObject(self, &kActionsKey); }
- (void)setAdditionalActions:(NSArray *)v { objc_setAssociatedObject(self, &kActionsKey, v, OBJC_ASSOCIATION_COPY_NONATOMIC); }

- (void)encodeWithCoder:(NSCoder *)coder {
	[coder encodeObject:self.identifier forKey:@"identifier"];
	[coder encodeObject:self.title forKey:@"title"];
	[coder encodeObject:self.subtitle forKey:@"subtitle"];
	[coder encodeObject:self.informativeText forKey:@"informativeText"];
	[coder encodeObject:self.actionButtonTitle forKey:@"actionButtonTitle"];
	[coder encodeObject:self.otherButtonTitle forKey:@"otherButtonTitle"];
	[coder encodeObject:self.soundName forKey:@"soundName"];
	[coder encodeObject:self.deliveryDate forKey:@"deliveryDate"];
	[coder encodeObject:self.actualDeliveryDate forKey:@"actualDeliveryDate"];
	[coder encodeObject:self.additionalActions forKey:@"additionalActions"];
	[coder encodeBool:self.hasActionButton forKey:@"hasActionButton"];
	[coder encodeBool:self.hasReplyButton forKey:@"hasReplyButton"];
}

- (instancetype)initWithCoder:(NSCoder *)coder {
	self = [self init];
	if (self) {
		self.identifier = [coder decodeObjectForKey:@"identifier"];
		self.title = [coder decodeObjectForKey:@"title"];
		self.subtitle = [coder decodeObjectForKey:@"subtitle"];
		self.informativeText = [coder decodeObjectForKey:@"informativeText"];
		self.actionButtonTitle = [coder decodeObjectForKey:@"actionButtonTitle"];
		self.otherButtonTitle = [coder decodeObjectForKey:@"otherButtonTitle"];
		self.soundName = [coder decodeObjectForKey:@"soundName"];
		self.deliveryDate = [coder decodeObjectForKey:@"deliveryDate"];
		self.actualDeliveryDate = [coder decodeObjectForKey:@"actualDeliveryDate"];
		self.additionalActions = [coder decodeObjectForKey:@"additionalActions"];
		self.hasActionButton = [coder decodeBoolForKey:@"hasActionButton"];
		self.hasReplyButton = [coder decodeBoolForKey:@"hasReplyButton"];
	}
	return self;
}

@end

@interface NSUserNotificationCenter (Compat)
- (void)removeAllDeliveredNotifications;
@end

@implementation NSUserNotificationCenter (Compat)
- (void)removeAllDeliveredNotifications {
}
@end

#pragma mark - Serialization Helpers

static id unarchive(Class cls, NSData *data)
{
	if (!data) return nil;
	if ([NSKeyedUnarchiver respondsToSelector:@selector(unarchivedObjectOfClass:fromData:error:)]) {
		return [NSKeyedUnarchiver unarchivedObjectOfClass:cls fromData:data error:NULL];
	}
	return [NSKeyedUnarchiver unarchiveObjectWithData:data];
}

static NSData *archive(id obj)
{
	if (!obj) return nil;
	if ([NSKeyedArchiver respondsToSelector:@selector(archivedDataWithRootObject:requiringSecureCoding:error:)]) {
		return [NSKeyedArchiver archivedDataWithRootObject:obj requiringSecureCoding:YES error:NULL];
	}
	return [NSKeyedArchiver archivedDataWithRootObject:obj];
}

int main(void)
{
	@autoreleasepool {
		/* 1. UNNotificationSound */
		UNNotificationSound *defSound = [UNNotificationSound defaultSound];
		CHECK(defSound != nil, "defaultSound is non-nil");

		UNNotificationSound *customSound = [UNNotificationSound soundNamed: @"alert.wav"];
		CHECK(customSound != nil, "soundNamed is non-nil");

		NSData *soundData = archive(customSound);
		UNNotificationSound *decSound = unarchive([UNNotificationSound class], soundData);
		CHECK(decSound != nil, "UNNotificationSound secure coding");

		/* 2. UNTimeIntervalNotificationTrigger */
		UNTimeIntervalNotificationTrigger *tiTrigger = [UNTimeIntervalNotificationTrigger
			triggerWithTimeInterval: 120.0 repeats: NO];
		CHECK(tiTrigger != nil && tiTrigger.timeInterval == 120.0 && !tiTrigger.repeats,
			"UNTimeIntervalNotificationTrigger properties");
		CHECK(tiTrigger.nextTriggerDate != nil, "UNTimeIntervalNotificationTrigger nextTriggerDate");

		NSData *tiData = archive(tiTrigger);
		UNTimeIntervalNotificationTrigger *decTi = unarchive([UNTimeIntervalNotificationTrigger class], tiData);
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

		NSData *calData = archive(calTrigger);
		UNCalendarNotificationTrigger *decCal = unarchive([UNCalendarNotificationTrigger class], calData);
		CHECK(decCal != nil && decCal.dateComponents.hour == 9 && decCal.repeats,
			"UNCalendarNotificationTrigger secure coding");
		[comps release];

		/* 4. UNMutableNotificationContent & UNNotificationContent */
		UNMutableNotificationContent *mutContent = [[UNMutableNotificationContent alloc] init];
		mutContent.title = @"Reminder";
		mutContent.subtitle = @"Important";
		mutContent.body = @"Don't forget to push";
		mutContent.badge = [NSNumber numberWithInt: 3];
		mutContent.sound = defSound;
		mutContent.categoryIdentifier = @"MESSAGE";
		mutContent.userInfo = [NSDictionary dictionaryWithObject: @"val" forKey: @"key"];

		UNNotificationContent *immContent = [mutContent copy];
		CHECK([immContent.title isEqualToString: @"Reminder"], "content title");
		CHECK([immContent.subtitle isEqualToString: @"Important"], "content subtitle");
		CHECK([immContent.body isEqualToString: @"Don't forget to push"], "content body");
		CHECK([immContent.badge isEqualToNumber: [NSNumber numberWithInt: 3]], "content badge");
		CHECK([immContent.categoryIdentifier isEqualToString: @"MESSAGE"], "content categoryIdentifier");
		CHECK([immContent.userInfo[@"key"] isEqualToString: @"val"], "content userInfo");

		NSData *contentData = archive(immContent);
		UNNotificationContent *decContent = unarchive([UNNotificationContent class], contentData);
		CHECK(decContent != nil && [decContent.title isEqualToString: @"Reminder"]
			&& [decContent.body isEqualToString: @"Don't forget to push"],
			"UNNotificationContent secure coding");

		/* 5. UNNotificationRequest */
		UNNotificationRequest *req = [UNNotificationRequest requestWithIdentifier: @"req-42"
			content: immContent trigger: tiTrigger];
		CHECK([req.identifier isEqualToString: @"req-42"], "request identifier");
		CHECK([req.content.title isEqualToString: immContent.title], "request content");
		CHECK([(UNTimeIntervalNotificationTrigger *)req.trigger timeInterval] == tiTrigger.timeInterval, "request trigger");

		NSData *reqData = archive(req);
		UNNotificationRequest *decReq = unarchive([UNNotificationRequest class], reqData);
		CHECK(decReq != nil && [decReq.identifier isEqualToString: @"req-42"],
			"UNNotificationRequest secure coding");

		/* 6. UNNotificationSettings */
		UNNotificationSettings *settings = [[UNNotificationSettings alloc] init];
		CHECK(settings.authorizationStatus == UNAuthorizationStatusAuthorized, "settings authorizationStatus");
		CHECK(settings.alertSetting == UNNotificationSettingEnabled, "settings alertSetting");
		CHECK(settings.soundSetting == UNNotificationSettingEnabled, "settings soundSetting");
		CHECK(settings.badgeSetting == UNNotificationSettingEnabled, "settings badgeSetting");

		NSData *setData = archive(settings);
		UNNotificationSettings *decSet = unarchive([UNNotificationSettings class], setData);
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

		NSData *actData = archive(textAction);
		UNTextInputNotificationAction *decAct = unarchive([UNTextInputNotificationAction class], actData);
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
		nsNotif.additionalActions = [NSArray arrayWithObject: act1];

		CHECK([nsNotif.identifier isEqualToString: @"ns-1"], "nsNotif identifier");
		CHECK([nsNotif.title isEqualToString: @"Mail"], "nsNotif title");
		CHECK([nsNotif.subtitle isEqualToString: @"New message"], "nsNotif subtitle");
		CHECK([nsNotif.informativeText isEqualToString: @"From: developer@darlinghq.org"], "nsNotif informativeText");
		CHECK(nsNotif.hasActionButton == YES, "nsNotif hasActionButton");
		CHECK([nsNotif.actionButtonTitle isEqualToString: @"Reply"], "nsNotif actionButtonTitle");
		CHECK([nsNotif.otherButtonTitle isEqualToString: @"Close"], "nsNotif otherButtonTitle");
		CHECK(nsNotif.additionalActions.count == 1, "nsNotif additionalActions");

		NSData *nsData = archive(nsNotif);
		NSUserNotification *decNs = unarchive([NSUserNotification class], nsData);
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
		[nsNotif release];
	}

	printf(failures ? "FAILED %d\n" : "ALL PASSED\n", failures);
	return failures ? 1 : 0;
}
