// Copyright (C) 2026 Lubos Dolezel
//
// The names that the first Carbon gave to the command event and to its parameter: kEventProcessCommand is the kind
// kEventCommandProcess, kEventParamHICommand names the type of the parameter that holds the HICommand. A handler
// written with them gets the commands that ProcessHICommand() sends, with the HICommand in the direct object.
// (Carbon Event Manager Reference, "Command events".) Both 32-bit and 64-bit processes have the names.
#include <Carbon/Carbon.h>
#include <stdio.h>

static int g_failures;
#define CHECK(cond) do { if (!(cond)) { printf("FAIL line %d: %s\n", __LINE__, #cond); g_failures++; } } while (0)

static int g_received;
static UInt32 g_commandID;
static UInt32 g_attributes;

static OSStatus handler(EventHandlerCallRef call, EventRef event, void *userData)
{
	HICommand command;
	CHECK(GetEventKind(event) == kEventProcessCommand);
	if (GetEventParameter(event, kEventParamDirectObject, kEventParamHICommand, NULL, sizeof(command), NULL, &command) != noErr)
		return eventNotHandledErr;
	g_received++;
	g_commandID = command.commandID;
	g_attributes = command.attributes;
	return noErr;
}

int main(void)
{
	CHECK(kEventProcessCommand == kEventCommandProcess);
	CHECK(kEventParamHICommand == typeHICommand);
	CHECK(kEventParamHICommand == 'hcmd');

	static const EventTypeSpec types[] = { { kEventClassCommand, kEventProcessCommand } };
	EventHandlerRef ref;
	CHECK(InstallApplicationEventHandler(NewEventHandlerUPP(handler), 1, types, NULL, &ref) == noErr);
	HICommand command = { kHICommandFromMenu, 'rrtl', { NULL, 0 } };
	CHECK(ProcessHICommand(&command) == noErr);
	CHECK(g_received == 1 && g_commandID == 'rrtl' && g_attributes == kHICommandFromMenu);
	RemoveEventHandler(ref);

	printf(g_failures ? "FAIL carbon_event_legacy_names\n" : "PASS carbon_event_legacy_names\n");
	return g_failures != 0;
}
