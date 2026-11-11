// Copyright (C) 2026 Lubos Dolezel
//
// IsCmdChar (deprecated) as observed on macOS 27.0.1 with the US layout: the result depends
// on the key code in message bits 8-15 and the low byte of the test character, in the layer
// selected by Control, otherwise Option, otherwise neither. The event kind, Command, Shift,
// the message's character byte and the test's high byte do not matter.
// Probes: mac-probes/v375-iscmdchar-probe.c and v375-iscmdchar-map.c.
// The program passes unchanged on macOS 27.0.1 (arm64 and x86_64).
#include <Carbon/Carbon.h>
#include <stdio.h>

static int g_failures = 0;

#define CHECK(cond) do { if (!(cond)) { printf("FAIL %d: %s\n", __LINE__, #cond); g_failures++; } } while (0)

static Boolean is(short what, unsigned long message, unsigned short mods, short test)
{
	EventRecord e = { what, message, 0, { 0, 0 }, mods };
	return IsCmdChar(&e, test);
}

int main(void)
{
	// Key code 0x2F is the period key, 0x41 the keypad decimal key; 0x00 the A key.
	CHECK(is(keyDown, 0x2F00 | '.', cmdKey, '.'));
	CHECK(is(keyDown, 0x4100 | '.', cmdKey, '.'));
	CHECK(!is(keyDown, '.', cmdKey, '.'));
	CHECK(!is(keyDown, 0x0000 | 'a', cmdKey, '.'));

	// The event kind and Command do not matter: any event, no modifiers.
	CHECK(is(mouseDown, 0x2F00, 0, '.'));
	CHECK(is(nullEvent, 0x2F00, 0, '.'));
	CHECK(is(keyUp, 0x2F00, cmdKey | shiftKey, '.'));
	CHECK(is(autoKey, 0x2F00, alphaLock, '.'));

	// The message's character byte and the upper message bits do not matter; the key code does.
	CHECK(is(keyDown, 0x0000 | 'b', cmdKey, 'a'));
	CHECK(is(keyDown, 0xABCD0000UL | 0x0000 | 'z', cmdKey, 'a'));
	CHECK(!is(keyDown, 0x0B00 | 'a', cmdKey, 'a'));

	// The test is case-insensitive in the plain layer, and its high byte is ignored.
	CHECK(is(keyDown, 0x0000, cmdKey, 'A'));
	CHECK(is(keyDown, 0x0000, cmdKey, 0x7F00 | 'a'));
	CHECK(is(keyDown, 0x0B00, cmdKey, 'B'));

	// Shifted characters belong to the same key.
	CHECK(is(keyDown, 0x2900, cmdKey, ';'));
	CHECK(is(keyDown, 0x2900, cmdKey, ':'));
	CHECK(is(keyDown, 0x1800, cmdKey, '+'));
	CHECK(is(keyDown, 0x4500, cmdKey, '+'));

	// Key code bit 7 is ignored (it marks no different key).
	CHECK(is(keyDown, 0xAF00, cmdKey, '.'));

	// Option layer: Option-A is a-ring (Mac Roman 0x8C), Shift-Option-A is A-ring (0x81).
	CHECK(is(keyDown, 0x0000, optionKey, 0x8C));
	CHECK(is(keyDown, 0x0000, optionKey | shiftKey, 0x81));
	CHECK(!is(keyDown, 0x0000, optionKey, 'a'));
	CHECK(!is(keyDown, 0x0000, cmdKey, 0x8C));

	// Control layer: Control-A is character 1, and Control wins over Option.
	CHECK(is(keyDown, 0x0000, controlKey, 1));
	CHECK(is(keyDown, 0x0000, controlKey | optionKey, 1));
	CHECK(!is(keyDown, 0x0000, controlKey, 'a'));

	// A test character of 0 matches keys that produce no character (such as Shift, 0x38).
	CHECK(is(keyDown, 0x3800, cmdKey, 0));
	CHECK(!is(keyDown, 0x0000, cmdKey, 0));

	if (g_failures == 0)
		printf("PASS\n");
	return g_failures ? 1 : 0;
}
