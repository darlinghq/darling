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

#ifndef _CommonPanels_ColorPicker_H_
#define _CommonPanels_ColorPicker_H_

#include <ApplicationServices/ApplicationServices.h>
#include <HIToolbox/Events.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __LP64__

#pragma pack(push, 2)

enum {
	kDefaultColorPickerWidth = 383,
	kDefaultColorPickerHeight = 238
};

typedef SInt16 DialogPlacementSpec;
enum {
	kAtSpecifiedOrigin = 0,
	kDeepestColorScreen = 1,
	kCenterOnMainScreen = 2
};

enum {
	kColorPickerDialogIsMoveable = 1,
	kColorPickerDialogIsModal = 2,
	kColorPickerCanModifyPalette = 4,
	kColorPickerCanAnimatePalette = 8,
	kColorPickerAppIsColorSyncAware = 16,
	kColorPickerInSystemDialog = 32,
	kColorPickerInApplicationDialog = 64,
	kColorPickerInPickerDialog = 128,
	kColorPickerDetachedFromChoices = 256,
	kColorPickerCallColorProcLive = 512
};

typedef unsigned short SmallFract;

struct HSVColor { SmallFract hue; SmallFract saturation; SmallFract value; };
typedef struct HSVColor HSVColor;
struct HSLColor { SmallFract hue; SmallFract saturation; SmallFract lightness; };
typedef struct HSLColor HSLColor;
struct CMYColor { SmallFract cyan; SmallFract magenta; SmallFract yellow; };
typedef struct CMYColor CMYColor;
struct CMYKColor { UInt16 cyan; UInt16 magenta; UInt16 yellow; UInt16 black; };
typedef struct CMYKColor CMYKColor;
struct CMGrayColor { UInt16 gray; };
typedef struct CMGrayColor CMGrayColor;

// A colour in one of the colour models; RGB is the one of the colour picker
union CMColor {
	RGBColor rgb;
	HSVColor hsv;
	HSLColor hsl;
	CMYKColor cmyk;
	CMYColor cmy;
	CMGrayColor gray;
	UInt8 multichannel[8];
};
typedef union CMColor CMColor;

struct NPMColor {
	CMProfileRef profile;
	CMColor color;
};
typedef struct NPMColor NPMColor;
typedef NPMColor* NPMColorPtr;

struct PickerMenuItemInfo {
	SInt16 editMenuID;
	SInt16 cutItem;
	SInt16 copyItem;
	SInt16 pasteItem;
	SInt16 clearItem;
	SInt16 undoItem;
};
typedef struct PickerMenuItemInfo PickerMenuItemInfo;

typedef void (*NColorChangedProcPtr)(SRefCon userData, NPMColor* newColor);
typedef NColorChangedProcPtr NColorChangedUPP;
typedef Boolean (*UserEventProcPtr)(EventRecord* event);
typedef UserEventProcPtr UserEventUPP;

#define NewNColorChangedUPP(userRoutine) ((NColorChangedUPP) (userRoutine))
#define DisposeNColorChangedUPP(userUPP)
#define InvokeNColorChangedUPP(userData, newColor, userUPP) (*(userUPP))(userData, newColor)

// What NPickColor() takes and returns. The colour is the one to start with, and the picked colour when
// the user accepted it (newColorChosen).
struct NColorPickerInfo {
	NPMColor theColor;
	CMProfileRef dstProfile;
	UInt32 flags;
	DialogPlacementSpec placeWhere;
	Point dialogOrigin;
	OSType pickerType;
	UserEventUPP eventProc;
	NColorChangedUPP colorProc;
	UInt32 colorProcData;
	Str255 prompt;
	PickerMenuItemInfo mInfo;
	Boolean newColorChosen;
	SInt8 filler;
};
typedef struct NColorPickerInfo NColorPickerInfo;

// Shows the colour picker (the colour panel) and returns what the user chose
OSErr NPickColor(NColorPickerInfo* theColorInfo);

#pragma pack(pop)

#endif

#ifdef __cplusplus
}
#endif

#endif
