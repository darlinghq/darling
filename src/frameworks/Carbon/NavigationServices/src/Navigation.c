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

// Navigation Services: the dialogs that ask for a file or folder. There is no dialog to show yet: a
// dialog that is run is cancelled at once, from the event loop, as when the user dismisses it. The
// event procedure gets the user action (cancel) and the termination, as it does for a dialog that is
// dismissed.

#ifndef __LP64__

#include <NavigationServices/Navigation.h>
#include <dispatch/dispatch.h>
#include <stdlib.h>
#include <string.h>

struct __NavDialog
{
	NavEventUPP eventProc;
	void* clientData;
	NavUserAction userAction;
	Boolean running;
	OSType fileType, fileCreator;
	CFStringRef saveFileName;
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
	if (inOptions && inOptions->saveFileName)
		dialog->saveFileName = CFRetain(inOptions->saveFileName);
	*outDialog = dialog;
	return noErr;
}

static void cancelDialog(NavDialogRef dialog)
{
	NavCBRec parameters;

	memset(&parameters, 0, sizeof(parameters));
	parameters.version = 0;
	parameters.context = dialog;
	dialog->userAction = kNavUserActionCancel;
	dialog->running = false;
	if (dialog->eventProc)
	{
		InvokeNavEventUPP(kNavCBUserAction, &parameters, dialog->clientData, dialog->eventProc);
		InvokeNavEventUPP(kNavCBTerminate, &parameters, dialog->clientData, dialog->eventProc);
	}
}

OSStatus NavDialogRun(NavDialogRef inDialog)
{
	if (!inDialog)
		return paramErr;
	if (inDialog->running)
		return kNavWrongDialogStateErr;
	inDialog->running = true;
	CFRunLoopPerformBlock(CFRunLoopGetMain(), kCFRunLoopCommonModes, ^{
		cancelDialog(inDialog);
	});
	CFRunLoopWakeUp(CFRunLoopGetMain());
	return noErr;
}

void NavDialogDispose(NavDialogRef inDialog)
{
	if (!inDialog)
		return;
	if (inDialog->saveFileName)
		CFRelease(inDialog->saveFileName);
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
