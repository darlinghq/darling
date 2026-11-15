// Copyright (C) 2026 Lubos Dolezel
//
// GetScriptManagerVariable (the selectors with a fixed answer, the region code range and the KCHR
// cache block when the system has a keyboard layout) and UpdateSystemActivity.
// The expected values were observed on macOS 10.14.4 (i386 and x86_64). Selectors that depend on
// the scripts a system has enabled are only range-checked. Build for x86_64 and i386.
#include <Carbon/Carbon.h>
#include <CoreServices/CoreServices.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#pragma clang diagnostic ignored "-Wdeprecated-declarations"

extern long GetScriptManagerVariable(short selector);
extern OSErr UpdateSystemActivity(UInt8 activity);

static int g_failures = 0;

#define CHECK(cond) do { if (!(cond)) { printf("FAIL %d: %s\n", __LINE__, #cond); g_failures++; } } while (0)
#define CHECK_VAR(selector, expected) do { long v_ = GetScriptManagerVariable(selector); if (v_ != (long) (expected)) { printf("FAIL %d: selector %d is %ld, expected %ld\n", __LINE__, selector, v_, (long) (expected)); g_failures++; } } while (0)

int main(void)
{
	CHECK_VAR(0, 0x1000);   // smVersion
	CHECK_VAR(2, 0);        // smMunged
	CHECK_VAR(6, 0);        // smBidirect
	CHECK_VAR(8, 0);        // smFontForce
	CHECK_VAR(10, 255);     // smIntlForce
	CHECK_VAR(12, 0);       // smForced
	CHECK_VAR(14, 0);       // smDefault
	CHECK_VAR(16, 0);       // smPrint
	CHECK_VAR(18, 0);       // smSysScript
	CHECK_VAR(20, 0);       // smLastScript
	CHECK_VAR(22, 0);       // smKeyScript
	CHECK_VAR(24, 0);       // smSysRef
	CHECK_VAR(26, 0);       // smKeyCache
	CHECK_VAR(28, 0);       // smKeySwap
	CHECK_VAR(30, 0);       // smGenFlags
	CHECK_VAR(32, 0);       // smOverride
	CHECK_VAR(34, 0);       // smCharPortion
	CHECK_VAR(42, 255);     // smKeyDisableState
	// A selector that does not exist.
	CHECK_VAR(1, 0);
	CHECK_VAR(44, 0);
	CHECK_VAR(100, 0);
	CHECK_VAR(-1, 0);

	CHECK(GetScriptManagerVariable(4) >= 1);   // smEnabled counts at least the Roman script
	long region = GetScriptManagerVariable(40); // smRegionCode
	CHECK(region >= 0 && region <= 100);

	// The KCHR cache is the same block each time, and well formed. There may be none when the
	// system has no keyboard layout.
	const uint8_t* kchr = (const uint8_t*) GetScriptManagerVariable(38);
	if (kchr) {
		CHECK(kchr == (const uint8_t*) GetScriptManagerVariable(38));
		uint16_t version, tables;
		memcpy(&version, kchr, 2);
		memcpy(&tables, kchr + 258, 2);
		CHECK(version == 0);
		CHECK(tables >= 1 && tables <= 256);
		for (int m = 0; m < 256; m++)
			CHECK(kchr[2 + m] < tables);

		// Its characters are those of the current Unicode layout for the keys that make one ASCII
		// character: the unmodified, Shift, Option and Control layers (dead keys are left out).
		TISInputSourceRef source = TISCopyCurrentKeyboardLayoutInputSource();
		CFDataRef layout_data = source ? (CFDataRef) TISGetInputSourceProperty(source, kTISPropertyUnicodeKeyLayoutData) : NULL;
		CHECK(layout_data != NULL);
		if (layout_data) {
			const UCKeyboardLayout* layout = (const UCKeyboardLayout*) CFDataGetBytePtr(layout_data);
			const uint8_t layers[] = { 0x00, 0x02, 0x08, 0x10 };
			int compared = 0;
			for (unsigned l = 0; l < sizeof(layers); l++) {
				for (UInt16 key = 0; key < 128; key++) {
					UInt32 dead = 0;
					UniChar ch[4];
					UniCharCount length = 0;
					OSStatus err = UCKeyTranslate(layout, key, kUCKeyActionDown, layers[l], LMGetKbdType(), kUCKeyTranslateNoDeadKeysMask, &dead, 4, &length, ch);
					if (err != noErr || length != 1 || ch[0] >= 0x80 || ch[0] == 0)
						continue;
					uint8_t value = kchr[260 + (size_t) kchr[2 + layers[l]] * 128 + key];
					if (value == 0)
						continue; // a dead key: the table has no character for it
					if (value != ch[0]) {
						printf("FAIL: key %d, modifiers %02x: KCHR has %02x, layout %04x\n", key, layers[l], value, ch[0]);
						g_failures++;
					}
					compared++;
				}
			}
			CHECK(compared > 100);
		}
		if (source)
			CFRelease(source);
	} else {
		printf("note: no KCHR cache (no keyboard layout)\n");
	}

	// UpdateSystemActivity takes the five activity types and nothing else.
	for (int activity = 0; activity <= 4; activity++)
		CHECK(UpdateSystemActivity((UInt8) activity) == noErr);
	CHECK(UpdateSystemActivity(5) == paramErr);
	CHECK(UpdateSystemActivity(6) == paramErr);
	CHECK(UpdateSystemActivity(255) == paramErr);

	if (g_failures == 0)
		printf("PASS\n");
	else
		printf("%d failure(s)\n", g_failures);
	return g_failures != 0;
}
