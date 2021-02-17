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


#ifndef _JavaRuntimeSupport_H_
#define _JavaRuntimeSupport_H_

#import <Foundation/Foundation.h>

#import <JavaRuntimeSupport/JRSDrag.h>
#import <JavaRuntimeSupport/JRSAppKitAWT.h>
#import <JavaRuntimeSupport/JRSMenu.h>
#import <JavaRuntimeSupport/_JRSUIRenderer.h>
#import <JavaRuntimeSupport/JRSInputMethodController.h>
#import <JavaRuntimeSupport/MIGThread.h>
#import <JavaRuntimeSupport/JRSRenderServer.h>
#import <JavaRuntimeSupport/JRSSymbolicator.h>
#import <JavaRuntimeSupport/JRSSymbolicatorImpl.h>
#import <JavaRuntimeSupport/JRSSecurity.h>

void* CopyCFStringToFilesystemRepresentation(void);
void* CopyCFStringToUTF8(void);
void* GetHitPart(void);
void* GetScrollBarHitPart(void);
void* JRSAccessibilityUnregisterUniqueIdForUIElement(void);
void* JRSCopyCanonicalLanguageForPrimaryLanguage(void);
void* JRSCopyOSJavaSupportVersion(void);
void* JRSCopyOSName(void);
void* JRSCopyOSVersion(void);
void* JRSCopyPrimaryLanguage(void);
void* JRSFontAlignStyleForFractionalMeasurement(void);
void* JRSFontAlignStyleForIntegerMeasurement(void);
void* JRSFontCreateFallbackFontForCharacters(void);
void* JRSFontGetAdvancesForGlyphsAndStyle(void);
void* JRSFontGetBoundingBoxesForGlyphsAndStyle(void);
void* JRSFontGetRenderingStyleForContext(void);
void* JRSFontGetRenderingStyleForHints(void);
void* JRSFontSetRenderingStyleOnContext(void);
void* JRSFontStyleIsAntialiased(void);
void* JRSFontStyleUsesFractionalMetrics(void);
void* JRSSetDefaultLocalization(void);
void* JRSUIControlCreate(void);
void* JRSUIControlDraw(void);
void* JRSUIControlGetHitPart(void);
void* JRSUIControlGetScrollBarOffsetFor(void);
void* JRSUIControlGetScrollBarPartBounds(void);
void* JRSUIControlRelease(void);
void* JRSUIControlSetAlignmentHorizontal(void);
void* JRSUIControlSetAlignmentVertical(void);
void* JRSUIControlSetAnimating(void);
void* JRSUIControlSetDirection(void);
void* JRSUIControlSetOrientation(void);
void* JRSUIControlSetPresentationState(void);
void* JRSUIControlSetScrollBarPart(void);
void* JRSUIControlSetSegmentPosition(void);
void* JRSUIControlSetShowArrows(void);
void* JRSUIControlSetSize(void);
void* JRSUIControlSetState(void);
void* JRSUIControlSetUserInterfaceLayoutDirection(void);
void* JRSUIControlSetValueByKey(void);
void* JRSUIControlSetVariant(void);
void* JRSUIControlSetWidget(void);
void* JRSUIControlSetWindowType(void);
void* JRSUIControlShouldScrollToClick(void);
void* JRSUIGetKey(void);
void* JRSUIRendererCreate(void);
void* JRSUIRendererRelease(void);
void* do_transfer_port_send_right(void);
void* port_send_right_transfer_server(void);
void* port_send_right_transfer_server_routine(void);
void* transfer_port_send_right(void);

#endif
