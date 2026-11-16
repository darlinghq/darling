// Copyright (C) 2026 Lubos Dolezel
//
// The 'uchr' keyboard layout that TIS hands out for the active keymap of the window system (X11
// XKB or a Wayland compositor's keymap), read through TISCopyCurrentKeyboardLayoutInputSource,
// kTISPropertyUnicodeKeyLayoutData and UCKeyTranslate: the modifier byte selecting the Shift,
// Caps Lock, Option (the keymap's AltGr level) and Control tables, dead keys with deadKeyState,
// kUCKeyTranslateNoDeadKeysMask, and the too-small output buffer.
//
// usage: keyboard_layout_uchr <layout> [x11|wayland]
//   layout: us, intl (us with dead keys), cz or de; the keymap must already be set so, for
//   example with `setxkbmap us -variant intl` on the X server the test connects to.
// Prints PASS, FAIL lines, or SKIP when the display backend is not the one asked for.
#import <AppKit/AppKit.h>
#import <Carbon/Carbon.h>
#include <stdio.h>
#include <string.h>

static int g_failures;
static int g_dump;

#define CHECK(cond) do { if (!(cond)) { printf("FAIL line %d: %s\n", __LINE__, #cond); g_failures++; } } while (0)

static const UCKeyboardLayout* g_layout;

typedef struct {
	OSStatus rc;
	UniCharCount length;
	UniChar chars[8];
	UInt32 state;
} result_t;

static result_t press(int key, int mods, UInt32 state, OptionBits options, UniCharCount max)
{
	result_t r;
	memset(&r, 0, sizeof r);
	r.state = state;
	r.rc = UCKeyTranslate(g_layout, (UInt16) key, kUCKeyActionDown, (UInt32) mods, 40, options, &r.state, max, &r.length, r.chars);
	return r;
}

// Whether the result is exactly the given string of up to three characters.
static int is(result_t r, int count, UniChar a, UniChar b, UniChar c)
{
	UniChar want[3] = { a, b, c };
	if (r.rc != noErr || r.length != (UniCharCount) count)
		return 0;
	return memcmp(r.chars, want, count * sizeof(UniChar)) == 0;
}

enum { Shift = 2, Caps = 4, Option = 8, Control = 16, Command = 1 };
enum { kA = 0, kX = 7, kE = 14, kQuote = 39, kGrave = 50, kSpace = 49, kReturn = 36, kTab = 48, kDelete = 51, kEscape = 53,
	kLeft = 123, kUp = 126, kF1 = 122, kEqual = 24, kY = 16, kZ = 6, k2 = 19, k6 = 22 };

static void common(void)
{
	CHECK(is(press(kA, 0, 0, 0, 8), 1, 'a', 0, 0));
	CHECK(is(press(kA, Shift, 0, 0, 8), 1, 'A', 0, 0));
	CHECK(is(press(kA, Caps, 0, 0, 8), 1, 'A', 0, 0));
	// Command and the right-hand modifier bits pick the same tables as the left-hand ones.
	CHECK(is(press(kA, Command, 0, 0, 8), 1, 'a', 0, 0));
	CHECK(is(press(kA, 0x20, 0, 0, 8), 1, 'A', 0, 0));
	// Control gives control codes for letters.
	CHECK(is(press(kA, Control, 0, 0, 8), 1, 0x01, 0, 0));
	CHECK(is(press(kE, Control, 0, 0, 8), 1, 0x05, 0, 0));
	// Keys that edit or navigate give control codes, as the Mac layouts do.
	CHECK(is(press(kReturn, 0, 0, 0, 8), 1, 0x0D, 0, 0));
	CHECK(is(press(kTab, 0, 0, 0, 8), 1, 0x09, 0, 0));
	CHECK(is(press(kDelete, 0, 0, 0, 8), 1, 0x08, 0, 0));
	CHECK(is(press(kEscape, 0, 0, 0, 8), 1, 0x1B, 0, 0));
	CHECK(is(press(kLeft, 0, 0, 0, 8), 1, 0x1C, 0, 0));
	CHECK(is(press(kUp, 0, 0, 0, 8), 1, 0x1E, 0, 0));
	CHECK(is(press(kF1, 0, 0, 0, 8), 1, 0x10, 0, 0));
	CHECK(is(press(kSpace, 0, 0, 0, 8), 1, ' ', 0, 0));
	// A key code past the table gives no character.
	result_t r = press(200, 0, 0, 0, 8);
	CHECK(r.rc == noErr && r.length == 0);
	// A one-character buffer for a one-character result is enough, a zero-length one is not.
	r = press(kA, 0, 0, 0, 1);
	CHECK(is(r, 1, 'a', 0, 0));
	r = press(kA, 0, 0, 0, 0);
	CHECK(r.rc == kUCOutputBufferTooSmall && r.length == 0);
}

static void us(void)
{
	CHECK(is(press(k2, 0, 0, 0, 8), 1, '2', 0, 0));
	CHECK(is(press(k2, Shift, 0, 0, 8), 1, '@', 0, 0));
	CHECK(is(press(kZ, 0, 0, 0, 8), 1, 'z', 0, 0));
	// No dead keys and no third level on this layout: Option does not change the letter.
	CHECK(is(press(kE, Option, 0, 0, 8), 1, 'e', 0, 0));
	CHECK(is(press(kQuote, 0, 0, 0, 8), 1, '\'', 0, 0));
	result_t r = press(kQuote, 0, 0, 0, 8);
	CHECK(r.state == 0);
}

static void intl(void)
{
	// The ' key is a dead acute: no character, a pending state.
	result_t dead = press(kQuote, 0, 0, 0, 8);
	CHECK(dead.rc == noErr && dead.length == 0 && dead.state != 0);
	// deadKeyState carried into the next call: acute + e = e acute.
	result_t r = press(kE, 0, dead.state, 0, 8);
	CHECK(is(r, 1, 0x00E9, 0, 0));
	// The state handed back after the combination is not a pending dead key: feeding it to the
	// next key gives that key's plain character.
	r = press(kA, 0, r.state, 0, 8);
	CHECK(is(r, 1, 'a', 0, 0));
	// Shift-e: E acute.
	r = press(kE, Shift, dead.state, 0, 8);
	CHECK(is(r, 1, 0x00C9, 0, 0));
	// Acute + Space gives the accent alone.
	r = press(kSpace, 0, dead.state, 0, 8);
	CHECK(is(r, 1, 0x00B4, 0, 0));
	// Acute + a key that does not combine: the accent, then the character.
	r = press(kX, 0, dead.state, 0, 8);
	CHECK(is(r, 2, 0x00B4, 'x', 0));
	// With a one-character buffer the first character is delivered and the error reported.
	r = press(kX, 0, dead.state, 0, 1);
	CHECK(r.rc == kUCOutputBufferTooSmall && r.length == 1 && r.chars[0] == 0x00B4);
	// Two acutes: the first accent comes out and a new dead state starts.
	r = press(kQuote, 0, dead.state, 0, 8);
	CHECK(is(r, 1, 0x00B4, 0, 0) && r.state != 0);
	// Shift-' is a different dead key (diaeresis): its state differs and it combines with e.
	result_t dead2 = press(kQuote, Shift, 0, 0, 8);
	CHECK(dead2.length == 0 && dead2.state != 0 && dead2.state != dead.state);
	r = press(kE, 0, dead2.state, 0, 8);
	CHECK(is(r, 1, 0x00EB, 0, 0));
	// Grave and circumflex and tilde dead keys.
	result_t grave = press(kGrave, 0, 0, 0, 8);
	r = press(kE, 0, grave.state, 0, 8);
	CHECK(grave.length == 0 && is(r, 1, 0x00E8, 0, 0));
	result_t circ = press(k6, Shift, 0, 0, 8);
	r = press(kE, 0, circ.state, 0, 8);
	CHECK(circ.length == 0 && is(r, 1, 0x00EA, 0, 0));
	result_t tilde = press(kGrave, Shift, 0, 0, 8);
	r = press(kA, 0, tilde.state, 0, 8);
	CHECK(tilde.length == 0 && is(r, 1, 0x00E3, 0, 0));
	// kUCKeyTranslateNoDeadKeysMask: the dead key gives its accent and sets no state.
	r = press(kQuote, 0, 0, kUCKeyTranslateNoDeadKeysMask, 8);
	CHECK(is(r, 1, 0x00B4, 0, 0) && r.state == 0);
	r = press(kE, 0, 0, kUCKeyTranslateNoDeadKeysMask, 8);
	CHECK(is(r, 1, 'e', 0, 0));
	// The Option layer is the keymap's AltGr level.
	CHECK(is(press(kE, Option, 0, 0, 8), 1, 0x00E9, 0, 0));
	CHECK(is(press(kE, Option | Shift, 0, 0, 8), 1, 0x00C9, 0, 0));
	CHECK(is(press(kA, Option, 0, 0, 8), 1, 0x00E1, 0, 0));
}

static void cz(void)
{
	// QWERTZ with the digit row giving accented letters; Shift gives the digits.
	CHECK(is(press(kZ, 0, 0, 0, 8), 1, 'y', 0, 0));
	CHECK(is(press(kY, 0, 0, 0, 8), 1, 'z', 0, 0));
	CHECK(is(press(k2, 0, 0, 0, 8), 1, 0x011B, 0, 0));
	CHECK(is(press(k2, Shift, 0, 0, 8), 1, '2', 0, 0));
	CHECK(is(press(kY, Caps, 0, 0, 8), 1, 'Z', 0, 0));
	CHECK(is(press(k2, Option, 0, 0, 8), 1, '@', 0, 0)); // AltGr-2 on this layout
	// The = key is a dead acute, shifted a dead caron.
	result_t acute = press(kEqual, 0, 0, 0, 8);
	CHECK(acute.length == 0 && acute.state != 0);
	result_t r = press(kE, 0, acute.state, 0, 8);
	CHECK(is(r, 1, 0x00E9, 0, 0));
	result_t caron = press(kEqual, Shift, 0, 0, 8);
	CHECK(caron.length == 0 && caron.state != 0 && caron.state != acute.state);
	r = press(kE, 0, caron.state, 0, 8);
	CHECK(is(r, 1, 0x011B, 0, 0));
	r = press(kX, 0, caron.state, 0, 8);
	CHECK(is(r, 2, 0x02C7, 'x', 0));
	r = press(kEqual, 0, 0, kUCKeyTranslateNoDeadKeysMask, 8);
	CHECK(is(r, 1, 0x00B4, 0, 0) && r.state == 0);
}

static void de(void)
{
	CHECK(is(press(kZ, 0, 0, 0, 8), 1, 'y', 0, 0));
	CHECK(is(press(kY, 0, 0, 0, 8), 1, 'z', 0, 0));
	// The ^ key (left of 1) is a dead circumflex, the ´ key a dead acute.
	result_t circ = press(kGrave, 0, 0, 0, 8);
	CHECK(circ.length == 0 && circ.state != 0);
	result_t r = press(kE, 0, circ.state, 0, 8);
	CHECK(is(r, 1, 0x00EA, 0, 0));
	result_t acute = press(kEqual, 0, 0, 0, 8);
	CHECK(acute.length == 0 && acute.state != 0 && acute.state != circ.state);
	r = press(kE, 0, acute.state, 0, 8);
	CHECK(is(r, 1, 0x00E9, 0, 0));
	// AltGr-e is the euro sign.
	CHECK(is(press(kE, Option, 0, 0, 8), 1, 0x20AC, 0, 0));
	r = press(kEqual, 0, 0, kUCKeyTranslateNoDeadKeysMask, 8);
	CHECK(is(r, 1, 0x00B4, 0, 0) && r.state == 0);
}

int main(int argc, char** argv)
{
	@autoreleasepool {
		const char* layout = argc > 1 ? argv[1] : "us";
		const char* backend = argc > 2 ? argv[2] : NULL;
		g_dump = argc > 3 && strcmp(argv[3], "dump") == 0;
		[NSApplication sharedApplication];
		NSString* displayClass = NSStringFromClass([[NSClassFromString(@"NSDisplay") performSelector: @selector(currentDisplay)] class]);
		printf("display backend: %s\n", [displayClass UTF8String]);
		if (backend && strcmp(backend, "x11") == 0 && ![displayClass hasPrefix: @"X11"]) {
			printf("SKIP: not the X11 backend\n");
			return 0;
		}
		if (backend && strcmp(backend, "wayland") == 0 && ![displayClass hasPrefix: @"Wayland"]) {
			printf("SKIP: not the Wayland backend\n");
			return 0;
		}

		// A Wayland compositor sends the keymap once a keyboard exists (the test driver creates a
		// virtual one), so give the event loop time to receive it.
		TISInputSourceRef source = TISCopyCurrentKeyboardLayoutInputSource();
		for (int i = 0; !source && i < 100; i++) {
			[[NSRunLoop currentRunLoop] runUntilDate: [NSDate dateWithTimeIntervalSinceNow: 0.2]];
			source = TISCopyCurrentKeyboardLayoutInputSource();
		}
		if (!source) {
			printf("FAIL: no keyboard layout input source\n");
			return 1;
		}
		CFDataRef data = (CFDataRef) TISGetInputSourceProperty(source, kTISPropertyUnicodeKeyLayoutData);
		if (!data) {
			printf("FAIL: no kTISPropertyUnicodeKeyLayoutData\n");
			return 1;
		}
		g_layout = (const UCKeyboardLayout*) CFDataGetBytePtr(data);

		if (g_dump) {
			printf("layout data: %ld bytes, %u keyboard type entries\n", (long) CFDataGetLength(data), (unsigned) g_layout->keyboardTypeCount);
			for (int mods = 0; mods < 16; mods += (mods == 4 ? 4 : 2)) {
				printf("modifier byte %d:", mods);
				for (int key = 0; key < 128; key++) {
					result_t r = press(key, mods, 0, 0, 8);
					if (r.length == 0 && r.state == 0)
						continue;
					printf(" %d:", key);
					for (UniCharCount i = 0; i < r.length; i++)
						printf("%04X%s", r.chars[i], i + 1 < r.length ? "," : "");
					if (r.state)
						printf("(dead %u)", (unsigned) r.state);
				}
				printf("\n");
			}
		}

		common();
		if (strcmp(layout, "us") == 0)
			us();
		else if (strcmp(layout, "intl") == 0)
			intl();
		else if (strcmp(layout, "cz") == 0)
			cz();
		else if (strcmp(layout, "de") == 0)
			de();
		else {
			printf("FAIL: unknown layout %s\n", layout);
			return 1;
		}
		CFRelease(source);
	}
	if (g_failures == 0)
		printf("PASS\n");
	else
		printf("%d failure(s)\n", g_failures);
	return g_failures != 0;
}
