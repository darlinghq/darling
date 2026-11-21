// CFLAGS: -framework UserNotifications -framework Foundation
// Copyright (C) 2026 Lubos Dolezel
//
// UNNotificationAttachmentOptionsTypeHintKey is the string "typeHint". Observed on macOS 26.7.3;
// passes unchanged there.
#import <UserNotifications/UserNotifications.h>
#include <stdio.h>

int main(void)
{
	int ok = [UNNotificationAttachmentOptionsTypeHintKey isEqualToString:@"typeHint"];
	printf(ok ? "PASS\n" : "FAIL: UNNotificationAttachmentOptionsTypeHintKey\n");
	return !ok;
}
