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

#ifndef _NavigationServices_Navigation_H_
#define _NavigationServices_Navigation_H_

#include <ApplicationServices/ApplicationServices.h>
#include <HIToolbox/Events.h>
#include <HIToolbox/MacWindows.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __LP64__

#pragma pack(push, 2)

typedef struct __NavDialog* NavDialogRef;

typedef UInt32 NavUserAction;
enum {
	kNavUserActionNone = 0,
	kNavUserActionCancel = 1,
	kNavUserActionOpen = 2,
	kNavUserActionSaveAs = 3,
	kNavUserActionChoose = 4,
	kNavUserActionNewFolder = 5,
	kNavUserActionSaveChanges = 6,
	kNavUserActionDontSaveChanges = 7,
	kNavUserActionDiscardChanges = 8,
	kNavUserActionReviewDocuments = 9,
	kNavUserActionDiscardDocuments = 10
};

typedef UInt32 NavEventCallbackMessage;
enum {
	kNavCBEvent = 0,
	kNavCBCustomize = 1,
	kNavCBStart = 2,
	kNavCBTerminate = 3,
	kNavCBAdjustRect = 4,
	kNavCBNewLocation = 5,
	kNavCBShowDesktop = 6,
	kNavCBSelectEntry = 7,
	kNavCBPopupMenuSelect = 8,
	kNavCBAccept = 9,
	kNavCBCancel = 10,
	kNavCBAdjustPreview = 11,
	kNavCBUserAction = 12,
	kNavCBOpenSelection = (SInt32) 0x80000000
};

typedef UInt32 NavTranslationOptions;
enum {
	kNavTranslateInPlace = 0,
	kNavTranslateCopy = 1
};

typedef UInt32 NavDialogOptionFlags;
enum {
	kNavDefaultNavDlogOptions = 0x000093E4,
	kNavNoTypePopup = 0x00000001,
	kNavDontAutoTranslate = 0x00000002,
	kNavDontAddTranslateItems = 0x00000004,
	kNavAllFilesInPopup = 0x00000010,
	kNavAllowStationery = 0x00000020,
	kNavAllowPreviews = 0x00000040,
	kNavAllowMultipleFiles = 0x00000080,
	kNavAllowInvisibleFiles = 0x00000100,
	kNavDontResolveAliases = 0x00000200,
	kNavSelectDefaultLocation = 0x00000400,
	kNavSelectAllReadableItem = 0x00000800,
	kNavSupportPackages = 0x00001000,
	kNavAllowOpenPackages = 0x00002000,
	kNavDontAddRecents = 0x00004000,
	kNavDontUseCustomFrame = 0x00008000,
	kNavDontConfirmReplacement = 0x00010000,
	kNavPreserveSaveFileExtension = 0x00020000
};

struct NavEventData {
	union {
		EventRecord* event;
		void* param;
	} eventDataParms;
	SInt16 itemHit;
};
typedef struct NavEventData NavEventData;

struct NavCBRec {
	UInt16 version;
	NavDialogRef context;
	WindowRef window;
	Rect customRect;
	Rect previewRect;
	NavEventData eventData;
	char reserved[220];
};
typedef struct NavCBRec NavCBRec;
typedef NavCBRec* NavCBRecPtr;

typedef void (*NavEventProcPtr)(NavEventCallbackMessage callBackSelector, NavCBRecPtr callBackParms, void* callBackUD);
typedef NavEventProcPtr NavEventUPP;

#define NewNavEventUPP(userRoutine) ((NavEventUPP) (userRoutine))
#define DisposeNavEventUPP(userUPP)
#define InvokeNavEventUPP(callBackSelector, callBackParms, callBackUD, userUPP) (*(userUPP))(callBackSelector, callBackParms, callBackUD)

enum {
	kNavReplyRecordVersion = 2,
	kNavDialogCreationOptionsVersion = 0
};

struct NavReplyRecord {
	UInt16 version;
	Boolean validRecord;
	Boolean replacing;
	Boolean isStationery;
	Boolean translationNeeded;
	AEDescList selection;
	ScriptCode keyScript;
	void* fileTranslation;
	UInt32 reserved1;
	CFStringRef saveFileName;
	Boolean saveFileExtensionHidden;
	UInt8 reserved2;
	char reserved[225];
};
typedef struct NavReplyRecord NavReplyRecord;

struct NavDialogCreationOptions {
	UInt16 version;
	NavDialogOptionFlags optionFlags;
	Point location;
	CFStringRef clientName;
	CFStringRef windowTitle;
	CFStringRef actionButtonLabel;
	CFStringRef cancelButtonLabel;
	CFStringRef saveFileName;
	CFStringRef message;
	UInt32 preferenceKey;
	CFArrayRef popupExtension;
	WindowModality modality;
	WindowRef parentWindow;
	char reserved[16];
};
typedef struct NavDialogCreationOptions NavDialogCreationOptions;

// The defaults of the options of a dialog
OSStatus NavGetDefaultDialogCreationOptions(NavDialogCreationOptions* outOptions);

// A dialog that asks where to save a document. The events of the dialog go to the event procedure.
OSStatus NavCreatePutFileDialog(const NavDialogCreationOptions* inOptions, OSType inFileType, OSType inFileCreator,
	NavEventUPP inEventProc, void* inClientData, NavDialogRef* outDialog);

OSStatus NavDialogRun(NavDialogRef inDialog);
void NavDialogDispose(NavDialogRef inDialog);
NavUserAction NavDialogGetUserAction(NavDialogRef inDialog);
OSStatus NavDialogGetReply(NavDialogRef inDialog, NavReplyRecord* outReply);
OSErr NavDisposeReply(NavReplyRecord* reply);
OSStatus NavCompleteSave(const NavReplyRecord* reply, NavTranslationOptions howToTranslate);

/* The dialogs of the first Navigation Services API: they run to the end and return the user's choice */
struct NavTypeList {
	OSType componentSignature;
	SInt16 reserved;
	SInt16 osTypeCount;
	OSType osType[1];
};
typedef struct NavTypeList NavTypeList;
typedef NavTypeList *NavTypeListPtr;
typedef NavTypeListPtr *NavTypeListHandle;

typedef struct OpaqueNavMenuItemSpecArray **NavMenuItemSpecArrayHandle;

struct NavDialogOptions {
	UInt16 version;
	NavDialogOptionFlags dialogOptionFlags;
	Point location;
	Str255 clientName;
	Str255 windowTitle;
	Str255 actionButtonLabel;
	Str255 cancelButtonLabel;
	Str255 savedFileName;
	Str255 message;
	UInt32 preferenceKey;
	NavMenuItemSpecArrayHandle popupExtension;
	char reserved[16];
};
typedef struct NavDialogOptions NavDialogOptions;

enum {
	kNavDialogOptionsVersion = 0
};

typedef SInt16 NavFilterModes;
typedef Boolean (*NavPreviewProcPtr)(NavCBRecPtr callBackParms, void *callBackUD);
typedef Boolean (*NavObjectFilterProcPtr)(AEDesc *theItem, void *info, void *callBackUD, NavFilterModes filterMode);
typedef NavPreviewProcPtr NavPreviewUPP;
typedef NavObjectFilterProcPtr NavObjectFilterUPP;

#define NewNavPreviewUPP(userRoutine) ((NavPreviewUPP) (userRoutine))
#define DisposeNavPreviewUPP(userUPP)
#define NewNavObjectFilterUPP(userRoutine) ((NavObjectFilterUPP) (userRoutine))
#define DisposeNavObjectFilterUPP(userUPP)

/* The defaults of the options of the dialogs below */
OSErr NavGetDefaultDialogOptions(NavDialogOptions *dialogOptions);
Boolean NavServicesAvailable(void);
OSErr NavLoad(void);
OSErr NavUnload(void);

/* Ask the user for files to open (NavGetFile, NavChooseFile), a folder to choose (NavChooseFolder) and a name to
   save a document under (NavPutFile). The result is userCanceledErr when the user cancels; the reply holds the
   choice otherwise. */
OSErr NavGetFile(const AEDesc *defaultLocation, NavReplyRecord *reply, const NavDialogOptions *dialogOptions,
	NavEventUPP eventProc, NavPreviewUPP previewProc, NavObjectFilterUPP filterProc, NavTypeListHandle typeList,
	void *callBackUD);
OSErr NavChooseFile(const AEDesc *defaultLocation, NavReplyRecord *reply, const NavDialogOptions *dialogOptions,
	NavEventUPP eventProc, NavPreviewUPP previewProc, NavObjectFilterUPP filterProc, NavTypeListHandle typeList,
	void *callBackUD);
OSErr NavChooseFolder(const AEDesc *defaultLocation, NavReplyRecord *reply, const NavDialogOptions *dialogOptions,
	NavEventUPP eventProc, NavObjectFilterUPP filterProc, void *callBackUD);
OSErr NavPutFile(const AEDesc *defaultLocation, NavReplyRecord *reply, const NavDialogOptions *dialogOptions,
	NavEventUPP eventProc, OSType fileType, OSType fileCreator, void *callBackUD);

#pragma pack(pop)

#endif

#ifdef __cplusplus
}
#endif

#endif
