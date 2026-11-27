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
#include <sys/xattr.h>
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

@interface NSObject (NavigationServicesOpenPanel)
+ (id) openPanel;
- (void) setAllowsMultipleSelection: (BOOL) flag;
- (void) setCanChooseFiles: (BOOL) flag;
- (void) setCanChooseDirectories: (BOOL) flag;
- (void) setResolvesAliases: (BOOL) flag;
- (void) setDirectoryURL: (NSURL*) url;
- (void) setDelegate: (id) delegate;
- (NSArray*) URLs;
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
static Class appKitClass(NSString* name)
{
	static BOOL loaded;
	static dispatch_once_t once;

	dispatch_once(&once, ^{
		loaded = dlopen("/System/Library/Frameworks/AppKit.framework/AppKit", RTLD_LAZY | RTLD_GLOBAL) != NULL;
	});
	return loaded ? NSClassFromString(name) : Nil;
}

static Class filePanelClass(void)
{
	return appKitClass(@"NSSavePanel");
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

// ---- The first Navigation Services API: dialogs that return when the user is done ----------------------------------

OSErr NavGetDefaultDialogOptions(NavDialogOptions* dialogOptions)
{
	if (!dialogOptions)
		return paramErr;
	memset(dialogOptions, 0, sizeof(*dialogOptions));
	dialogOptions->version = kNavDialogOptionsVersion;
	dialogOptions->dialogOptionFlags = kNavDefaultNavDlogOptions;
	dialogOptions->location.h = -1;
	dialogOptions->location.v = -1;
	return noErr;
}

Boolean NavServicesAvailable(void)
{
	return true;
}

OSErr NavLoad(void)
{
	return noErr;
}

OSErr NavUnload(void)
{
	return noErr;
}

static NSString* stringOfPascal(ConstStr255Param string)
{
	if (!string || !string[0])
		return nil;
	return [(NSString*) CFStringCreateWithPascalString(NULL, string, kCFStringEncodingMacRoman) autorelease];
}

// The Finder type of a file: the first four bytes of its Finder information
static OSType finderTypeOfFile(const char* path)
{
	unsigned char info[32];

	if (getxattr(path, "com.apple.FinderInfo", info, sizeof(info), 0, 0) < 4)
		return 0;
	return ((OSType) info[0] << 24) | (info[1] << 16) | (info[2] << 8) | info[3];
}

// Which files of a chooser are offered: those of the types of the list. A file is of a type when its Finder type is
// the type or its file name extension is one of that type's (as the type conforms to the Uniform Type
// Identifier made from the OS type). A list that holds '????' or no types offers every file; folders are always
// offered so that the user can look into them.
@interface NavigationTypeFilter : NSObject
{
	NSMutableArray* types;
	BOOL anyType;
}
- (id) initWithTypeList: (NavTypeListHandle) list;
@end

@implementation NavigationTypeFilter

- (id) initWithTypeList: (NavTypeListHandle) list
{
	if ((self = [super init]))
	{
		types = [[NSMutableArray alloc] init];
		anyType = !list || !*list || (**list).osTypeCount <= 0;
		for (int i = 0; !anyType && i < (**list).osTypeCount; i++)
		{
			OSType type = (**list).osType[i];

			if (type == '????')
				anyType = YES;
			[types addObject: [NSNumber numberWithUnsignedInt: type]];
		}
	}
	return self;
}

- (void) dealloc
{
	[types release];
	[super dealloc];
}

- (BOOL) fileAtPath: (const char*) path hasType: (OSType) type
{
	CFStringRef extension;
	CFStringRef extensionUTI, typeUTI;
	char tag[5] = { (char) (type >> 24), (char) (type >> 16), (char) (type >> 8), (char) type, 0 };
	BOOL matches = NO;
	const char* dot = strrchr(path, '.');

	if (finderTypeOfFile(path) == type)
		return YES;
	if (!dot || dot[1] == 0)
		return NO;
	extension = CFStringCreateWithCString(NULL, dot + 1, kCFStringEncodingUTF8);
	extensionUTI = extension ? UTTypeCreatePreferredIdentifierForTag(kUTTagClassFilenameExtension, extension, NULL) : NULL;
	CFStringRef tagString = CFStringCreateWithCString(NULL, tag, kCFStringEncodingMacRoman);
	typeUTI = tagString ? UTTypeCreatePreferredIdentifierForTag(kUTTagClassOSType, tagString, NULL) : NULL;
	if (extensionUTI && typeUTI)
		matches = UTTypeConformsTo(extensionUTI, typeUTI);
	if (typeUTI)
		CFRelease(typeUTI);
	if (tagString)
		CFRelease(tagString);
	if (extensionUTI)
		CFRelease(extensionUTI);
	if (extension)
		CFRelease(extension);
	return matches;
}

- (BOOL) panel: (id) sender shouldEnableURL: (NSURL*) url
{
	NSNumber* isDirectory = nil;
	const char* path = [url fileSystemRepresentation];

	if (anyType || !path)
		return YES;
	[url getResourceValue: &isDirectory forKey: NSURLIsDirectoryKey error: NULL];
	if ([isDirectory boolValue])
		return YES;
	for (NSNumber* type in types)
		if ([self fileAtPath: path hasType: [type unsignedIntValue]])
			return YES;
	return NO;
}

@end

// The file specification of a path (of a file that need not exist: the folder and the name)
static BOOL makeSpecOfPath(NSString* path, FSSpec* spec)
{
	FSRef folderRef;
	FSCatalogInfo info;
	const char* name = [[path lastPathComponent] fileSystemRepresentation];
	Str255 pascalName;
	OSErr err;

	if (!name || strlen(name) > 31)
		return NO;
	if (FSPathMakeRef((const UInt8*) [[path stringByDeletingLastPathComponent] fileSystemRepresentation], &folderRef, NULL) != noErr
		|| FSGetCatalogInfo(&folderRef, kFSCatInfoVolume | kFSCatInfoNodeID, &info, NULL, NULL, NULL) != noErr)
		return NO;
	pascalName[0] = (unsigned char) strlen(name);
	memcpy(pascalName + 1, name, pascalName[0]);
	err = FSMakeFSSpec(info.volume, (SInt32) info.nodeID, pascalName, spec);
	return err == noErr || err == fnfErr;
}

// The reply for the files that the user chose: a list of file specifications
static BOOL fillReply(NavReplyRecord* reply, NSArray* urls, Boolean replacing, NSString* saveName)
{
	AEDescList list;

	if (AECreateList(NULL, 0, false, &list) != noErr)
		return NO;
	for (NSURL* url in urls)
	{
		FSSpec spec;

		if (!makeSpecOfPath([url path], &spec) || AEPutPtr(&list, 0, typeFSS, &spec, sizeof(spec)) != noErr)
		{
			AEDisposeDesc(&list);
			return NO;
		}
	}
	memset(reply, 0, sizeof(*reply));
	reply->version = kNavReplyRecordVersion;
	reply->validRecord = true;
	reply->replacing = replacing;
	reply->selection = list;
	reply->saveFileName = saveName ? (CFStringRef) [saveName retain] : NULL;
	return YES;
}

static BOOL resolveDefaultLocation(const AEDesc* location, NSURL** url)
{
	FSRef ref;
	AEDesc coerced;
	UInt8 path[1024];

	*url = nil;
	if (!location || location->descriptorType == typeNull)
		return NO;
	if (AECoerceDesc(location, typeFSRef, &coerced) != noErr)
		return NO;
	if (AEGetDescData(&coerced, &ref, sizeof(ref)) == noErr && FSRefMakePath(&ref, path, sizeof(path)) == noErr)
		*url = [NSURL fileURLWithPath: [NSString stringWithUTF8String: (const char*) path]];
	AEDisposeDesc(&coerced);
	return *url != nil;
}

enum { kNavChooseFiles, kNavChooseFolders, kNavChooseSave };

// Runs a file panel of the kind and fills the reply
static OSErr runChooser(int kind, const AEDesc* defaultLocation, NavReplyRecord* reply, const NavDialogOptions* options,
	NavEventUPP eventProc, NavTypeListHandle typeList, void* callBackUD)
{
	struct __NavDialog dialog;
	OSErr result = userCanceledErr;
	NavDialogOptions defaults;

	if (!reply)
		return paramErr;
	if (!options)
	{
		NavGetDefaultDialogOptions(&defaults);
		options = &defaults;
	}
	memset(&dialog, 0, sizeof(dialog));
	dialog.eventProc = eventProc;
	dialog.clientData = callBackUD;
	memset(reply, 0, sizeof(*reply));
	reply->version = kNavReplyRecordVersion;
	AECreateDesc(typeNull, NULL, 0, &reply->selection);
	dialog.userAction = kNavUserActionCancel;
	sendEvent(&dialog, kNavCBStart);
	@autoreleasepool
	{
		Class panelClass = appKitClass(kind == kNavChooseSave ? @"NSSavePanel" : @"NSOpenPanel");

		if (!panelClass)
		{
			result = unimpErr;
		}
		else
		{
			NSURL* directory = nil;
			NavigationTypeFilter* filter = nil;
			NSString* title = stringOfPascal(options->windowTitle);
			NSString* message = stringOfPascal(options->message);
			NSString* prompt = stringOfPascal(options->actionButtonLabel);
			id panel;

			[NSClassFromString(@"NSApplication") sharedApplication];
			panel = kind == kNavChooseSave ? [panelClass savePanel] : [panelClass openPanel];
			if (kind != kNavChooseSave)
			{
				[panel setCanChooseFiles: kind == kNavChooseFiles];
				[panel setCanChooseDirectories: kind == kNavChooseFolders];
				[panel setAllowsMultipleSelection: (options->dialogOptionFlags & kNavAllowMultipleFiles) != 0 && kind == kNavChooseFiles];
				[panel setResolvesAliases: (options->dialogOptionFlags & kNavDontResolveAliases) == 0];
				if (kind == kNavChooseFiles)
				{
					filter = [[[NavigationTypeFilter alloc] initWithTypeList: typeList] autorelease];
					[panel setDelegate: filter];
				}
			}
			if (resolveDefaultLocation(defaultLocation, &directory))
				[panel setDirectoryURL: directory];
			if (message)
				[panel setMessage: message];
			if (prompt)
				[panel setPrompt: prompt];
			if (title && [panel respondsToSelector: @selector(setTitle:)])
				[panel performSelector: @selector(setTitle:) withObject: title];
			if (kind == kNavChooseSave)
			{
				NSString* name = stringOfPascal(options->savedFileName);

				if (name)
					[panel setNameFieldStringValue: name];
				[panel setCanCreateDirectories: YES];
			}
			if ([panel runModal] == kPanelOK)
			{
				NSArray* urls = kind == kNavChooseSave ? [NSArray arrayWithObject: [panel URL]] : [panel URLs];
				NSString* saveName = kind == kNavChooseSave ? [[[panel URL] path] lastPathComponent] : nil;
				Boolean replacing = kind == kNavChooseSave && [[NSFileManager defaultManager] fileExistsAtPath: [[panel URL] path]];

				AEDisposeDesc(&reply->selection);
				if (fillReply(reply, urls, replacing, saveName))
				{
					result = noErr;
					dialog.userAction = kind == kNavChooseSave ? kNavUserActionSaveAs : (kind == kNavChooseFolders ? kNavUserActionChoose : kNavUserActionOpen);
				}
				else
				{
					AECreateDesc(typeNull, NULL, 0, &reply->selection);
					result = fnfErr;
				}
			}
		}
	}
	sendEvent(&dialog, kNavCBUserAction);
	sendEvent(&dialog, kNavCBTerminate);
	return result;
}

OSErr NavGetFile(const AEDesc* defaultLocation, NavReplyRecord* reply, const NavDialogOptions* dialogOptions,
	NavEventUPP eventProc, NavPreviewUPP previewProc, NavObjectFilterUPP filterProc, NavTypeListHandle typeList,
	void* callBackUD)
{
	return runChooser(kNavChooseFiles, defaultLocation, reply, dialogOptions, eventProc, typeList, callBackUD);
}

OSErr NavChooseFile(const AEDesc* defaultLocation, NavReplyRecord* reply, const NavDialogOptions* dialogOptions,
	NavEventUPP eventProc, NavPreviewUPP previewProc, NavObjectFilterUPP filterProc, NavTypeListHandle typeList,
	void* callBackUD)
{
	return runChooser(kNavChooseFiles, defaultLocation, reply, dialogOptions, eventProc, typeList, callBackUD);
}

OSErr NavChooseFolder(const AEDesc* defaultLocation, NavReplyRecord* reply, const NavDialogOptions* dialogOptions,
	NavEventUPP eventProc, NavObjectFilterUPP filterProc, void* callBackUD)
{
	return runChooser(kNavChooseFolders, defaultLocation, reply, dialogOptions, eventProc, NULL, callBackUD);
}

OSErr NavPutFile(const AEDesc* defaultLocation, NavReplyRecord* reply, const NavDialogOptions* dialogOptions,
	NavEventUPP eventProc, OSType fileType, OSType fileCreator, void* callBackUD)
{
	return runChooser(kNavChooseSave, defaultLocation, reply, dialogOptions, eventProc, NULL, callBackUD);
}

#endif
