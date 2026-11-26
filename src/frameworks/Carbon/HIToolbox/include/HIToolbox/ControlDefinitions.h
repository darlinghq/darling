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

#ifndef _HITOOLBOX_CONTROLDEFINITIONS_H_
#define _HITOOLBOX_CONTROLDEFINITIONS_H_

// The standard controls: their definition IDs, data tags and parts (bevel buttons, sliders, scroll bars,
// static and edit text, check boxes and radio buttons, ...).

#include <HIToolbox/Controls.h>

#pragma pack(push, 2)

#ifdef __cplusplus
extern "C" {
#endif

// Control definition IDs: procID of the controls made with NewControl
enum {
	kControlSliderProc = 48,
	kControlScrollBarProc = 384,
	kControlScrollBarLiveProc = 386,
	kControlStaticTextProc = 288,
	kControlPushButtonProc = 368,
	kControlCheckBoxProc = 369,
	kControlRadioButtonProc = 370,
	kControlPopupButtonProc = 400,
	kControlGroupBoxTextTitleProc = 160,
	kControlSeparatorLineProc = 144,
	kControlLittleArrowsProc = 96,
	kControlProgressBarProc = 80,
	kControlUserPaneProc = 256,
	kControlEditTextProc = 272,
	kControlTabLargeProc = 128,
	kControlTabSmallProc = 129,
	kControlImageWellProc = 176,
	kControlChasingArrowsProc = 112,
};

enum {
	// The variants of the pop-up button (Appearance) and of the System 7 pop-up menu control (1008, popupMenuProc)
	kControlPopupFixedWidthVariant = 1 << 0,
	kControlPopupVariableWidthVariant = 1 << 1,
	kControlPopupUseAddResMenuVariant = 1 << 2,
	kControlPopupUseWFontVariant = 1 << 3,
};

// Bevel buttons
enum {
	kControlBevelButtonSmallBevel = 0,
	kControlBevelButtonNormalBevel = 1,
	kControlBevelButtonLargeBevel = 2,
};

enum {
	kControlBevelButtonSmallBevelProc = 32,
	kControlBevelButtonNormalBevelProc = 33,
	kControlBevelButtonLargeBevelProc = 34,
	kControlBevelButtonSmallBevelVariant = 0,
	kControlBevelButtonNormalBevelVariant = 1,
	kControlBevelButtonLargeBevelVariant = 2,
	kControlBevelButtonMenuOnBottom = 0,
	kControlBevelButtonMenuOnRight = 4,
	kControlBehaviorPushbutton = 0,
	kControlBehaviorToggles = 256,
	kControlBehaviorSticky = 512,
	kControlBehaviorSingleValueMenu = 0,
	kControlBehaviorMultiValueMenu = 16384,
	kControlBehaviorOffsetContents = 32768,
	kControlBehaviorCommandMenu = 8192,
};

enum {
	kControlBevelButtonMenuValueTag = 'mval',
	kControlBevelButtonMenuHandleTag = 'mhnd',
	kControlBevelButtonMenuRefTag = 'mhnd',
	kControlBevelButtonContentTag = 'cont',
	kControlBevelButtonTransformTag = 'tran',
	kControlBevelButtonTextAlignTag = 'tali',
	kControlBevelButtonTextOffsetTag = 'toff',
	kControlBevelButtonGraphicAlignTag = 'gali',
	kControlBevelButtonGraphicOffsetTag = 'goff',
	kControlBevelButtonTextPlaceTag = 'tplc',
	kControlBevelButtonCenterPopupGlyphTag = 'pglc',
	kControlBevelButtonLastMenuTag = 'lmnu',
	kControlBevelButtonMenuDelayTag = 'mdly',
	kControlBevelButtonScaleIconTag = 'scal',
	kControlBevelButtonOwnedMenuRefTag = 'omrf',
	kControlBevelButtonKindTag = 'bebk',
};

enum {
	kControlContentTextOnly = 0,
	kControlNoContent = 0,
	kControlContentIconSuiteRes = 1,
	kControlContentCIconRes = 2,
	kControlContentPictRes = 3,
	kControlContentPictHandle = 131,
	kControlContentIconSuiteHandle = 129,
	kControlContentCIconHandle = 130,
	kControlContentIconRef = 132,
	kControlContentCGImageRef = 134,
};

// Static text, edit text and scroll bars
enum {
	kControlStaticTextTextTag = 'text',
	kControlStaticTextTextHeightTag = 'thei',
	kControlStaticTextStyleTag = 'font',
	kControlStaticTextTruncTag = 'trun',
	kControlStaticTextCFStringTag = 'cfst',
	kControlEditTextCFStringTag = 'cfst',
	kControlEditTextTextTag = 'text',
	kControlEditTextSelectionTag = 'sele',
	kControlEditTextPasswordTag = 'pass',
	kControlEditTextPasswordCFStringTag = 'pwcf',
	kControlScrollBarShowsArrowsTag = 'arro',
};

enum {
	kControlCheckBoxUncheckedValue = 0,
	kControlCheckBoxCheckedValue = 1,
	kControlCheckBoxMixedValue = 2,
	kControlRadioButtonUncheckedValue = 0,
	kControlRadioButtonCheckedValue = 1,
	kControlRadioButtonMixedValue = 2,
};

#ifdef __cplusplus
}
#endif

#pragma pack(pop)

#endif
