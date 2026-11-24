// CFLAGS: -framework Foundation -framework CoreWLAN
// Copyright (C) 2026 Lubos Dolezel

// The string constants CoreWLAN exports have the values they have on macOS 26.7.3 and 10.14.4.

#import <Foundation/Foundation.h>
#import <CoreWLAN/CoreWLAN.h>
#include <stdio.h>

static int failures = 0;
#define expect(cond) do { int ok_ = (cond) ? 1 : 0; printf("%s: %s\n", ok_ ? "PASS" : "FAIL", #cond); if (!ok_) failures++; } while (0)

int main(void)
{
	@autoreleasepool {
		expect([CWErrorDomain isEqualToString:@"com.apple.coreWLAN.error"]);
		expect([kCWAssocKey8021XProfile isEqualToString:@"ASSOC_KEY_8021X_PROFILE"]);
		expect([kCWAssocKeyPassphrase isEqualToString:@"ASSOC_KEY_PASSPHRASE"]);
		expect([kCWBSSIDDidChangeNotification isEqualToString:@"com.apple.coreWLAN.notification.bssid.legacy"]);
		expect([kCWCountryCodeDidChangeNotification isEqualToString:@"com.apple.coreWLAN.notification.countryCode.legacy"]);
		expect([kCWErrorDomain isEqualToString:@"com.apple.coreWLAN.error"]);
		expect([kCWIBSSKeyChannel isEqualToString:@"IBSS_KEY_CHANNEL"]);
		expect([kCWIBSSKeyPassphrase isEqualToString:@"IBSS_KEY_PASSPHRASE"]);
		expect([kCWIBSSKeySSID isEqualToString:@"IBSS_KEY_SSID"]);
		expect([kCWLinkDidChangeNotification isEqualToString:@"com.apple.coreWLAN.notification.link.legacy"]);
		expect([kCWModeDidChangeNotification isEqualToString:@"com.apple.coreWLAN.notification.mode.legacy"]);
		expect([kCWPowerDidChangeNotification isEqualToString:@"com.apple.coreWLAN.notification.power.legacy"]);
		expect([kCWScanKeyBSSID isEqualToString:@"BSSID"]);
		expect([kCWScanKeyDwellTime isEqualToString:@"SCAN_DWELL_TIME"]);
		expect([kCWScanKeyMerge isEqualToString:@"SCAN_MERGE"]);
		expect([kCWScanKeyRestTime isEqualToString:@"SCAN_REST_TIME"]);
		expect([kCWScanKeyScanType isEqualToString:@"SCAN_TYPE"]);
		expect([kCWScanKeySSID isEqualToString:@"SSID_STR"]);
		expect([kCWSSIDDidChangeNotification isEqualToString:@"com.apple.coreWLAN.notification.ssid.legacy"]);
	}
	printf("%s\n", failures ? "FAILED" : "ALL PASSED");
	return failures != 0;
}
