/*
 This file is part of Darling.

 Copyright (C) 2026 Lubos Dolezel

 Darling is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 Darling is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with Darling.  If not, see <http://www.gnu.org/licenses/>.
*/

// Navigation Services: the dialogs that ask for a file or folder.
//
// A Navigation Services dialog is shown by the Cocoa file panel, which is how the framework is layered on
// macOS: the Carbon framework does not link the application framework, it loads it when a dialog
// is first run. The panel is shown from the event loop of the application (the dialog is run as the
// program's next event is handled, not inside NavDialogRun()), and takes the place of the program's
// windows while it is open, as a modal dialog does. When the user is done, the event procedure gets the user
// action and then the termination, and the reply holds the folder and the name that were chosen.

#ifndef __LP64__

#import <Foundation/Foundation.h>
#include <NavigationServices/Navigation.h>
#include <CoreServices/CoreServices.h>
#include <HIToolbox/CarbonEvents.h>
#include <dlfcn.h>
#include <stdlib.h>
#include <string.h>

// What of the file panel is used. The class is looked up by name once the application framework is loaded
@interface NSObject (NavigationServicesFilePanel)
+ (id) sharedApplication;
+ (id) savePanel;
- (void) setNameFieldStringValue: (NSString*) name;
- (void) setMessage: (NSString*) message;
- (void) setPrompt: (NSString*) prompt;
- (void) setCanCreateDirectories: (BOOL) flag;
- (NSInteger) runModal;
- (NSURL*) URL;
@end

enum { kPanelOK = 1 };

struct __NavDialog
{
	NavEventUPP eventProc;
	void* clientData;
	NavUserAction userAction;
	Boolean running;
	OSType fileType, fileCreator;
	CFStringRef saveFileName;
	CFStringRef windowTitle, message, actionButtonLabel;
	NavReplyRecord reply;
};

OSStatus NavGetDefaultDialogCreationOptions(NavDialogCreationOptions* outOptions)
{
	if (!outOptions)
		return paramErr;
	memset(outOptions, 0, sizeof(*outOptions));
	outOptions->version = kNavDialogCreationOptionsVersion;
	outOptions->optionFlags = kNavDefaultNavDlogOptions;
	outOptions->location.h = -1;
	outOptions->location.v = -1;
	outOptions->modality = kWindowModalityNone;
	return noErr;
}

static CFStringRef copyOrNull(CFStringRef string)
{
	return string ? CFRetain(string) : NULL;
}

OSStatus NavCreatePutFileDialog(const NavDialogCreationOptions* inOptions, OSType inFileType, OSType inFileCreator,
	NavEventUPP inEventProc, void* inClientData, NavDialogRef* outDialog)
{
	if (!outDialog)
		return paramErr;
	*outDialog = NULL;

	struct __NavDialog* dialog = calloc(1, sizeof(*dialog));

	if (!dialog)
		return memFullErr;
	dialog->eventProc = inEventProc;
	dialog->clientData = inClientData;
	dialog->fileType = inFileType;
	dialog->fileCreator = inFileCreator;
	if (inOptions)
	{
		dialog->saveFileName = copyOrNull(inOptions->saveFileName);
		dialog->windowTitle = copyOrNull(inOptions->windowTitle);
		dialog->message = copyOrNull(inOptions->message);
		dialog->actionButtonLabel = copyOrNull(inOptions->actionButtonLabel);
	}
	*outDialog = dialog;
	return noErr;
}

static void sendEvent(NavDialogRef dialog, NavEventCallbackMessage message)
{
	NavCBRec parameters;

	if (!dialog->eventProc)
		return;
	memset(&parameters, 0, sizeof(parameters));
	parameters.version = 0;
	parameters.context = dialog;
	InvokeNavEventUPP(message, &parameters, dialog->clientData, dialog->eventProc);
}

static void clearReply(NavDialogRef dialog)
{
	AEDisposeDesc(&dialog->reply.selection);
	if (dialog->reply.saveFileName)
		CFRelease(dialog->reply.saveFileName);
	memset(&dialog->reply, 0, sizeof(dialog->reply));
}

// The reply for the file the user chose to save as: the folder it is in, and its name
static BOOL makeSaveReply(NavDialogRef dialog, NSURL* url)
{
	NSString* path = [url path];
	NSString* folder = [path stringByDeletingLastPathComponent];
	FSRef folderRef;
	AEDescList list;

	if (!path || FSPathMakeRef((const UInt8*) [folder fileSystemRepresentation], &folderRef, NULL) != noErr)
		return NO;
	if (AECreateList(NULL, 0, false, &list) != noErr)
		return NO;
	if (AEPutPtr(&list, 1, typeFSRef, &folderRef, sizeof(folderRef)) != noErr)
	{
		AEDisposeDesc(&list);
		return NO;
	}
	clearReply(dialog);
	dialog->reply.version = kNavReplyRecordVersion;
	dialog->reply.validRecord = true;
	// the panel asked the user before it took a name that is in use, so a file of that name is replaced
	dialog->reply.replacing = [[NSFileManager defaultManager] fileExistsAtPath: path];
	dialog->reply.selection = list;
	dialog->reply.saveFileName = (CFStringRef) [[path lastPathComponent] retain];
	return YES;
}

// The panel class, once the application framework is loaded (it is not linked: see the top of the file)
static Class filePanelClass(void)
{
	static Class panelClass;
	static dispatch_once_t once;

	dispatch_once(&once, ^{
		if (dlopen("/System/Library/Frameworks/AppKit.framework/AppKit", RTLD_LAZY | RTLD_GLOBAL))
			panelClass = NSClassFromString(@"NSSavePanel");
	});
	return panelClass;
}

static void runDialog(NavDialogRef dialog)
{
	Class panelClass = filePanelClass();
	BOOL saved = NO;

	dialog->userAction = kNavUserActionCancel;
	sendEvent(dialog, kNavCBStart);
	@autoreleasepool
	{
		if (panelClass)
		{
			[NSClassFromString(@"NSApplication") sharedApplication];
			id panel = [panelClass savePanel];
			if (dialog->saveFileName)
				[panel setNameFieldStringValue: (NSString*) dialog->saveFileName];
			if (dialog->message)
				[panel setMessage: (NSString*) dialog->message];
			if (dialog->actionButtonLabel)
				[panel setPrompt: (NSString*) dialog->actionButtonLabel];
			if (dialog->windowTitle && [panel respondsToSelector: @selector(setTitle:)])
				[panel performSelector: @selector(setTitle:) withObject: (NSString*) dialog->windowTitle];
			[panel setCanCreateDirectories: YES];
			if ([panel runModal] == kPanelOK && makeSaveReply(dialog, [panel URL]))
				saved = YES;
		}
	}
	if (saved)
		dialog->userAction = kNavUserActionSaveAs;
	dialog->running = false;
	sendEvent(dialog, kNavCBUserAction);
	sendEvent(dialog, kNavCBTerminate);
}

OSStatus NavDialogRun(NavDialogRef inDialog)
{
	if (!inDialog)
		return paramErr;
	if (inDialog->running)
		return kNavWrongDialogStateErr;
	inDialog->running = true;
	CFRunLoopPerformBlock(CFRunLoopGetMain(), kCFRunLoopCommonModes, ^{
		runDialog(inDialog);
	});
	CFRunLoopWakeUp(CFRunLoopGetMain());
	return noErr;
}

void NavDialogDispose(NavDialogRef inDialog)
{
	if (!inDialog)
		return;
	clearReply(inDialog);
	if (inDialog->saveFileName)
		CFRelease(inDialog->saveFileName);
	if (inDialog->windowTitle)
		CFRelease(inDialog->windowTitle);
	if (inDialog->message)
		CFRelease(inDialog->message);
	if (inDialog->actionButtonLabel)
		CFRelease(inDialog->actionButtonLabel);
	free(inDialog);
}

NavUserAction NavDialogGetUserAction(NavDialogRef inDialog)
{
	return inDialog ? inDialog->userAction : kNavUserActionNone;
}

OSStatus NavDialogGetReply(NavDialogRef inDialog, NavReplyRecord* outReply)
{
	if (!inDialog || !outReply)
		return paramErr;
	memset(outReply, 0, sizeof(*outReply));
	outReply->version = kNavReplyRecordVersion;
	outReply->validRecord = false;
	AECreateDesc(typeNull, NULL, 0, &outReply->selection);
	if (!inDialog->reply.validRecord)
		return noErr;
	outReply->validRecord = true;
	outReply->replacing = inDialog->reply.replacing;
	AEDisposeDesc(&outReply->selection);
	AEDuplicateDesc(&inDialog->reply.selection, &outReply->selection);
	outReply->saveFileName = copyOrNull(inDialog->reply.saveFileName);
	return noErr;
}

OSErr NavDisposeReply(NavReplyRecord* reply)
{
	if (!reply)
		return paramErr;
	AEDisposeDesc(&reply->selection);
	if (reply->saveFileName)
		CFRelease(reply->saveFileName);
	reply->saveFileName = NULL;
	reply->validRecord = false;
	return noErr;
}

OSStatus NavCompleteSave(const NavReplyRecord* reply, NavTranslationOptions howToTranslate)
{
	if (!reply)
		return paramErr;
	return noErr;
}

#endif
