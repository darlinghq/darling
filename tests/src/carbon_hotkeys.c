// Copyright (C) 2026 Lubos Dolezel
//
// Carbon event handlers and global hot keys (RegisterEventHotKey): the documented contract of the
// registration calls, the event targets, handler installation and dispatch order, and the event
// parameters. The program passes unchanged on macOS 26.7.3 (the calls were observed with
// mac-probes/v443-register-event-hotkey.c, v443-hotkey-modifier-bits.c and
// v443-handler-install-duplicates.c).
//
// With HOTKEY_CGS=1 (Darling only) the CoreGraphics hot key SPI under RegisterEventHotKey is
// checked as well. Delivery of the hot key events is tested by carbon_hotkeys_appkit.m, since
// the events travel through the application's event queue.
#include <Carbon/Carbon.h>
#include <CoreFoundation/CoreFoundation.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int g_failures = 0;

#define CHECK(cond) do { if (!(cond)) { printf("FAIL %d: %s\n", __LINE__, #cond); g_failures++; } } while (0)

static int g_order[16], g_calls;
static OSStatus g_result;

static OSStatus handlerA(EventHandlerCallRef next, EventRef e, void* ud) { g_order[g_calls++] = 1; return (long) ud == 1 ? eventNotHandledErr : noErr; }
static OSStatus handlerB(EventHandlerCallRef next, EventRef e, void* ud) { g_order[g_calls++] = 2; return (long) ud == 2 ? CallNextEventHandler(next, e) : noErr; }
static OSStatus handlerC(EventHandlerCallRef next, EventRef e, void* ud)
{
	g_order[g_calls++] = 3;
	return (long) ud == 3 ? eventPassToNextTargetErr : (OSStatus) 77;
}

static int g_pressed, g_released;
static EventHotKeyID g_lastID;

static OSStatus hotKeyHandler(EventHandlerCallRef next, EventRef e, void* ud)
{
	EventHotKeyID id;
	OSType type = 0;
	ByteCount size = 0;

	if (GetEventClass(e) != kEventClassKeyboard)
		return eventNotHandledErr;
	if (GetEventParameter(e, kEventParamDirectObject, typeEventHotKeyID, &type, sizeof(id), &size, &id) != noErr)
		return eventNotHandledErr;
	g_lastID = id;
	if (GetEventKind(e) == kEventHotKeyPressed)
		g_pressed++;
	else if (GetEventKind(e) == kEventHotKeyReleased)
		g_released++;
	return noErr;
}

static EventRef makeEvent(UInt32 kind)
{
	EventRef event = NULL;
	OSStatus s = CreateEvent(NULL, kEventClassKeyboard, kind, 0, kEventAttributeNone, &event);
	CHECK(s == noErr && event != NULL);
	return event;
}

static void run(EventRef event, EventTargetRef target)
{
	g_calls = 0;
	g_result = SendEventToEventTarget(event, target);
}

static void testEvents(void)
{
	EventRef e = makeEvent(kEventHotKeyPressed);
	EventHotKeyID id = { 'tst1', 1 }, got = { 0, 0 };
	OSType type = 0;
	ByteCount size = 0;
	unsigned char small[8];

	CHECK(GetEventClass(e) == kEventClassKeyboard);
	CHECK(GetEventKind(e) == kEventHotKeyPressed);
	CHECK(GetEventTime(e) > 0);
	CHECK(GetEventRetainCount(e) == 1);
	RetainEvent(e);
	CHECK(GetEventRetainCount(e) == 2);
	ReleaseEvent(e);

	CHECK(GetEventParameter(e, kEventParamDirectObject, typeEventHotKeyID, NULL, sizeof(got), NULL, &got) == eventParameterNotFoundErr);
	CHECK(SetEventParameter(e, kEventParamDirectObject, typeEventHotKeyID, sizeof(id), &id) == noErr);
	CHECK(GetEventParameter(e, kEventParamDirectObject, typeEventHotKeyID, &type, sizeof(got), &size, &got) == noErr);
	CHECK(type == typeEventHotKeyID && size == sizeof(id) && got.signature == 'tst1' && got.id == 1);
	// Another type than the parameter was stored as is a coercion failure, a wildcard is not accepted
	CHECK(GetEventParameter(e, kEventParamDirectObject, typeUInt32, NULL, sizeof(got), NULL, &got) == errAECoercionFail);
	CHECK(GetEventParameter(e, kEventParamDirectObject, typeWildCard, NULL, sizeof(got), NULL, &got) == paramErr);
	// A small buffer gets the beginning of the value and the full size is reported
	memset(small, 0xee, sizeof(small));
	size = 0;
	CHECK(GetEventParameter(e, kEventParamDirectObject, typeEventHotKeyID, NULL, 4, &size, small) == noErr);
	CHECK(size == 8 && small[0] == ((unsigned char*) &got)[0] && small[4] == 0xee);
	// Only the size
	size = 0;
	CHECK(GetEventParameter(e, kEventParamDirectObject, typeEventHotKeyID, NULL, 0, &size, NULL) == noErr && size == 8);
	// Setting again replaces the value
	id.id = 9;
	CHECK(SetEventParameter(e, kEventParamDirectObject, typeEventHotKeyID, sizeof(id), &id) == noErr);
	CHECK(GetEventParameter(e, kEventParamDirectObject, typeEventHotKeyID, NULL, sizeof(got), NULL, &got) == noErr && got.id == 9);
	ReleaseEvent(e);
}

// Handlers for no event types are accepted and never see an event (observed on macOS)
static void quirk_handler_for_no_event_types(void)
{
	EventTargetRef target = GetApplicationEventTarget();
	EventTypeSpec press = { kEventClassKeyboard, kEventHotKeyPressed };
	EventHandlerRef r = NULL, r2 = NULL;
	EventRef event = makeEvent(kEventHotKeyPressed);

	CHECK(InstallEventHandler(target, handlerA, 0, &press, (void*) 11, &r) == noErr && r != NULL);
	CHECK(InstallEventHandler(target, handlerA, 1, NULL, (void*) 12, &r2) == noErr && r2 != NULL);
	run(event, target);
	CHECK(g_result == eventNotHandledErr && g_calls == 0);
	CHECK(RemoveEventHandler(r) == noErr);
	CHECK(RemoveEventHandler(r2) == noErr);
	ReleaseEvent(event);
}

// No event to send is reported as not handled (observed on macOS)
static void quirk_send_no_event_is_not_handled(void)
{
	CHECK(SendEventToEventTarget(NULL, GetApplicationEventTarget()) == eventNotHandledErr);
}

static void testHandlers(void)
{
	EventTargetRef target = GetApplicationEventTarget();
	EventTypeSpec press = { kEventClassKeyboard, kEventHotKeyPressed }, release = { kEventClassKeyboard, kEventHotKeyReleased };
	EventTypeSpec both[2] = { press, release };
	EventHandlerRef r[8] = { 0 };
	EventRef pressEvent = makeEvent(kEventHotKeyPressed), releaseEvent = makeEvent(kEventHotKeyReleased);

	quirk_handler_for_no_event_types();
	CHECK(target != NULL);
	CHECK(GetEventDispatcherTarget() != NULL && GetEventDispatcherTarget() != target);
	CHECK(GetApplicationEventTarget() == target);

	// Which installations are duplicates: the same routine with the same user data, whatever the types
	CHECK(InstallEventHandler(target, handlerA, 1, &press, (void*) 0, &r[0]) == noErr && r[0] != NULL);
	CHECK(InstallEventHandler(target, handlerA, 1, &press, (void*) 5, &r[1]) == noErr);
	CHECK(InstallEventHandler(target, handlerA, 1, &release, (void*) 0, &r[2]) == eventHandlerAlreadyInstalledErr);
	CHECK(InstallEventHandler(target, handlerA, 2, both, (void*) 0, &r[2]) == eventHandlerAlreadyInstalledErr);
	CHECK(InstallEventHandler(target, handlerB, 1, &press, (void*) 0, &r[3]) == noErr);
	for (int i = 0; i < 4; i++)
		if (r[i])
			CHECK(RemoveEventHandler(r[i]) == noErr);
	CHECK(RemoveEventHandler(r[0]) == paramErr);
	CHECK(RemoveEventHandler(NULL) == paramErr);

	// Bad arguments
	CHECK(InstallEventHandler(NULL, handlerA, 1, &press, NULL, &r[0]) == paramErr);
	CHECK(InstallEventHandler(target, NULL, 1, &press, NULL, &r[0]) == paramErr);

	// No handler: not handled
	run(pressEvent, target);
	CHECK(g_result == eventNotHandledErr && g_calls == 0);

	// The newest handler comes first; a handler that calls the next one gets its result
	CHECK(InstallEventHandler(target, handlerA, 1, &press, (void*) 1, &r[0]) == noErr);   // not handled
	CHECK(InstallEventHandler(target, handlerB, 1, &press, (void*) 2, &r[1]) == noErr);   // calls next
	run(pressEvent, target);
	CHECK(g_result == eventNotHandledErr && g_calls == 2 && g_order[0] == 2 && g_order[1] == 1);

	// A handler that returns another status ends the dispatch with it
	CHECK(InstallEventHandler(target, handlerC, 1, &press, (void*) 9, &r[2]) == noErr);
	run(pressEvent, target);
	CHECK(g_result == 77 && g_calls == 1 && g_order[0] == 3);
	CHECK(RemoveEventHandler(r[2]) == noErr);

	// eventPassToNextTargetErr leaves the target; the last target has none behind it
	CHECK(InstallEventHandler(target, handlerC, 1, &press, (void*) 3, &r[2]) == noErr);
	run(pressEvent, target);
	CHECK(g_result == eventNotHandledErr && g_calls == 1 && g_order[0] == 3);
	CHECK(RemoveEventHandler(r[2]) == noErr);

	// A handler only sees the events it was installed for
	run(releaseEvent, target);
	CHECK(g_result == eventNotHandledErr && g_calls == 0);

	// The dispatcher target hands an event on to the application target
	EventHandlerRef d = NULL;
	CHECK(InstallEventHandler(GetEventDispatcherTarget(), handlerA, 1, &press, (void*) 1, &d) == noErr);
	run(pressEvent, GetEventDispatcherTarget());
	CHECK(g_calls == 3 && g_order[0] == 1 && g_order[1] == 2 && g_order[2] == 1);
	CHECK(RemoveEventHandler(d) == noErr);

	CHECK(SendEventToEventTarget(pressEvent, NULL) == paramErr);
	quirk_send_no_event_is_not_handled();

	CHECK(RemoveEventHandler(r[0]) == noErr);
	CHECK(RemoveEventHandler(r[1]) == noErr);
	ReleaseEvent(pressEvent);
	ReleaseEvent(releaseEvent);
}

// A key code that names no key registers without an error (observed on macOS; the hot key never fires)
static void quirk_unknown_key_code_registers(void)
{
	EventHotKeyID id = { 'tst1', 2 };
	EventHotKeyRef ref = NULL;

	CHECK(RegisterEventHotKey(200, cmdKey, id, GetApplicationEventTarget(), 0, &ref) == noErr && ref != NULL);
	if (ref)
		CHECK(UnregisterEventHotKey(ref) == noErr);
}

// The Caps Lock modifier bit is refused with eventInternalErr (observed on macOS)
static void quirk_caps_lock_modifier_is_refused(void)
{
	EventHotKeyID id = { 'tst1', 2 };
	EventHotKeyRef ref = NULL;

	CHECK(RegisterEventHotKey(kVK_ANSI_C, alphaLock, id, GetApplicationEventTarget(), 0, &ref) == eventInternalErr && ref == NULL);
}

static void testRegistration(void)
{
	EventTargetRef target = GetApplicationEventTarget();
	EventHotKeyID id1 = { 'tst1', 1 }, id2 = { 'tst1', 2 };
	EventHotKeyRef a = NULL, b = NULL, c = NULL;
	UInt32 all = cmdKey | shiftKey | optionKey | controlKey;

	CHECK(RegisterEventHotKey(kVK_ANSI_A, all, id1, target, 0, &a) == noErr && a != NULL);
	// Only one registration per combination in a process, with or without the exclusive option
	CHECK(RegisterEventHotKey(kVK_ANSI_A, all, id2, target, 0, &b) == eventHotKeyExistsErr && b == NULL);
	CHECK(RegisterEventHotKey(kVK_ANSI_A, all, id2, target, kEventHotKeyExclusive, &b) == eventHotKeyExistsErr);
	// The same key code with other modifiers is another hot key, and no modifiers are allowed
	CHECK(RegisterEventHotKey(kVK_ANSI_A, 0, id2, target, 0, &b) == noErr && b != NULL);
	CHECK(UnregisterEventHotKey(b) == noErr);

	// Bad arguments
	CHECK(RegisterEventHotKey(kVK_ANSI_B, cmdKey, id2, NULL, 0, &c) == paramErr);
	CHECK(RegisterEventHotKey(kVK_ANSI_B, cmdKey, id2, target, 0, NULL) == paramErr);

	CHECK(UnregisterEventHotKey(a) == noErr);
	CHECK(UnregisterEventHotKey(a) == paramErr);
	CHECK(UnregisterEventHotKey(NULL) == paramErr);
	CHECK(RegisterEventHotKey(kVK_ANSI_A, all, id1, target, 0, &a) == noErr);
	CHECK(UnregisterEventHotKey(a) == noErr);

	quirk_unknown_key_code_registers();
	quirk_caps_lock_modifier_is_refused();

	// Any other modifier bit is accepted
	for (int bit = 0; bit < 16; bit++)
	{
		if ((1u << bit) == alphaLock)
			continue;
		b = NULL;
		CHECK(RegisterEventHotKey(kVK_ANSI_D, 1u << bit, id2, target, 0, &b) == noErr);
		if (b)
			UnregisterEventHotKey(b);
	}
}

// The hot key events reach the handler installed for them, with the ID of the registration
static void testDelivery(void)
{
	EventTargetRef target = GetApplicationEventTarget();
	EventTypeSpec specs[2] = { { kEventClassKeyboard, kEventHotKeyPressed }, { kEventClassKeyboard, kEventHotKeyReleased } };
	EventHandlerRef handler = NULL;
	EventHotKeyID id = { 'tst3', 42 };
	EventRef pressed = makeEvent(kEventHotKeyPressed), released = makeEvent(kEventHotKeyReleased);

	CHECK(InstallEventHandler(target, hotKeyHandler, 2, specs, NULL, &handler) == noErr);
	SetEventParameter(pressed, kEventParamDirectObject, typeEventHotKeyID, sizeof(id), &id);
	SetEventParameter(released, kEventParamDirectObject, typeEventHotKeyID, sizeof(id), &id);
	g_pressed = g_released = 0;
	CHECK(SendEventToEventTarget(pressed, target) == noErr);
	CHECK(SendEventToEventTarget(released, target) == noErr);
	CHECK(g_pressed == 1 && g_released == 1 && g_lastID.signature == 'tst3' && g_lastID.id == 42);
	CHECK(RemoveEventHandler(handler) == noErr);
	ReleaseEvent(pressed);
	ReleaseEvent(released);
}

// The window server SPI under RegisterEventHotKey (Darling's CoreGraphics; HOTKEY_CGS=1).
// The meaning of the arguments is inferred, see CoreGraphicsPrivate.h.
extern uint32_t CGSMainConnectionID(void);
extern void CGSSetHotKeyWithExclusion(uint32_t cid, int uid, uint16_t options, uint16_t key, uint32_t flags, int exclusion);
extern bool CGSGetHotKey(uint32_t cid, int uid, uint16_t* options, uint16_t* key, uint32_t* flags);
extern void CGSRemoveHotKey(uint32_t cid, int uid);
extern bool CGSIsHotKeyEnabled(uint32_t cid, int uid);
extern void CGSSetHotKeyEnabled(uint32_t cid, int uid, bool enabled);
extern int CGSGetGlobalHotKeyOperatingMode(uint32_t cid, int* mode);
extern int CGSSetGlobalHotKeyOperatingMode(uint32_t cid, int mode);

static void testCGSHotKeys(void)
{
	uint32_t cid = CGSMainConnectionID();
	uint16_t options = 99, key = 99;
	uint32_t flags = 0;
	int mode = -1;
	const uint32_t ctrlShift = 0x00040000 | 0x00020000;   // kCGEventFlagMaskControl | kCGEventFlagMaskShift

	if (cid == 0)
	{
		printf("SKIP: no window system for the CGS hot key checks\n");
		return;
	}
	CGSSetHotKeyWithExclusion(cid, 9001, 0, kVK_ANSI_Q, ctrlShift, 0);
	CHECK(CGSGetHotKey(cid, 9001, &options, &key, &flags));
	CHECK(key == kVK_ANSI_Q && flags == ctrlShift);
	CHECK(CGSIsHotKeyEnabled(cid, 9001));
	CGSSetHotKeyEnabled(cid, 9001, false);
	CHECK(!CGSIsHotKeyEnabled(cid, 9001));
	CGSSetHotKeyEnabled(cid, 9001, true);
	CHECK(CGSIsHotKeyEnabled(cid, 9001));
	CGSRemoveHotKey(cid, 9001);
	CHECK(!CGSGetHotKey(cid, 9001, NULL, NULL, NULL));
	CHECK(!CGSIsHotKeyEnabled(cid, 9001));

	// A uid set again replaces the hot key
	CGSSetHotKeyWithExclusion(cid, 9002, 0, kVK_ANSI_Q, ctrlShift, 0);
	CGSSetHotKeyWithExclusion(cid, 9002, 0, kVK_ANSI_W, ctrlShift, 0);
	CHECK(CGSGetHotKey(cid, 9002, NULL, &key, NULL) && key == kVK_ANSI_W);
	CGSRemoveHotKey(cid, 9002);

	CHECK(CGSGetGlobalHotKeyOperatingMode(cid, &mode) == 0 && mode == 0);
	CHECK(CGSSetGlobalHotKeyOperatingMode(cid, 1) == 0);
	CHECK(CGSGetGlobalHotKeyOperatingMode(cid, &mode) == 0 && mode == 1);
	CHECK(CGSSetGlobalHotKeyOperatingMode(cid, 2) == 0);
	CHECK(CGSSetGlobalHotKeyOperatingMode(cid, 7) != 0);
	CHECK(CGSSetGlobalHotKeyOperatingMode(cid, 0) == 0);
}

int main(void)
{
	testEvents();
	testHandlers();
	testRegistration();
	testDelivery();

	if (getenv("HOTKEY_CGS"))
		testCGSHotKeys();

	printf(g_failures ? "FAIL: %d checks failed\n" : "PASS\n", g_failures);
	return g_failures != 0;
}
