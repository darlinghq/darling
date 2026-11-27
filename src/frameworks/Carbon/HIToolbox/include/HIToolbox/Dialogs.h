#ifndef _Carbon_Dialogs_H_
#define _Carbon_Dialogs_H_

#include <HIToolbox/Events.h>
#include <HIToolbox/MacWindows.h>
#include <HIToolbox/Controls.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef int16_t AlertType;
typedef struct OpaqueDialogPtr* DialogPtr;
typedef DialogPtr DialogRef;
typedef uint32_t AlertStdCFStringAlertParamRec;
typedef void* AlertStdCFStringAlertParamPtr;

typedef int16_t DialogItemIndex;
typedef int16_t DialogItemIndexZeroBased;
typedef int16_t DialogItemType;

typedef Boolean (*ModalFilterProcPtr)(DialogRef theDialog, EventRecord* theEvent, DialogItemIndex* itemHit);
typedef ModalFilterProcPtr ModalFilterUPP;
typedef void (*UserItemProcPtr)(DialogRef theDialog, DialogItemIndex itemNo);
typedef UserItemProcPtr UserItemUPP;

// The item types of a 'DITL' resource; itemDisable is added to an item that does not report clicks
enum {
	ctrlItem = 4,
	btnCtrl = 0,
	chkCtrl = 1,
	radCtrl = 2,
	resCtrl = 3,
	statText = 8,
	editText = 16,
	iconItem = 32,
	picItem = 64,
	userItem = 0,
	itemDisable = 128
};

enum {
	kButtonDialogItem = ctrlItem + btnCtrl,
	kCheckBoxDialogItem = ctrlItem + chkCtrl,
	kRadioButtonDialogItem = ctrlItem + radCtrl,
	kResourceControlDialogItem = ctrlItem + resCtrl,
	kStaticTextDialogItem = statText,
	kEditTextDialogItem = editText,
	kIconDialogItem = iconItem,
	kPictureDialogItem = picItem,
	kUserDialogItem = userItem,
	kHelpDialogItem = 1,
	kItemDisableBit = itemDisable
};

enum {
	ok = 1,
	cancel = 2,
	kStdOkItemIndex = 1,
	kStdCancelItemIndex = 2
};

enum {
	kStopIcon = 0,
	kNoteIcon = 1,
	kCautionIcon = 2
};

enum {
	kAlertStopAlert = 0,
	kAlertNoteAlert = 1,
	kAlertCautionAlert = 2,
	kAlertPlainAlert = 3
};

ModalFilterUPP NewModalFilterUPP(ModalFilterProcPtr userRoutine);
void DisposeModalFilterUPP(ModalFilterUPP userUPP);
Boolean InvokeModalFilterUPP(DialogRef theDialog, EventRecord* theEvent, DialogItemIndex* itemHit, ModalFilterUPP userUPP);
UserItemUPP NewUserItemUPP(UserItemProcPtr userRoutine);
void DisposeUserItemUPP(UserItemUPP userUPP);
void InvokeUserItemUPP(DialogRef theDialog, DialogItemIndex itemNo, UserItemUPP userUPP);

OSStatus CreateStandardAlert(AlertType a, CFStringRef b, CFStringRef c, const AlertStdCFStringAlertParamRec * d, DialogRef * e);
OSStatus GetStandardAlertDefaultParams(AlertStdCFStringAlertParamPtr a, UInt32 b);

OSStatus RunStandardAlert(DialogRef a, ModalFilterUPP b, DialogItemIndex * c);

#ifndef __LP64__
// The Dialog Manager (Macintosh Toolbox Essentials, chapter 6): dialogs and alerts made from 'DLOG', 'ALRT' and
// 'DITL' resources
DialogRef GetNewDialog(SInt16 dialogID, void* dStorage, WindowRef behind);
void DisposeDialog(DialogRef theDialog);
WindowRef GetDialogWindow(DialogRef dialog);
CGrafPtr GetDialogPort(DialogRef dialog);
DialogRef GetDialogFromWindow(WindowRef window);

void DrawDialog(DialogRef theDialog);
void ModalDialog(ModalFilterUPP modalFilter, DialogItemIndex* itemHit);
Boolean IsDialogEvent(const EventRecord* theEvent);
Boolean DialogSelect(const EventRecord* theEvent, DialogRef* theDialog, DialogItemIndex* itemHit);

void GetDialogItem(DialogRef theDialog, DialogItemIndex itemNo, DialogItemType* itemType, Handle* item, Rect* box);
void SetDialogItem(DialogRef theDialog, DialogItemIndex itemNo, DialogItemType itemType, Handle item, const Rect* box);
void GetDialogItemText(Handle item, Str255 text);
void SetDialogItemText(Handle item, ConstStr255Param text);
void HideDialogItem(DialogRef theDialog, DialogItemIndex itemNo);
void ShowDialogItem(DialogRef theDialog, DialogItemIndex itemNo);
DialogItemIndexZeroBased FindDialogItem(DialogRef theDialog, Point thePt);
DialogItemIndex CountDITL(DialogRef theDialog);

OSStatus SetDialogDefaultItem(DialogRef theDialog, DialogItemIndex newItem);
OSStatus SetDialogCancelItem(DialogRef theDialog, DialogItemIndex newItem);
DialogItemIndex GetDialogDefaultItem(DialogRef dialog);
DialogItemIndex GetDialogCancelItem(DialogRef dialog);
DialogItemIndex GetDialogKeyboardFocusItem(DialogRef dialog);
void SetDialogFont(SInt16 fontNum);

void ParamText(ConstStr255Param param0, ConstStr255Param param1, ConstStr255Param param2, ConstStr255Param param3);
DialogItemIndex Alert(SInt16 alertID, ModalFilterUPP modalFilter);
DialogItemIndex StopAlert(SInt16 alertID, ModalFilterUPP modalFilter);
DialogItemIndex NoteAlert(SInt16 alertID, ModalFilterUPP modalFilter);
DialogItemIndex CautionAlert(SInt16 alertID, ModalFilterUPP modalFilter);
#endif

#ifdef __cplusplus
}
#endif

#endif
