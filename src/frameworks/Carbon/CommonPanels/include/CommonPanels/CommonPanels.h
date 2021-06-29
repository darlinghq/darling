/*
 This file is part of Darling.

 Copyright (C) 2021 Lubos Dolezel

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


#ifndef _CommonPanels_H_
#define _CommonPanels_H_

#import <Foundation/Foundation.h>


void* CCP_FontPanelClose(void);
void* CCP_FontPanelGetSelection(void);
void* CCP_FontPanelIsVisible(void);
void* CCP_FontPanelOpen(void);
void* CCP_FontPanelSelectFonts(void);
void* CCP_NPickColor(void);
void* CMCalibrateDisplay(void);
void* Call_CommonCocoaPanels_NPickColor(void);
void* DisposeCalibrateEventUPP(void);
void* DisposeCalibrateUPP(void);
void* DisposeCanCalibrateUPP(void);
void* DisposeNColorChangedUPP(void);
void* FCAddCollection(void);
void* FCAddFontDescriptorToCollection(void);
void* FCCopyCollectionNames(void);
void* FCCopyFontDescriptorsInCollection(void);
void* FCFontDescriptorCreateWithFontAttributes(void);
void* FCFontDescriptorCreateWithName(void);
void* FCRemoveCollection(void);
void* FCRemoveFontDescriptorFromCollection(void);
void* FPIsFontPanelVisible(void);
void* FPShowHideFontPanel(void);
void* GetColor(void);
void* GetHandleApplicationEventsUPP(void);
void* GetHandleWinEventsUPP(void);
void* InvokeCalibrateEventUPP(void);
void* InvokeCalibrateUPP(void);
void* InvokeCanCalibrateUPP(void);
void* InvokeNColorChangedUPP(void);
void* LoadNSTypographyBundle(void);
void* NPickColor(void);
void* NewCalibrateEventUPP(void);
void* NewCalibrateUPP(void);
void* NewCanCalibrateUPP(void);
void* NewNColorChangedUPP(void);
void* SetFontInfoForSelection(void);
void* SoftLinkingGetFunctionPointerFromBundleInDirectory(void);
void* UserWantsColorPanelFromCommonCocoaPanels(void);
void* _SoftLinkingLoadBundleFromLocation(void);

#endif
