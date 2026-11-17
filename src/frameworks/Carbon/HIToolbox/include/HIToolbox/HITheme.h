/*
 This file is part of Darling.

 Copyright (C) 2026 Darling Developers

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

#ifndef _HITOOLBOX_HITHEME_H_
#define _HITOOLBOX_HITHEME_H_

#include <CoreGraphics/CoreGraphics.h>
#include <CoreText/CTFont.h>
#include <CoreServices/CoreServices.h>
#include <HIServices/HIShape.h>
#include <HIToolbox/Appearance.h>
#include <HIToolbox/HIGeometry.h>
#include <HIToolbox/MacWindows.h>

#ifdef __cplusplus
extern "C" {
#endif

#pragma pack(push, 2)

typedef SInt16 ControlPartCode;

enum {
  kHIThemeOrientationNormal     = 0,
  kHIThemeOrientationInverted   = 1
};
typedef UInt32                          HIThemeOrientation;
enum {
  kHIThemeSplitterAdornmentNone = 0,
  kHIThemeSplitterAdornmentMetal = 1
};
typedef UInt32                          HIThemeSplitterAdornment;
enum {
  kHIThemeGrowBoxKindNormal     = 0,
  kHIThemeGrowBoxKindNone       = 1
};
typedef UInt32                          HIThemeGrowBoxKind;
enum {
  kHIThemeGrowBoxSizeNormal     = 0,
  kHIThemeGrowBoxSizeSmall      = 1
};
typedef UInt32                          HIThemeGrowBoxSize;
enum {
  kHIThemeGroupBoxKindPrimary   = 0,
  kHIThemeGroupBoxKindSecondary = 1,
  kHIThemeGroupBoxKindPrimaryOpaque = 3,
  kHIThemeGroupBoxKindSecondaryOpaque = 4
};
typedef UInt32                          HIThemeGroupBoxKind;
enum {
  kHIThemeHeaderKindWindow      = 0,
  kHIThemeHeaderKindList        = 1
};
typedef UInt32                          HIThemeHeaderKind;
enum {
  kHIThemeFrameTextFieldSquare  = 0,
  kHIThemeFrameListBox          = 1,
  kHIThemeFrameTextFieldRound   = 1000,
  kHIThemeFrameTextFieldRoundSmall = 1001,
  kHIThemeFrameTextFieldRoundMini = 1002
};
typedef UInt32                          HIThemeFrameKind;
enum {
  kHIThemeMenuTitleDrawCondensed = 1 << 0
};
struct HIScrollBarTrackInfo {
  UInt32              version;
  ThemeTrackEnableState  enableState;
  ThemeTrackPressState  pressState;
  CGFloat             viewsize;
};
typedef struct HIScrollBarTrackInfo     HIScrollBarTrackInfo;
struct HIThemeTrackDrawInfo {
  UInt32              version;
  ThemeTrackKind      kind;
  HIRect              bounds;
  SInt32              min;
  SInt32              max;
  SInt32              value;
  UInt32              reserved;
  ThemeTrackAttributes  attributes;
  ThemeTrackEnableState  enableState;
  UInt8               filler1;
  union {
    ScrollBarTrackInfo  scrollbar;
    SliderTrackInfo     slider;
    ProgressTrackInfo   progress;
  }                       trackInfo;
};
typedef struct HIThemeTrackDrawInfo     HIThemeTrackDrawInfo;
struct HIThemeAnimationTimeInfo {
  CFAbsoluteTime      start;
  CFAbsoluteTime      current;
};
typedef struct HIThemeAnimationTimeInfo HIThemeAnimationTimeInfo;
struct HIThemeAnimationFrameInfo {
  UInt32              index;
};
typedef struct HIThemeAnimationFrameInfo HIThemeAnimationFrameInfo;
struct HIThemeButtonDrawInfo {
  UInt32              version;
  ThemeDrawState      state;
  ThemeButtonKind     kind;
  ThemeButtonValue    value;
  ThemeButtonAdornment  adornment;
  union {
    HIThemeAnimationTimeInfo  time;
    HIThemeAnimationFrameInfo  frame;
  }                       animation;
};
typedef struct HIThemeButtonDrawInfo    HIThemeButtonDrawInfo;
typedef HIThemeButtonDrawInfo *         HIThemeButtonDrawInfoPtr;
struct HIThemeSplitterDrawInfo {
  UInt32              version;
  ThemeDrawState      state;
  HIThemeSplitterAdornment  adornment;
};
typedef struct HIThemeSplitterDrawInfo  HIThemeSplitterDrawInfo;
typedef HIThemeSplitterDrawInfo *       HIThemeSplitterDrawInfoPtr;
enum {
  kHIThemeTabAdornmentNone      = 0,
  kHIThemeTabAdornmentFocus     = (1 << 2),
  kHIThemeTabAdornmentLeadingSeparator = (1 << 3),
  kHIThemeTabAdornmentTrailingSeparator = (1 << 4)
};
typedef UInt32                          HIThemeTabAdornment;
enum {
  kHIThemeTabSizeNormal         = 0,
  kHIThemeTabSizeSmall          = 1,
  kHIThemeTabSizeMini           = 3
};
typedef UInt32                          HIThemeTabSize;
enum {
  kHIThemeTabPositionFirst      = 0,
  kHIThemeTabPositionMiddle     = 1,
  kHIThemeTabPositionLast       = 2,
  kHIThemeTabPositionOnly       = 3
};
typedef UInt32                          HIThemeTabPosition;
enum {
  kHIThemeTabKindNormal         = 0
};
typedef UInt32                          HIThemeTabKind;
struct HIThemeTabDrawInfo {
  UInt32              version;
  ThemeTabStyle       style;
  ThemeTabDirection   direction;
  HIThemeTabSize      size;
  HIThemeTabAdornment  adornment;
  HIThemeTabKind      kind;
  HIThemeTabPosition  position;
};
typedef struct HIThemeTabDrawInfo       HIThemeTabDrawInfo;
struct HIThemeTabDrawInfoVersionZero {
  UInt32              version;
  ThemeTabStyle       style;
  ThemeTabDirection   direction;
  HIThemeTabSize      size;
  HIThemeTabAdornment  adornment;
};
typedef struct HIThemeTabDrawInfoVersionZero HIThemeTabDrawInfoVersionZero;
enum {
  kHIThemeTabPaneAdornmentNormal = 0
};
typedef UInt32                          HIThemeTabPaneAdornment;
struct HIThemeTabPaneDrawInfo {
  UInt32              version;
  ThemeDrawState      state;
  ThemeTabDirection   direction;
  HIThemeTabSize      size;
  HIThemeTabKind      kind;
  HIThemeTabPaneAdornment  adornment;
};
typedef struct HIThemeTabPaneDrawInfo   HIThemeTabPaneDrawInfo;
struct HIThemeTabPaneDrawInfoVersionZero {
  UInt32              version;
  ThemeDrawState      state;
  ThemeTabDirection   direction;
  HIThemeTabSize      size;
};
typedef struct HIThemeTabPaneDrawInfoVersionZero HIThemeTabPaneDrawInfoVersionZero;
enum {
  kHIThemeMenuDrawInfoVersionZero = 0,
  kHIThemeMenuDrawInfoVersionOne = 1001
};
struct HIThemeMenuDrawInfo {
  UInt32              version;
  ThemeMenuType       menuType;
  unsigned long       reserved1;
  CGFloat             reserved2;
  UInt32              menuDirection;
  CGFloat             reserved3;
  CGFloat             reserved4;
};
typedef struct HIThemeMenuDrawInfo      HIThemeMenuDrawInfo;
typedef HIThemeMenuDrawInfo *           HIThemeMenuDrawInfoPtr;
struct HIThemeMenuDrawInfoVersionZero {
  UInt32              version;
  ThemeMenuType       menuType;
};
typedef struct HIThemeMenuDrawInfoVersionZero HIThemeMenuDrawInfoVersionZero;
typedef HIThemeMenuDrawInfoVersionZero * HIThemeMenuDrawInfoVersionZeroPtr;
struct HIThemeMenuItemDrawInfo {
  UInt32              version;
  ThemeMenuItemType   itemType;
  ThemeMenuState      state;
};
typedef struct HIThemeMenuItemDrawInfo  HIThemeMenuItemDrawInfo;
typedef HIThemeMenuItemDrawInfo *       HIThemeMenuItemDrawInfoPtr;
struct HIThemeFrameDrawInfo {
  UInt32              version;
  HIThemeFrameKind    kind;
  ThemeDrawState      state;
  Boolean             isFocused;
};
typedef struct HIThemeFrameDrawInfo     HIThemeFrameDrawInfo;
typedef HIThemeFrameDrawInfo *          HIThemeFrameDrawInfoPtr;
struct HIThemeGroupBoxDrawInfo {
  UInt32              version;
  ThemeDrawState      state;
  HIThemeGroupBoxKind  kind;
};
typedef struct HIThemeGroupBoxDrawInfo  HIThemeGroupBoxDrawInfo;
typedef HIThemeGroupBoxDrawInfo *       HIThemeGroupBoxDrawInfoPtr;
struct HIThemeGrabberDrawInfo {
  UInt32              version;
  ThemeDrawState      state;
};
typedef struct HIThemeGrabberDrawInfo   HIThemeGrabberDrawInfo;
typedef HIThemeGrabberDrawInfo *        HIThemeGrabberDrawInfoPtr;
struct HIThemePlacardDrawInfo {
  UInt32              version;
  ThemeDrawState      state;
};
typedef struct HIThemePlacardDrawInfo   HIThemePlacardDrawInfo;
typedef HIThemePlacardDrawInfo *        HIThemePlacardDrawInfoPtr;
struct HIThemeHeaderDrawInfo {
  UInt32              version;
  ThemeDrawState      state;
  HIThemeHeaderKind   kind;
};
typedef struct HIThemeHeaderDrawInfo    HIThemeHeaderDrawInfo;
typedef HIThemeHeaderDrawInfo *         HIThemeHeaderDrawInfoPtr;
struct HIThemeMenuBarDrawInfo {
  UInt32              version;
  ThemeMenuBarState   state;
  OptionBits          attributes;
};
typedef struct HIThemeMenuBarDrawInfo   HIThemeMenuBarDrawInfo;
typedef HIThemeMenuBarDrawInfo *        HIThemeMenuBarDrawInfoPtr;
struct HIThemeMenuTitleDrawInfo {
  UInt32              version;
  ThemeMenuState      state;
  OptionBits          attributes;
  CGFloat             condensedTitleExtra;
};
typedef struct HIThemeMenuTitleDrawInfo HIThemeMenuTitleDrawInfo;
typedef HIThemeMenuTitleDrawInfo *      HIThemeMenuTitleDrawInfoPtr;
struct HIThemeTickMarkDrawInfo {
  UInt32              version;
  ThemeDrawState      state;
};
typedef struct HIThemeTickMarkDrawInfo  HIThemeTickMarkDrawInfo;
typedef HIThemeTickMarkDrawInfo *       HIThemeTickMarkDrawInfoPtr;
struct HIThemeWindowDrawInfo {
  UInt32              version;
  ThemeDrawState      state;
  ThemeWindowType     windowType;
  ThemeWindowAttributes  attributes;
  CGFloat             titleHeight;
  CGFloat             titleWidth;
};
typedef struct HIThemeWindowDrawInfo    HIThemeWindowDrawInfo;
typedef HIThemeWindowDrawInfo *         HIThemeWindowDrawInfoPtr;
struct HIThemeWindowWidgetDrawInfo {
  UInt32              version;
  ThemeDrawState      widgetState;
  ThemeTitleBarWidget  widgetType;
  ThemeDrawState      windowState;
  ThemeWindowType     windowType;
  ThemeWindowAttributes  attributes;
  CGFloat             titleHeight;
  CGFloat             titleWidth;
};
typedef struct HIThemeWindowWidgetDrawInfo HIThemeWindowWidgetDrawInfo;
typedef HIThemeWindowWidgetDrawInfo *   HIThemeWindowWidgetDrawInfoPtr;
struct HIThemeSeparatorDrawInfo {
  UInt32              version;
  ThemeDrawState      state;
};
typedef struct HIThemeSeparatorDrawInfo HIThemeSeparatorDrawInfo;
typedef HIThemeSeparatorDrawInfo *      HIThemeSeparatorDrawInfoPtr;
struct HIThemeScrollBarDelimitersDrawInfo {
  UInt32              version;
  ThemeDrawState      state;
  ThemeWindowType     windowType;
  ThemeWindowAttributes  attributes;
};
typedef struct HIThemeScrollBarDelimitersDrawInfo HIThemeScrollBarDelimitersDrawInfo;
typedef HIThemeScrollBarDelimitersDrawInfo * HIThemeScrollBarDelimitersDrawInfoPtr;
struct HIThemeChasingArrowsDrawInfo {
  UInt32              version;
  ThemeDrawState      state;
  UInt32              index;
};
typedef struct HIThemeChasingArrowsDrawInfo HIThemeChasingArrowsDrawInfo;
typedef HIThemeChasingArrowsDrawInfo *  HIThemeChasingArrowsDrawInfoPtr;
struct HIThemePopupArrowDrawInfo {
  UInt32              version;
  ThemeDrawState      state;
  ThemeArrowOrientation  orientation;
  ThemePopupArrowSize  size;
};
typedef struct HIThemePopupArrowDrawInfo HIThemePopupArrowDrawInfo;
typedef HIThemePopupArrowDrawInfo *     HIThemePopupArrowDrawInfoPtr;
struct HIThemeGrowBoxDrawInfo {
  UInt32              version;
  ThemeDrawState      state;
  HIThemeGrowBoxKind  kind;
  ThemeGrowDirection  direction;
  HIThemeGrowBoxSize  size;
};
typedef struct HIThemeGrowBoxDrawInfo   HIThemeGrowBoxDrawInfo;
typedef HIThemeGrowBoxDrawInfo *        HIThemeGrowBoxDrawInfoPtr;
struct HIThemeBackgroundDrawInfo {
  UInt32              version;
  ThemeDrawState      state;
  ThemeBackgroundKind  kind;
};
typedef struct HIThemeBackgroundDrawInfo HIThemeBackgroundDrawInfo;
typedef HIThemeBackgroundDrawInfo *     HIThemeBackgroundDrawInfoPtr;
extern OSStatus
HIThemeDrawButton(
  const HIRect *                 inBounds,
  const HIThemeButtonDrawInfo *  inDrawInfo,
  CGContextRef                   inContext,
  HIThemeOrientation             inOrientation,
  HIRect *                       outLabelRect);
extern OSStatus
HIThemeGetButtonShape(
  const HIRect *                 inBounds,
  const HIThemeButtonDrawInfo *  inDrawInfo,
  HIShapeRef *                   outShape);
extern OSStatus
HIThemeGetButtonContentBounds(
  const HIRect *                 inBounds,
  const HIThemeButtonDrawInfo *  inDrawInfo,
  HIRect *                       outBounds);
extern OSStatus
HIThemeGetButtonBackgroundBounds(
  const HIRect *                 inBounds,
  const HIThemeButtonDrawInfo *  inDrawInfo,
  HIRect *                       outBounds);
extern OSStatus
HIThemeDrawChasingArrows(
  const HIRect *                        inBounds,
  const HIThemeChasingArrowsDrawInfo *  inDrawInfo,
  CGContextRef                          inContext,
  HIThemeOrientation                    inOrientation);
extern OSStatus
HIThemeDrawPopupArrow(
  const HIRect *                     inBounds,
  const HIThemePopupArrowDrawInfo *  inDrawInfo,
  CGContextRef                       inContext,
  HIThemeOrientation                 inOrientation);
extern OSStatus
HIThemeDrawMenuBarBackground(
  const HIRect *                  inBounds,
  const HIThemeMenuBarDrawInfo *  inDrawInfo,
  CGContextRef                    inContext,
  HIThemeOrientation              inOrientation);
extern OSStatus
HIThemeDrawMenuTitle(
  const HIRect *                    inMenuBarRect,
  const HIRect *                    inTitleRect,
  const HIThemeMenuTitleDrawInfo *  inDrawInfo,
  CGContextRef                      inContext,
  HIThemeOrientation                inOrientation,
  HIRect *                          outLabelRect);
extern OSStatus
HIThemeDrawMenuBackground(
  const HIRect *               inMenuRect,
  const HIThemeMenuDrawInfo *  inMenuDrawInfo,
  CGContextRef                 inContext,
  HIThemeOrientation           inOrientation);
extern OSStatus
HIThemeDrawMenuItem(
  const HIRect *                   inMenuRect,
  const HIRect *                   inItemRect,
  const HIThemeMenuItemDrawInfo *  inItemDrawInfo,
  CGContextRef                     inContext,
  HIThemeOrientation               inOrientation,
  HIRect *                         outContentRect);
extern OSStatus
HIThemeDrawMenuSeparator(
  const HIRect *                   inMenuRect,
  const HIRect *                   inItemRect,
  const HIThemeMenuItemDrawInfo *  inItemDrawInfo,
  CGContextRef                     inContext,
  HIThemeOrientation               inOrientation);
extern OSStatus
HIThemeGetMenuBackgroundShape(
  const HIRect *               inMenuRect,
  const HIThemeMenuDrawInfo *  inMenuDrawInfo,
  HIShapeRef *                 outShape);
enum {
  kHIThemeSegmentPositionFirst  = 0,
  kHIThemeSegmentPositionMiddle = 1,
  kHIThemeSegmentPositionLast   = 2,
  kHIThemeSegmentPositionOnly   = 3
};
typedef UInt32                          HIThemeSegmentPosition;
enum {
  kHIThemeSegmentKindNormal     = 0,
  kHIThemeSegmentKindTextured   = 1,
  kHIThemeSegmentKindInset      = kHIThemeSegmentKindTextured
};
typedef UInt32                          HIThemeSegmentKind;
enum {
  kHIThemeSegmentSizeNormal     = 0,
  kHIThemeSegmentSizeSmall      = 1,
  kHIThemeSegmentSizeMini       = 3
};
typedef UInt32                          HIThemeSegmentSize;
enum {
  kHIThemeSegmentAdornmentNone  = 0,
  kHIThemeSegmentAdornmentFocus = (1 << 2),
  kHIThemeSegmentAdornmentLeadingSeparator = (1 << 3),
  kHIThemeSegmentAdornmentTrailingSeparator = (1 << 4)
};
typedef UInt32                          HIThemeSegmentAdornment;
struct HIThemeSegmentDrawInfo {
  UInt32              version;
  ThemeDrawState      state;
  ThemeButtonValue    value;
  HIThemeSegmentSize  size;
  HIThemeSegmentKind  kind;
  HIThemeSegmentPosition  position;
  HIThemeSegmentAdornment  adornment;
};
typedef struct HIThemeSegmentDrawInfo   HIThemeSegmentDrawInfo;
typedef HIThemeSegmentDrawInfo *        HIThemeSegmentDrawInfoPtr;
extern OSStatus
HIThemeDrawSegment(
  const HIRect *                  inBounds,
  const HIThemeSegmentDrawInfo *  inDrawInfo,
  CGContextRef                    inContext,
  HIThemeOrientation              inOrientation);
extern OSStatus
HIThemeDrawTabPane(
  const HIRect *                  inRect,
  const HIThemeTabPaneDrawInfo *  inDrawInfo,
  CGContextRef                    inContext,
  HIThemeOrientation              inOrientation);
extern OSStatus
HIThemeDrawTab(
  const HIRect *              inRect,
  const HIThemeTabDrawInfo *  inDrawInfo,
  CGContextRef                inContext,
  HIThemeOrientation          inOrientation,
  HIRect *                    outLabelRect);
extern OSStatus
HIThemeGetTabPaneDrawShape(
  const HIRect *      inRect,
  ThemeTabDirection   inDirection,
  HIThemeTabSize      inTabSize,
  HIShapeRef *        outShape);
extern OSStatus
HIThemeGetTabPaneContentShape(
  const HIRect *      inRect,
  ThemeTabDirection   inDirection,
  HIThemeTabSize      inTabSize,
  HIShapeRef *        outShape);
extern OSStatus
HIThemeGetTabDrawShape(
  const HIRect *              inRect,
  const HIThemeTabDrawInfo *  inDrawInfo,
  HIShapeRef *                outShape);
extern OSStatus
HIThemeGetTabShape(
  const HIRect *              inRect,
  const HIThemeTabDrawInfo *  inDrawInfo,
  HIShapeRef *                outShape);
enum {
  kHIThemeTextTruncationNone    = 0,
  kHIThemeTextTruncationMiddle  = 1,
  kHIThemeTextTruncationEnd     = 2,
  kHIThemeTextTruncationDefault = 3
};
typedef UInt32                          HIThemeTextTruncation;
enum {
  kHIThemeTextHorizontalFlushLeft = 0,
  kHIThemeTextHorizontalFlushCenter = 1,
  kHIThemeTextHorizontalFlushRight = 2,
  kHIThemeTextHorizontalFlushDefault = 3
};
typedef UInt32                          HIThemeTextHorizontalFlush;
enum {
  kHIThemeTextVerticalFlushTop  = 0,
  kHIThemeTextVerticalFlushCenter = 1,
  kHIThemeTextVerticalFlushBottom = 2,
  kHIThemeTextVerticalFlushDefault = 3
};
typedef UInt32                          HIThemeTextVerticalFlush;
enum {
  kHIThemeTextBoxOptionNone     = 0,
  kHIThemeTextBoxOptionStronglyVertical = (1 << 1),
  kHIThemeTextBoxOptionEngraved = (1 << 2),
  kHIThemeTextBoxOptionDontClip = (1 << 18)
};
typedef OptionBits                      HIThemeTextBoxOptions;
enum {
  kHIThemeTextInfoVersionZero   = 0,
  kHIThemeTextInfoVersionOne    = 1
};
struct HIThemeTextInfo {
  UInt32              version;
  ThemeDrawState      state;
  ThemeFontID         fontID;
  HIThemeTextHorizontalFlush  horizontalFlushness;
  HIThemeTextVerticalFlush  verticalFlushness;
  HIThemeTextBoxOptions  options;
  HIThemeTextTruncation  truncationPosition;
  UInt32              truncationMaxLines;
  Boolean             truncationHappened;
  UInt8               filler1;
  CTFontRef           font;
};
typedef struct HIThemeTextInfo          HIThemeTextInfo;
extern OSStatus
HIThemeGetTextDimensions(
  CFTypeRef          inString,
  CGFloat            inWidth,
  HIThemeTextInfo *  inTextInfo,
  CGFloat *          outWidth,
  CGFloat *          outHeight,
  CGFloat *          outBaseline);
extern OSStatus
HIThemeDrawTextBox(
  CFTypeRef            inString,
  const HIRect *       inBounds,
  HIThemeTextInfo *    inTextInfo,
  CGContextRef         inContext,
  HIThemeOrientation   inOrientation);
extern CTFontUIFontType
HIThemeGetUIFontType(ThemeFontID inFontID);
extern OSStatus
HIThemeDrawTrack(
  const HIThemeTrackDrawInfo *  inDrawInfo,
  const HIRect *                inGhostRect,
  CGContextRef                  inContext,
  HIThemeOrientation            inOrientation);
extern OSStatus
HIThemeDrawTrackTickMarks(
  const HIThemeTrackDrawInfo *  inDrawInfo,
  ItemCount                     inNumTicks,
  CGContextRef                  inContext,
  HIThemeOrientation            inOrientation);
extern OSStatus
HIThemeDrawTickMark(
  const HIRect *                   inBounds,
  const HIThemeTickMarkDrawInfo *  inDrawInfo,
  CGContextRef                     inContext,
  HIThemeOrientation               inOrientation);
extern OSStatus
HIThemeGetTrackThumbShape(
  const HIThemeTrackDrawInfo *  inDrawInfo,
  HIShapeRef *                  outThumbShape);
extern Boolean
HIThemeHitTestTrack(
  const HIThemeTrackDrawInfo *  inDrawInfo,
  const HIPoint *               inMousePoint,
  ControlPartCode *             outPartHit);
extern OSStatus
HIThemeGetTrackBounds(
  const HIThemeTrackDrawInfo *  inDrawInfo,
  HIRect *                      outBounds);
extern OSStatus
HIThemeGetTrackPartBounds(
  const HIThemeTrackDrawInfo *  inDrawInfo,
  ControlPartCode               inPartCode,
  HIRect *                      outPartBounds);
extern OSStatus
HIThemeGetTrackParts(
  const HIThemeTrackDrawInfo *  inDrawInfo,
  UInt32 *                      outNumberOfParts,
  UInt32                        inMaxParts,
  ControlPartCode *             ioPartsBuffer);
extern OSStatus
HIThemeGetTrackDragRect(
  const HIThemeTrackDrawInfo *  inDrawInfo,
  HIRect *                      outDragRect);
extern OSStatus
HIThemeGetTrackThumbPositionFromOffset(
  const HIThemeTrackDrawInfo *  inDrawInfo,
  const HIPoint *               inThumbOffset,
  CGFloat *                     outRelativePosition);
extern OSStatus
HIThemeGetTrackThumbPositionFromBounds(
  const HIThemeTrackDrawInfo *  inDrawInfo,
  const HIRect *                inThumbBounds,
  CGFloat *                     outRelativePosition);
extern OSStatus
HIThemeGetTrackLiveValue(
  const HIThemeTrackDrawInfo *  inDrawInfo,
  CGFloat                       inRelativePosition,
  SInt32 *                      outValue);
extern OSStatus
HIThemeGetScrollBarTrackRect(
  const HIRect *                inBounds,
  const HIScrollBarTrackInfo *  inTrackInfo,
  Boolean                       inIsHoriz,
  HIRect *                      outTrackBounds);
extern Boolean
HIThemeHitTestScrollBarArrows(
  const HIRect *                inScrollBarBounds,
  const HIScrollBarTrackInfo *  inTrackInfo,
  Boolean                       inIsHoriz,
  const HIPoint *               inPtHit,
  HIRect *                      outTrackBounds,
  ControlPartCode *             outPartCode);
extern OSStatus
HIThemeDrawScrollBarDelimiters(
  const HIRect *                              inContRect,
  const HIThemeScrollBarDelimitersDrawInfo *  inDrawInfo,
  CGContextRef                                inContext,
  HIThemeOrientation                          inOrientation);
extern OSStatus
HIThemeDrawWindowFrame(
  const HIRect *                 inContRect,
  const HIThemeWindowDrawInfo *  inDrawInfo,
  CGContextRef                   inContext,
  HIThemeOrientation             inOrientation,
  HIRect *                       outTitleRect);
extern OSStatus
HIThemeDrawTitleBarWidget(
  const HIRect *                       inContRect,
  const HIThemeWindowWidgetDrawInfo *  inDrawInfo,
  CGContextRef                         inContext,
  HIThemeOrientation                   inOrientation);
extern OSStatus
HIThemeDrawGrowBox(
  const HIPoint *                 inOrigin,
  const HIThemeGrowBoxDrawInfo *  inDrawInfo,
  CGContextRef                    inContext,
  HIThemeOrientation              inOrientation);
extern OSStatus
HIThemeGetGrowBoxBounds(
  const HIPoint *                 inOrigin,
  const HIThemeGrowBoxDrawInfo *  inDrawInfo,
  HIRect *                        outBounds);
extern OSStatus
HIThemeGetWindowShape(
  const HIRect *                 inContRect,
  const HIThemeWindowDrawInfo *  inDrawInfo,
  WindowRegionCode               inWinRegion,
  HIShapeRef *                   outShape);
extern Boolean
HIThemeGetWindowRegionHit(
  const HIRect *                 inContRect,
  const HIThemeWindowDrawInfo *  inDrawInfo,
  const HIPoint *                inPoint,
  WindowRegionCode *             outRegionHit);
extern OSStatus
HIThemeDrawFrame(
  const HIRect *                inRect,
  const HIThemeFrameDrawInfo *  inDrawInfo,
  CGContextRef                  inContext,
  HIThemeOrientation            inOrientation);
extern OSStatus
HIThemeDrawGroupBox(
  const HIRect *                   inRect,
  const HIThemeGroupBoxDrawInfo *  inDrawInfo,
  CGContextRef                     inContext,
  HIThemeOrientation               inOrientation);
extern OSStatus
HIThemeDrawGenericWell(
  const HIRect *                 inRect,
  const HIThemeButtonDrawInfo *  inDrawInfo,
  CGContextRef                   inContext,
  HIThemeOrientation             inOrientation);
extern OSStatus
HIThemeDrawPaneSplitter(
  const HIRect *                   inRect,
  const HIThemeSplitterDrawInfo *  inDrawInfo,
  CGContextRef                     inContext,
  HIThemeOrientation               inOrientation);
extern OSStatus
HIThemeDrawGrabber(
  const HIRect *                  inRect,
  const HIThemeGrabberDrawInfo *  inDrawInfo,
  CGContextRef                    inContext,
  HIThemeOrientation              inOrientation);
extern OSStatus
HIThemeDrawPlacard(
  const HIRect *                  inRect,
  const HIThemePlacardDrawInfo *  inDrawInfo,
  CGContextRef                    inContext,
  HIThemeOrientation              inOrientation);
extern OSStatus
HIThemeDrawHeader(
  const HIRect *                 inRect,
  const HIThemeHeaderDrawInfo *  inDrawInfo,
  CGContextRef                   inContext,
  HIThemeOrientation             inOrientation);
extern OSStatus
HIThemeDrawFocusRect(
  const HIRect *       inRect,
  Boolean              inHasFocus,
  CGContextRef         inContext,
  HIThemeOrientation   inOrientation);
enum {
  kHIThemeFocusRingOnly         = 0,
  kHIThemeFocusRingAbove        = 1,
  kHIThemeFocusRingBelow        = 2
};
typedef UInt32                          HIThemeFocusRing;
extern OSStatus
HIThemeBeginFocus(
  CGContextRef       inContext,
  HIThemeFocusRing   inRing,
  void *             inReserved);
extern OSStatus
HIThemeEndFocus(CGContextRef inContext);
extern OSStatus
HIThemeDrawSeparator(
  const HIRect *                    inRect,
  const HIThemeSeparatorDrawInfo *  inDrawInfo,
  CGContextRef                      inContext,
  HIThemeOrientation                inOrientation);
extern OSStatus
HIThemeSetFill(
  ThemeBrush           inBrush,
  void *               inInfo,
  CGContextRef         inContext,
  HIThemeOrientation   inOrientation);
extern OSStatus
HIThemeSetStroke(
  ThemeBrush           inBrush,
  void *               inInfo,
  CGContextRef         inContext,
  HIThemeOrientation   inOrientation);
extern OSStatus
HIThemeSetTextFill(
  ThemeTextColor       inColor,
  void *               inInfo,
  CGContextRef         inContext,
  HIThemeOrientation   inOrientation);
extern OSStatus
HIThemeApplyBackground(
  const HIRect *                     inBounds,
  const HIThemeBackgroundDrawInfo *  inDrawInfo,
  CGContextRef                       inContext,
  HIThemeOrientation                 inOrientation);
extern OSStatus
HIThemeDrawBackground(
  const HIRect *                     inBounds,
  const HIThemeBackgroundDrawInfo *  inDrawInfo,
  CGContextRef                       inContext,
  HIThemeOrientation                 inOrientation);
extern OSStatus
HIThemeBrushCreateCGColor(
  ThemeBrush    inBrush,
  CGColorRef *  outColor);
extern OSStatus
HIThemeGetTextColorForThemeBrush(
  ThemeBrush        inBrush,
  Boolean           inWindowIsActive,
  ThemeTextColor *  outColor);
extern OSStatus
GetThemeMenuSeparatorHeight(SInt16 * outHeight);
extern OSStatus
GetThemeMenuItemExtra(
  ThemeMenuItemType   inItemType,
  SInt16 *            outHeight,
  SInt16 *            outWidth);
extern OSStatus
GetThemeMenuTitleExtra(
  SInt16 *  outWidth,
  Boolean   inIsSquished);
enum {
  kThemeMetricScrollBarWidth    = 0,
  kThemeMetricSmallScrollBarWidth = 1,
  kThemeMetricCheckBoxHeight    = 2,
  kThemeMetricRadioButtonHeight = 3,
  kThemeMetricEditTextWhitespace = 4,
  kThemeMetricEditTextFrameOutset = 5,
  kThemeMetricListBoxFrameOutset = 6,
  kThemeMetricFocusRectOutset   = 7,
  kThemeMetricImageWellThickness = 8,
  kThemeMetricScrollBarOverlap  = 9,
  kThemeMetricLargeTabHeight    = 10,
  kThemeMetricLargeTabCapsWidth = 11,
  kThemeMetricTabFrameOverlap   = 12,
  kThemeMetricTabIndentOrStyle  = 13,
  kThemeMetricTabOverlap        = 14,
  kThemeMetricSmallTabHeight    = 15,
  kThemeMetricSmallTabCapsWidth = 16,
  kThemeMetricPushButtonHeight  = 19,
  kThemeMetricListHeaderHeight  = 20,
  kThemeMetricDisclosureTriangleHeight = 25,
  kThemeMetricDisclosureTriangleWidth = 26,
  kThemeMetricLittleArrowsHeight = 27,
  kThemeMetricLittleArrowsWidth = 28,
  kThemeMetricPopupButtonHeight = 30,
  kThemeMetricSmallPopupButtonHeight = 31,
  kThemeMetricLargeProgressBarThickness = 32,
  kThemeMetricPullDownHeight    = 33,
  kThemeMetricSmallPullDownHeight = 34,
  kThemeMetricResizeControlHeight = 38,
  kThemeMetricSmallResizeControlHeight = 39,
  kThemeMetricHSliderHeight     = 41,
  kThemeMetricHSliderTickHeight = 42,
  kThemeMetricVSliderWidth      = 45,
  kThemeMetricVSliderTickWidth  = 46,
  kThemeMetricTitleBarControlsHeight = 49,
  kThemeMetricCheckBoxWidth     = 50,
  kThemeMetricRadioButtonWidth  = 52,
  kThemeMetricNormalProgressBarThickness = 58,
  kThemeMetricProgressBarShadowOutset = 59,
  kThemeMetricSmallProgressBarShadowOutset = 60,
  kThemeMetricPrimaryGroupBoxContentInset = 61,
  kThemeMetricSecondaryGroupBoxContentInset = 62,
  kThemeMetricMenuMarkColumnWidth = 63,
  kThemeMetricMenuExcludedMarkColumnWidth = 64,
  kThemeMetricMenuMarkIndent    = 65,
  kThemeMetricMenuTextLeadingEdgeMargin = 66,
  kThemeMetricMenuTextTrailingEdgeMargin = 67,
  kThemeMetricMenuIndentWidth   = 68,
  kThemeMetricMenuIconTrailingEdgeMargin = 69
};
enum {
  kThemeMetricDisclosureButtonHeight = 17,
  kThemeMetricRoundButtonSize   = 18,
  kThemeMetricSmallCheckBoxHeight = 21,
  kThemeMetricDisclosureButtonWidth = 22,
  kThemeMetricSmallDisclosureButtonHeight = 23,
  kThemeMetricSmallDisclosureButtonWidth = 24,
  kThemeMetricPaneSplitterHeight = 29,
  kThemeMetricSmallPushButtonHeight = 35,
  kThemeMetricSmallRadioButtonHeight = 36,
  kThemeMetricRelevanceIndicatorHeight = 37,
  kThemeMetricLargeRoundButtonSize = 40,
  kThemeMetricSmallHSliderHeight = 43,
  kThemeMetricSmallHSliderTickHeight = 44,
  kThemeMetricSmallVSliderWidth = 47,
  kThemeMetricSmallVSliderTickWidth = 48,
  kThemeMetricSmallCheckBoxWidth = 51,
  kThemeMetricSmallRadioButtonWidth = 53,
  kThemeMetricSmallHSliderMinThumbWidth = 54,
  kThemeMetricSmallVSliderMinThumbHeight = 55,
  kThemeMetricSmallHSliderTickOffset = 56,
  kThemeMetricSmallVSliderTickOffset = 57
};
enum {
  kThemeMetricComboBoxLargeBottomShadowOffset = 70,
  kThemeMetricComboBoxLargeRightShadowOffset = 71,
  kThemeMetricComboBoxSmallBottomShadowOffset = 72,
  kThemeMetricComboBoxSmallRightShadowOffset = 73,
  kThemeMetricComboBoxLargeDisclosureWidth = 74,
  kThemeMetricComboBoxSmallDisclosureWidth = 75,
  kThemeMetricRoundTextFieldContentInsetLeft = 76,
  kThemeMetricRoundTextFieldContentInsetRight = 77,
  kThemeMetricRoundTextFieldContentInsetBottom = 78,
  kThemeMetricRoundTextFieldContentInsetTop = 79,
  kThemeMetricRoundTextFieldContentHeight = 80,
  kThemeMetricComboBoxMiniBottomShadowOffset = 81,
  kThemeMetricComboBoxMiniDisclosureWidth = 82,
  kThemeMetricComboBoxMiniRightShadowOffset = 83,
  kThemeMetricLittleArrowsMiniHeight = 84,
  kThemeMetricLittleArrowsMiniWidth = 85,
  kThemeMetricLittleArrowsSmallHeight = 86,
  kThemeMetricLittleArrowsSmallWidth = 87,
  kThemeMetricMiniCheckBoxHeight = 88,
  kThemeMetricMiniCheckBoxWidth = 89,
  kThemeMetricMiniDisclosureButtonHeight = 90,
  kThemeMetricMiniDisclosureButtonWidth = 91,
  kThemeMetricMiniHSliderHeight = 92,
  kThemeMetricMiniHSliderMinThumbWidth = 93,
  kThemeMetricMiniHSliderTickHeight = 94,
  kThemeMetricMiniHSliderTickOffset = 95,
  kThemeMetricMiniPopupButtonHeight = 96,
  kThemeMetricMiniPullDownHeight = 97,
  kThemeMetricMiniPushButtonHeight = 98,
  kThemeMetricMiniRadioButtonHeight = 99,
  kThemeMetricMiniRadioButtonWidth = 100,
  kThemeMetricMiniTabCapsWidth  = 101,
  kThemeMetricMiniTabFrameOverlap = 102,
  kThemeMetricMiniTabHeight     = 103,
  kThemeMetricMiniTabOverlap    = 104,
  kThemeMetricMiniVSliderMinThumbHeight = 105,
  kThemeMetricMiniVSliderTickOffset = 106,
  kThemeMetricMiniVSliderTickWidth = 107,
  kThemeMetricMiniVSliderWidth  = 108,
  kThemeMetricRoundTextFieldContentInsetWithIconLeft = 109,
  kThemeMetricRoundTextFieldContentInsetWithIconRight = 110,
  kThemeMetricRoundTextFieldMiniContentHeight = 111,
  kThemeMetricRoundTextFieldMiniContentInsetBottom = 112,
  kThemeMetricRoundTextFieldMiniContentInsetLeft = 113,
  kThemeMetricRoundTextFieldMiniContentInsetRight = 114,
  kThemeMetricRoundTextFieldMiniContentInsetTop = 115,
  kThemeMetricRoundTextFieldMiniContentInsetWithIconLeft = 116,
  kThemeMetricRoundTextFieldMiniContentInsetWithIconRight = 117,
  kThemeMetricRoundTextFieldSmallContentHeight = 118,
  kThemeMetricRoundTextFieldSmallContentInsetBottom = 119,
  kThemeMetricRoundTextFieldSmallContentInsetLeft = 120,
  kThemeMetricRoundTextFieldSmallContentInsetRight = 121,
  kThemeMetricRoundTextFieldSmallContentInsetTop = 122,
  kThemeMetricRoundTextFieldSmallContentInsetWithIconLeft = 123,
  kThemeMetricRoundTextFieldSmallContentInsetWithIconRight = 124,
  kThemeMetricSmallTabFrameOverlap = 125,
  kThemeMetricSmallTabOverlap   = 126,
  kThemeMetricSmallPaneSplitterHeight = 127
};
enum {
  kThemeMetricHSliderTickOffset = 128,
  kThemeMetricVSliderTickOffset = 129,
  kThemeMetricSliderMinThumbHeight = 130,
  kThemeMetricSliderMinThumbWidth = 131,
  kThemeMetricScrollBarMinThumbHeight = 132,
  kThemeMetricScrollBarMinThumbWidth = 133,
  kThemeMetricSmallScrollBarMinThumbHeight = 134,
  kThemeMetricSmallScrollBarMinThumbWidth = 135,
  kThemeMetricButtonRoundedHeight = 136,
  kThemeMetricButtonRoundedRecessedHeight = 137
};
enum {
  kThemeMetricSeparatorSize     = 138,
  kThemeMetricTexturedPushButtonHeight = 139,
  kThemeMetricTexturedSmallPushButtonHeight = 140
};
typedef UInt32                          ThemeMetric;
extern OSStatus
GetThemeMetric(
  ThemeMetric   inMetric,
  SInt32 *      outMetric);
extern OSStatus
CopyThemeIdentifier(CFStringRef * outIdentifier);
enum {
  kThemeMetricCheckBoxGlyphHeight = kThemeMetricCheckBoxHeight,
  kThemeMetricRadioButtonGlyphHeight = kThemeMetricRadioButtonHeight,
  kThemeMetricDisclosureButtonSize = kThemeMetricDisclosureButtonHeight,
  kThemeMetricBestListHeaderHeight = kThemeMetricListHeaderHeight,
  kThemeMetricSmallProgressBarThickness = kThemeMetricNormalProgressBarThickness,
  kThemeMetricProgressBarThickness = kThemeMetricLargeProgressBarThickness
};

#pragma pack(pop)

#ifdef __cplusplus
}
#endif

#endif // _HITOOLBOX_HITHEME_H_
