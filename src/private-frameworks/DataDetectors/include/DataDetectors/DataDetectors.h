/*
 This file is part of Darling.

 Copyright (C) 2017 Lubos Dolezel

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


#ifndef _DataDetectors_H_
#define _DataDetectors_H_

#import <Foundation/Foundation.h>

#import <DataDetectors/DDActionViewPresenter.h>
#import <DataDetectors/DDHighlightsDataSource.h>
#import <DataDetectors/DDHighlightsDelegate.h>
#import <DataDetectors/DDMiscActionProtocol.h>
#import <DataDetectors/DDOperationDelegate.h>
#import <DataDetectors/NSPopoverDelegate.h>
#import <DataDetectors/NSRemoteViewDelegate.h>
#import <DataDetectors/NSWindowDelegate.h>
#import <DataDetectors/QLPreviewMenuItemDelegate.h>
#import <DataDetectors/DDMenuItem.h>
#import <DataDetectors/DDRemoteBubbleActionController.h>
#import <DataDetectors/DDAction.h>
#import <DataDetectors/DDURLAction.h>
#import <DataDetectors/DDBubbleAction.h>
#import <DataDetectors/DDMiscAction.h>
#import <DataDetectors/DDSeparatorAction.h>
#import <DataDetectors/DDDialController.h>
#import <DataDetectors/DDMessageController.h>
#import <DataDetectors/DDGenericTTYDialController.h>
#import <DataDetectors/DDTTYDirectDialController.h>
#import <DataDetectors/DDTTYRelayDialController.h>
#import <DataDetectors/DDFacetimeController.h>
#import <DataDetectors/DDFacetimeAudioController.h>
#import <DataDetectors/DDActionsManager.h>
#import <DataDetectors/DDHighlightObject.h>
#import <DataDetectors/DDHighlightRequest.h>
#import <DataDetectors/DDDataDetectorsViewHost.h>
#import <DataDetectors/DDSignaturePreviewController.h>
#import <DataDetectors/DDSignatureNewContactEditorController.h>
#import <DataDetectors/DDSignatureExistingContactEditorController.h>
#import <DataDetectors/DDSignatureMapController.h>
#import <DataDetectors/DDButtonBar.h>
#import <DataDetectors/DDScanRequest.h>
#import <DataDetectors/DDOperation.h>
#import <DataDetectors/DDHTMLMessageScanningOperation.h>
#import <DataDetectors/DDParsecController.h>
#import <DataDetectors/DDThreadManager.h>
#import <DataDetectors/DDPhoneOrMailURI.h>
#import <DataDetectors/DDBasicHighlightsView.h>
#import <DataDetectors/DDPatternCompiler.h>
#import <DataDetectors/DDFlightController.h>
#import <DataDetectors/DDPreviewMenuItem.h>
#import <DataDetectors/DDMapTable.h>
#import <DataDetectors/DDActionViewController.h>
#import <DataDetectors/DDHighlightRangeUpdater.h>
#import <DataDetectors/DDRange.h>
#import <DataDetectors/DDHighlightButton.h>
#import <DataDetectors/DDHighlightButtonCell.h>
#import <DataDetectors/DDLargeTypeController.h>
#import <DataDetectors/DDActionContext.h>
#import <DataDetectors/DDEventController.h>
#import <DataDetectors/DDPopover.h>
#import <DataDetectors/DDDetectorManager2.h>
#import <DataDetectors/DDPhoneNumber.h>
#import <DataDetectors/DDHighlightRangeUpdater2.h>
#import <DataDetectors/DDMapLinkController.h>
#import <DataDetectors/DDShowDateController.h>
#import <DataDetectors/DDGenericBubbleController.h>
#import <DataDetectors/DDRange2.h>
#import <DataDetectors/DDWK2Overlay.h>

void* DDActionLogHandle(void);
void* DDHighlightCreateWithRectsInVisibleRect(void);
void* DDHighlightCreateWithRectsInVisibleRectWithStyle(void);
void* DDHighlightCreateWithRectsInVisibleRectWithStyleAndDirection(void);
void* DDHighlightGetActionRect(void);
void* DDHighlightGetBoundingRect(void);
void* DDHighlightGetLayerWithContext(void);
void* DDHighlightGetTrackingRects(void);
void* DDHighlightPointIsOnButton(void);
void* DDHighlightPointIsOnHighlight(void);
void* DDHighlightWantsRollover(void);
void* DDMapsGetBestAddressForResults(void);
void* DDMapsGetDictionaryForFullAddress(void);
void* DDMapsURLStringForAddressResult(void);
void* DDMapsURLStringForString(void);
void* DDMenuItemGetActionUTI(void);
void* DDServiceLogHandle(void);
void* DDUILogHandle(void);
void* _DDUseRTLLayout(void);
void* dd_can_connect_to_service(void);
void* dd_has_bool_entitlement(void);
void* separatorColorForDarkMode(void);

#endif
