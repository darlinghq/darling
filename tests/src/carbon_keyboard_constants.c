// Copyright (C) 2026 Lubos Dolezel
//
// The virtual key codes of HIToolbox's Events.h, the ISO and JIS keyboard
// keys and the right Command key among them, and the parameter names of
// keyboard Carbon events. Programs store these values (in preferences, in
// shortcuts), so they must be those of the macOS SDK headers.
#include <Carbon/Carbon.h>
#include <stdio.h>

static int g_failures = 0;

#define CHECK(cond) do { if (!(cond)) { printf("FAIL %d: %s\n", __LINE__, #cond); g_failures++; } } while (0)

int main(void)
{
	CHECK(kVK_RightCommand == 0x36);
	CHECK(kVK_ISO_Section == 0x0A);
	CHECK(kVK_JIS_Yen == 0x5D);
	CHECK(kVK_JIS_Underscore == 0x5E);
	CHECK(kVK_JIS_KeypadComma == 0x5F);
	CHECK(kVK_JIS_Eisu == 0x66);
	CHECK(kVK_JIS_Kana == 0x68);

	CHECK(kEventParamKeyCode == 'kcod');
	CHECK(kEventParamKeyMacCharCodes == 'kchr');
	CHECK(kEventParamKeyModifiers == 'kmod');
	CHECK(kEventParamKeyUnicodes == 'kuni');
	CHECK(kEventParamKeyboardType == 'kbdt');

	printf("%s\n", g_failures ? "FAILED" : "ALL PASSED");
	return g_failures != 0;
}
